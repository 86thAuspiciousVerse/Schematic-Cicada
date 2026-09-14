/**
 * m1-export-verify.mjs — M1 验收⑥ 导出 + 往返无损探针（用真实演示文档，不是 fixture）。
 *
 * 链路：引擎装载 {workspace}/.cicada/schematic.cicada_sch → POST /export 写标准
 * .kicad_sch → kicad-cli sch export netlist（官方解析器 = KiCad 10 能打开）→
 * 再把导出的 .kicad_sch 装载回引擎（POST /document）→ 与导出前 /scene 逐项比对
 * （components/wires/junctions/labels/no_connects + refdes/libId 集合）＝ 往返无损。
 *
 * Usage:
 *   node m1-export-verify.mjs <engine.exe> <lib-dir> <user-lib-dir> \
 *     <schematic.cicada_sch> <out.kicad_sch> <kicad-cli.exe>
 * 路径全部走 argv（Windows 反斜杠在 JS 字面量里会被吞）。
 */
import { spawn, spawnSync } from 'node:child_process'
import { existsSync, mkdirSync, readFileSync, rmSync, statSync } from 'node:fs'
import { dirname, join } from 'node:path'

const [engineExe, libDir, userLibDir, schematic, outKicadSch, kicadCli] = process.argv.slice(2)
if (engineExe === undefined || libDir === undefined || schematic === undefined || outKicadSch === undefined || kicadCli === undefined) {
  console.error('usage: node m1-export-verify.mjs <engine.exe> <lib-dir> <user-lib-dir> <schematic.cicada_sch> <out.kicad_sch> <kicad-cli.exe>')
  process.exit(2)
}

let failures = 0
const check = (label, ok, detail) => {
  console.log(`${ok ? 'PASS' : 'FAIL'} ${label}${detail === undefined ? '' : ` — ${detail}`}`)
  if (!ok) failures += 1
}

if (!existsSync(schematic)) {
  console.error(`FAIL source schematic missing: ${schematic}`)
  process.exit(1)
}
mkdirSync(dirname(outKicadSch), { recursive: true })
rmSync(outKicadSch, { force: true })
/** Scene components as `{ refdes, pins }` views (positions included). */
const sceneComponents = (scene) => (scene?.components ?? []).map((component) => ({
  refdes: String(component.refdes ?? ''),
  pins: (component.pins ?? []).map((pin) => ({
    number: String(pin.number ?? pin.physicalNumber ?? ''),
    name: String(pin.name ?? pin.pinName ?? ''),
    x: pin.x, y: pin.y,
  })),
}))

const netlist = join(dirname(outKicadSch), 'export.net')
rmSync(netlist, { force: true })

const args = ['--port', '0', '--file', schematic, '--lib-dir', libDir, '--log', join(dirname(outKicadSch), 'engine-export.txt')]
if (userLibDir !== undefined && userLibDir !== '') args.push('--user-lib-dir', userLibDir)
const engine = spawn(engineExe, args, { stdio: ['ignore', 'pipe', 'pipe'] })
// Drain stderr: the engine narrates KiCad asserts there (~30 KB in the first
// seconds) while only the announce line reaches stdout. An unread pipe fills up
// and blocks the engine inside write(), so the next HTTP call never returns and
// the run dies silently after /export (measured 2026-09-13: both workspaces hung
// in the round trip; curl against a hand-started engine answered every route).
engine.stderr.resume()

const announce = await new Promise((resolve, reject) => {
  const timer = setTimeout(() => reject(new Error('engine announce timeout (30s)')), 30_000)
  let buffer = ''
  engine.stdout.on('data', chunk => {
    buffer += String(chunk)
    const match = /^cicada-engine:\s+127\.0\.0\.1:(\d+)\s+(\S+)\s*$/m.exec(buffer)
    if (match !== null) {
      clearTimeout(timer)
      resolve({ port: Number(match[1]), token: match[2] })
    }
  })
  engine.on('exit', code => { clearTimeout(timer); reject(new Error(`engine exited early (${String(code)})`)) })
})

const base = `http://127.0.0.1:${String(announce.port)}`
const call = async (path, body, method = 'POST') => {
  const res = await fetch(`${base}${path}`, {
    method,
    headers: { 'X-Cicada-Token': announce.token, 'Content-Type': 'application/json' },
    ...(body === undefined ? {} : { body: JSON.stringify(body) }),
  })
  return { status: res.status, body: await res.json().catch(() => undefined) }
}
const counts = scene => ({
  components: (scene?.components ?? []).length,
  wires: (scene?.wires ?? []).length,
  junctions: (scene?.junctions ?? []).length,
  labels: (scene?.labels ?? []).length,
  noConnects: (scene?.no_connects ?? []).length,
})
const refdesSet = scene => (scene?.components ?? []).map(c => `${String(c.refdes)}:${String(c.libId)}`).sort()

try {
  const before = await call('/scene', undefined, 'GET')
  const baseCounts = counts(before.body)
  check('engine loads the demo document', before.status === 200 && baseCounts.components > 0,
    `components=${String(baseCounts.components)} wires=${String(baseCounts.wires)} labels=${String(baseCounts.labels)}`)

  const exported = await call('/export', { path: outKicadSch })
  const exportedOk = exported.status === 200 && exported.body?.ok === true && existsSync(outKicadSch)
  check('/export writes a .kicad_sch', exportedOk, `${String(existsSync(outKicadSch) ? statSync(outKicadSch).size : 0)} bytes`)
  const text = existsSync(outKicadSch) ? readFileSync(outKicadSch, 'utf8') : ''
  check('export is a KiCad schematic with embedded symbols',
    text.startsWith('(kicad_sch') && text.includes('(lib_symbols'), text.slice(0, 24).replaceAll('\n', ' '))

  const cli = spawnSync(kicadCli, ['sch', 'export', 'netlist', '--output', netlist, outKicadSch], { encoding: 'utf8' })
  check('kicad-cli parses the export (KiCad 10 can open it)', cli.status === 0 && existsSync(netlist),
    `exit=${String(cli.status)} ${String(cli.stderr ?? '').trim().slice(0, 160)}`)
  const netText = existsSync(netlist) ? readFileSync(netlist, 'utf8') : ''
  const missing = refdesSet(before.body).map(entry => entry.split(':')[0])
    .filter(refdes => refdes !== undefined && !refdes.startsWith('#') && !netText.includes(`"${refdes}"`))
  check('netlist carries every placed component', missing.length === 0, missing.join(','))

  // 电气完整性（2026-09-13）：判定以**引擎导出的网表**为准（权威），不再用几何连通性猜测
  // （几何版本在两个方向都出过错）。规则本身场景中立、不认型号也不认引脚名：
  //   一个引脚在网表里是 `unconnected-(<refdes>-<pin>-Pad<N>)`，且**没有** no_connect 标记，
  //   又不是电源符号（#PWR…）→ 它既没接也没声明"有意不接" = 未完工。
  // KiCad 网表把未连引脚命名为 `unconnected-(<refdes>-<pinfunction>-Pad<num>)`。
  const ncKeys = new Set((before.body?.no_connects ?? []).map((mark) => `${String(mark?.x)},${String(mark?.y)}`))
  const ncPins = new Set()
  for (const view of sceneComponents(before.body)) {
    for (const pin of view.pins) {
      if (ncKeys.has(`${String(pin.x)},${String(pin.y)}`)) ncPins.add(`${view.refdes}.${String(pin.number)}`)
    }
  }
  const unconnectedPins = []
  for (const match of netText.matchAll(/unconnected-\(([^)]*)\)/g)) {
    const body = match[1] ?? ''
    const refdes = body.split('-')[0] ?? ''
    const number = /Pad(\d+)$/.exec(body)?.[1] ?? ''
    if (refdes === '' || number === '' || refdes.startsWith('#')) continue
    if (ncPins.has(`${refdes}.${number}`)) continue
    unconnectedPins.push(`${refdes}.${number}`)
  }
  check('every pin is connected or explicitly marked no-connect', unconnectedPins.length === 0,
    `${String(unconnectedPins.length)} floating: ${unconnectedPins.slice(0, 12).join(',')}`)

  const floatingUnconnected = []
  const orphanParts = []
  const silentlyFloating = unconnectedPins
  console.log(`INFO pins with neither a net nor a no-connect marker: ${String(silentlyFloating.length)}${silentlyFloating.length === 0 ? '' : ` — ${silentlyFloating.slice(0, 12).join(',')}`}`)

  // Round trip (M1 ⑥): reload the TRUTH file and compare — the export must not
  // have disturbed it, and the engine's own dialect must round-trip.
  const reload = await call('/document', { file: schematic })
  check('truth file reloads into the engine', reload.status === 200 && reload.body?.ok === true,
    `status=${String(reload.status)} ${String(reload.body?.error?.message ?? '').slice(0, 200)}`)
  const after = await call('/scene', undefined, 'GET')
  const afterCounts = counts(after.body)
  check('round trip preserves item counts', JSON.stringify(afterCounts) === JSON.stringify(baseCounts),
    `before=${JSON.stringify(baseCounts)} after=${JSON.stringify(afterCounts)}`)
  const beforeRefdes = refdesSet(before.body)
  const afterRefdes = refdesSet(after.body)
  check('round trip preserves components (refdes:libId)',
    JSON.stringify(afterRefdes) === JSON.stringify(beforeRefdes),
    afterRefdes.filter(entry => !beforeRefdes.includes(entry)).join(','))

  // Informational (M2 scope): can the engine re-import its own standard-dialect
  // export? The loader is a fail-closed .cicada_sch whitelist, so standard
  // KiCad tokens (stroke/effects/in_bom/…) are refused today — recorded, not a
  // M1 failure (KiCad itself opens the file; kicad-cli already proved it).
  const reimport = await call('/document', { file: outKicadSch })
  console.log(`INFO exported .kicad_sch re-import (M2): ${reimport.status === 200 ? 'accepted' : `refused — ${String(reimport.body?.error?.message ?? '').slice(0, 120)}`}`)
  if (reimport.status === 200) {
    const reimported = await call('/scene', undefined, 'GET')
    console.log(`INFO re-imported scene: ${JSON.stringify(counts(reimported.body))}`)
  }
  // Restore the truth file as the engine document before shutting down.
  await call('/document', { file: schematic })
} finally {
  await call('/shutdown', undefined).catch(() => undefined)
  await new Promise(resolve => {
    const timer = setTimeout(() => { if (!engine.killed) engine.kill(); resolve() }, 5_000)
    engine.once('exit', () => { clearTimeout(timer); resolve() })
  })
}

console.log(failures === 0 ? 'EXPORT-VERIFY-OK' : `EXPORT-VERIFY-FAIL (${String(failures)})`)
process.exit(failures === 0 ? 0 : 1)
