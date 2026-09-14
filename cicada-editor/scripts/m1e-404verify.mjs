// m1e-404verify.mjs — /lib/get 键寻址契约（docs/09 §4）：
//   ① 未命中 → 404 not_found（自动查缺 miss 判定，docs/09 §5）
//   ② name-only 命中内置件（R/C/LED）→ 200 + 规范键 R:R/C:C/LED:LED
//   ③ 尾段名命中精选库（C_Small）→ 200 + 规范键 C:C_Small
import { spawn } from 'node:child_process'
import { rmSync, mkdirSync } from 'node:fs'
import { join } from 'node:path'

const exe = 'C:\\dsh\\Schematic-Cicada\\cicada-editor\\build-msvc-kicad\\Release\\cicada-engine.exe'
const work = 'C:\\dsh\\Schematic-Cicada\\cicada-editor\\build-msvc-kicad\\m1e-404v'
try { rmSync(work, { recursive: true, force: true }) } catch {}
mkdirSync(work, { recursive: true })

const child = spawn(exe, ['--port', '0', '--lib-dir', 'C:\\dsh\\Schematic-Cicada\\assets\\cicada-libs', '--log', join(work, 'e.txt')], { stdio: ['ignore', 'pipe', 'pipe'] })
const announce = await new Promise((resolve, reject) => {
  const t = setTimeout(() => reject(new Error('no announce')), 30000)
  let buf = ''
  child.stdout.on('data', (c) => {
    buf += c.toString()
    const m = buf.match(/cicada-engine: 127\.0\.0\.1:(\d+) ([0-9a-f]+)/)
    if (m) { clearTimeout(t); resolve({ port: m[1], token: m[2] }) }
  })
  child.stderr.on('data', () => {})
})
console.log('engine:', announce)

async function get(libId) {
  const resp = await fetch(`http://127.0.0.1:${announce.port}/lib/get`, {
    method: 'POST',
    headers: { 'X-Cicada-Token': announce.token, 'Content-Type': 'application/json' },
    body: JSON.stringify({ libId }),
  })
  return { status: resp.status, body: await resp.json().catch(() => undefined) }
}

const miss = await get('IC:NOPE')
console.log('unknown libId ->', miss.status, JSON.stringify(miss.body))
const missOk = miss.status === 404 && miss.body?.error?.code === 'not_found'

let keysOk = true
for (const [query, expected] of [['R', 'R:R'], ['C', 'C:C'], ['LED', 'LED:LED'], ['C_Small', 'C:C_Small'], ['C:C', 'C:C']]) {
  const hit = await get(query)
  const ok = hit.status === 200 && hit.body?.libId === expected && (hit.body?.pins ?? []).length > 0
  if (!ok) keysOk = false
  console.log(`/lib/get ${query} -> ${hit.status} libId=${hit.body?.libId} pins=${(hit.body?.pins ?? []).length} ${ok ? 'OK' : 'FAIL'}`)
}

console.log(missOk && keysOk ? '404VERIFY-OK LIBKEYS-OK' : `404VERIFY-${missOk ? 'OK' : 'FAIL'} LIBKEYS-${keysOk ? 'OK' : 'FAIL'}`)

await fetch(`http://127.0.0.1:${announce.port}/shutdown`, { method: 'POST', headers: { 'X-Cicada-Token': announce.token } }).catch(() => {})
child.kill()
process.exit(missOk && keysOk ? 0 : 1)
