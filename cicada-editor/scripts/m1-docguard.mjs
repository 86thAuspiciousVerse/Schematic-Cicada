/**
 * m1-docguard.mjs — regression probe for the 2026-09-08 canvas defect.
 *
 * Two engine-side guarantees the canvas/host now rely on:
 *   ① /ops without a loaded document = 409 conflict (never "edit memory, then
 *      fail saveback with 'unable to open schematic file for writing'");
 *   ② /document with a not-yet-existing path creates the parent dirs + an empty
 *      .cicada_sch (engine owns the format), so a fresh workspace is editable
 *      the moment the canvas opens; /document "" clears the in-memory document.
 *
 * Usage: node m1-docguard.mjs <engine.exe> <lib-dir> <user-lib-dir> <work-dir>
 * (paths from argv: Windows backslashes must never be written inside JS).
 */
import { spawn } from 'node:child_process'
import { existsSync, mkdirSync, readFileSync, rmSync } from 'node:fs'
import { join } from 'node:path'

const [engineExe, libDir, userLibDir, workDir] = process.argv.slice(2)
if (engineExe === undefined || libDir === undefined || workDir === undefined) {
  console.error('usage: node m1-docguard.mjs <engine.exe> <lib-dir> <user-lib-dir> <work-dir>')
  process.exit(2)
}

let failures = 0
const check = (label, ok, detail) => {
  console.log(`${ok ? 'PASS' : 'FAIL'} ${label}${detail === undefined ? '' : ` — ${detail}`}`)
  if (!ok) failures += 1
}

rmSync(workDir, { recursive: true, force: true })
mkdirSync(workDir, { recursive: true })
const logPath = join(workDir, 'engine.txt')
const docPath = join(workDir, 'fresh-workspace', '.cicada', 'schematic.cicada_sch')

const args = ['--port', '0', '--lib-dir', libDir, '--log', logPath]
if (userLibDir !== undefined && userLibDir !== '') args.push('--user-lib-dir', userLibDir)
const engine = spawn(engineExe, args, { stdio: ['ignore', 'pipe', 'pipe'] })

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

try {
  // ① no document yet: a write op must be refused, not half-applied.
  const blocked = await call('/ops', { fileHash: '', op: 'draw-wire', points: [[0, 0], [12700, 0]] })
  check('/ops without a document -> 409 conflict', blocked.status === 409 && blocked.body?.error?.code === 'conflict',
    `status=${String(blocked.status)} code=${String(blocked.body?.error?.code)}`)
  check('conflict message names the missing document',
    String(blocked.body?.error?.message ?? '').includes('no document loaded'),
    String(blocked.body?.error?.message ?? ''))

  // ② /document on a fresh path creates the truth file and loads it.
  const created = await call('/document', { file: docPath })
  check('/document creates a missing truth file', created.status === 200 && created.body?.ok === true,
    `status=${String(created.status)}`)
  check('created file exists on disk', existsSync(docPath), docPath)
  check('engine serves the created document', created.body?.scene?.file?.replaceAll('\\', '/') === docPath.replaceAll('\\', '/'),
    String(created.body?.scene?.file ?? ''))
  check('created document parses back (empty scene)', Array.isArray(created.body?.scene?.components)
    && created.body.scene.components.length === 0)

  // ③ the same write now succeeds and reaches the file (saveback works).
  const drawn = await call('/ops', { fileHash: created.body?.scene?.hash ?? '', op: 'draw-wire', points: [[12700, 12700], [25400, 12700]] })
  check('/ops after sync -> ok', drawn.status === 200 && drawn.body?.ok === true, `status=${String(drawn.status)}`)
  const text = existsSync(docPath) ? readFileSync(docPath, 'utf8') : ''
  check('saveback wrote the wire to disk', text.includes('(wire') && text.includes('1.27 1.27'), `${String(text.length)} bytes`)

  // ③b saveback must preserve library semantics (2026-09-08 验收实测缺陷：引擎回写
  // 曾丢掉 (power)/电气类型 → 语义层与 KiCad netlist 双双退化为 Net-(C1-P1)）。
  const beforePlace = await call('/scene', undefined, 'GET')
  const placed = await call('/ops', {
    fileHash: beforePlace.body?.hash ?? '', op: 'place-symbol', libId: 'POWER:GND', x: 25400, y: 25400,
  })
  check('place-symbol POWER:GND -> ok', placed.status === 200 && placed.body?.ok === true,
    `status=${String(placed.status)}`)
  const saved = existsSync(docPath) ? readFileSync(docPath, 'utf8') : ''
  check('saveback keeps the (power) flag', /\(symbol "POWER:GND" \(power\)/.test(saved),
    saved.split('\n').find(line => line.includes('POWER:GND'))?.slice(0, 120) ?? '')
  check('saveback keeps pin electrical types', /\(pin power_in line/.test(saved),
    saved.split('\n').find(line => line.includes('(pin '))?.trim().slice(0, 120) ?? '')

  // ④ /document "" clears the document (no phantom content behind an empty file).
  const cleared = await call('/document', { file: '' })
  check('/document "" clears the document', cleared.status === 200
    && cleared.body?.scene?.file === '' && (cleared.body?.scene?.wires ?? []).length === 0,
    `wires=${String((cleared.body?.scene?.wires ?? []).length)}`)
  const blockedAgain = await call('/ops', { fileHash: '', op: 'draw-wire', points: [[0, 0], [12700, 0]] })
  check('/ops after clear -> 409 again', blockedAgain.status === 409, `status=${String(blockedAgain.status)}`)
} finally {
  await call('/shutdown', undefined).catch(() => undefined)
  // Wait for the process to actually leave before exiting: killing a live
  // libuv handle on the way out trips an assertion in the Windows runtime.
  await new Promise(resolve => {
    const timer = setTimeout(() => { if (!engine.killed) engine.kill(); resolve() }, 5000)
    engine.once('exit', () => { clearTimeout(timer); resolve() })
  })
}

console.log(failures === 0 ? 'DOCGUARD-OK' : `DOCGUARD-FAIL (${String(failures)})`)
process.exit(failures === 0 ? 0 : 1)
