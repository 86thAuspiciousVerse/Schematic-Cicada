// K6 web boot diag: fresh home on port 3124 → fetch the authenticated index →
// dump injected __DSH_BOOT__ graph summary + HTTP status of every page/bundle URL.
// Run with WSL node from the harness root (Windows node cannot import() a c:\ path):
//   cd cicada-harness && node --import tsx/esm ../cicada-editor/scripts/k6-web-diag.mjs
import fs from 'node:fs'
import path from 'node:path'
import { spawn, spawnSync, execSync } from 'node:child_process'

const here = process.cwd()
const HOME = process.env.CICADA_DIAG_HOME ?? '/mnt/c/dsh/Schematic-Cicada/cicada-dev-home'
const PORT = process.env.CICADA_DIAG_PORT ?? '3124'
const WIN_HOME = execSync('wslpath -w ' + JSON.stringify(HOME)).toString().trim()
const OUT = path.join(here, '..', 'cicada-editor', 'logs', 'web-diag')

fs.mkdirSync(OUT, { recursive: true })

const profileDir = path.join(HOME, 'profiles', 'cicada')
if (!fs.existsSync(path.join(profileDir, 'cordis.yml'))) {
  // Fresh home: initProfile + junctions (dev-home already assembled).
  console.log('[diag] initProfile ...')
  const { initProfile } = await import(path.join(here, 'packages/cicada/cicada-launcher/src/launcher.ts'))
  initProfile(HOME)
  const nmDst = path.join(profileDir, 'node_modules')
  if (!fs.existsSync(nmDst)) {
    console.log('[diag] junction profile node_modules -> harness node_modules')
    const winDst = execSync('wslpath -w ' + JSON.stringify(nmDst)).toString().trim()
    const winSrc = execSync('wslpath -w ' + JSON.stringify(path.join(here, 'node_modules'))).toString().trim()
    const r = spawnSync('cmd.exe', ['/c', 'mklink', '/J', winDst, winSrc])
    console.log('[diag] junction:', r.stdout?.toString().trim(), r.stderr?.toString().trim())
  }
} else {
  console.log('[diag] using existing profile:', profileDir)
}

console.log(`[diag] spawn host on ${PORT} ...`)
const child = spawn('/mnt/c/Program Files/nodejs/node.exe', [
  '--import', 'tsx/esm', 'apps/cli/src/bin.ts',
  '--profile', 'cicada', '--no-open', '--port', PORT, '--host', '127.0.0.1',
], {
  cwd: here,
  env: { ...process.env, DSH_HOME: WIN_HOME, CICADA_HOME: WIN_HOME },
  stdio: ['ignore', 'pipe', 'pipe'],
})

let out = ''
let done = false
const finish = (code) => {
  if (done) return
  done = true
  console.log(`[diag] done, killing host (exit ${code})`)
  child.kill()
  process.exit(code ?? 0)
}
const timer = setTimeout(() => { console.log('[diag] timeout 150s'); finish(2) }, 150000)
child.stdout.on('data', (d) => { out += d })
child.stderr.on('data', (d) => { out += d })
child.on('close', (code) => {
  console.log(`[diag] host closed (${code})`)
  console.log('---- host output tail ----')
  console.log(out.slice(-3000))
  clearTimeout(timer)
  finish(code ?? 0)
})

async function probe() {
  const m = out.match(/dsh web: (\S+)/)
  if (!m || done) return
  const url = m[1]
  console.log('[diag] web url:', url)
  let html
  try {
    const res = await fetch(url)
    html = await res.text()
    console.log('[diag] index status:', res.status, 'bytes:', html.length)
  } catch (e) {
    console.log('[diag] index fetch failed:', e)
    finish(3)
    return
  }
  fs.writeFileSync(path.join(OUT, 'index.html'), html)

  const boot = html.match(/globalThis\["__DSH_BOOT__"\] = ([\s\S]*?);<\/script>/)
  if (!boot) {
    console.log('[diag] NO __DSH_BOOT__ global in served HTML!')
    const rows = [...html.matchAll(/<script[^>]*src="([^"]+)"/g)].map((x) => x[1])
    console.log('[diag] script-src rows:', rows.length, rows.slice(0, 12))
    for (const r of rows) await statUrl(r, url)
    finish(4)
    return
  }
  fs.writeFileSync(path.join(OUT, 'boot.json'), boot[1])
  console.log('[diag] __DSH_BOOT__ bytes:', boot[1].length, 'head:', boot[1].slice(0, 120))

  // collect every .js URL in the served page (inline graph + script tags), dedupe, fetch
  const urls = new Set()
  for (const r of html.matchAll(/(?:src|href)="([^"]+\.js(?:[^"]*)?)"/g)) urls.add(r[1])
  for (const r of boot[1].matchAll(/"([^"]+\.js(?:[^"]*)?)"/g)) urls.add(r[1])
  console.log('[diag] unique .js urls:', urls.size)
  for (const u of urls) await statUrl(u, url)
  finish(0)
}

async function statUrl(u, baseUrl) {
  const abs = new URL(u, baseUrl).href
  try {
    const res = await fetch(abs)
    const len = parseInt(res.headers.get('content-length') ?? '0', 10) || (await res.text()).length
    console.log(`[diag]   ${res.status}  ${abs}  (${len} bytes)`)
  } catch (e) {
    console.log(`[diag]   ERR ${abs}  ${e}`)
  }
}

const poll = setInterval(probe, 500)
setTimeout(() => clearInterval(poll), 145000)
