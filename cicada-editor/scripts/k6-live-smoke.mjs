// K6 活体冒烟：隔离 CICADA_HOME 起 cicada host → 校验 P7 协议（C1 spawn/握手 + HTTP 路由 + WS hello）
// 用法: cd cicada-harness && CICADA_SMOKE_HOME=C:/dsh/Schematic-Cicada/cicada-smoke-home node --import tsx/esm ../cicada-editor/scripts/k6-live-smoke.mjs
import fs from 'node:fs'
import os from 'node:os'
import path from 'node:path'
import { spawn, spawnSync, execSync } from 'node:child_process'

const here = path.resolve(process.cwd())
// WSL 形态路径（node 跑在 WSL：/mnt/c/...），junction 另转 Windows 形态
const HOME = process.env.CICADA_SMOKE_HOME ?? '/tmp/cicada-smoke-home'

// 1) 隔离 home 预装配（initProfile；node_modules 用 junction 链到仓库——P7 隔离冒烟模式）
fs.rmSync(HOME, { recursive: true, force: true })
const { initProfile } = await import(path.join(here, 'packages/cicada/cicada-launcher/src/launcher.ts'))
initProfile(HOME)
const profilePkgDir = path.join(HOME, 'profiles', 'cicada')
const nmSrc = path.join(here, 'node_modules')
const nmDst = path.join(profilePkgDir, 'node_modules')
if (!fs.existsSync(nmDst)) {
  const winDst = execSync('wslpath -w ' + JSON.stringify(nmDst)).toString().trim()
  const winSrc = execSync('wslpath -w ' + JSON.stringify(nmSrc)).toString().trim()
  const r = spawnSync('cmd.exe', ['/c', 'mklink', '/J', winDst, winSrc])
  if (r.status !== 0) console.error('[smoke] base junction failed:', r.stdout?.toString())
}

// 工作区包闭包 → profile node_modules/@deepseek-ai/<pkg> junction（P7 "16 junction" 模式）
function walk(dir, acc) {
  for (const e of fs.readdirSync(dir, { withFileTypes: true })) {
    if (e.name === 'node_modules' || e.name === '.git') continue
    const p = path.join(dir, e.name)
    if (e.isDirectory()) {
      if (fs.existsSync(path.join(p, 'package.json'))) {
        try {
          const j = JSON.parse(fs.readFileSync(path.join(p, 'package.json'), 'utf8'))
          if (j.name?.startsWith('@deepseek-ai/')) acc.set(j.name, p)
        } catch {}
      }
      walk(p, acc)
    }
  }
}
const workspace = new Map()
walk(path.join(here, 'packages'), workspace)
const queue = ['@deepseek-ai/dsh-base', '@deepseek-ai/dsh-web-app', '@deepseek-ai/dsh-cicada-app']
const seen = new Set()
let junctioned = 0
while (queue.length) {
  const name = queue.shift()
  const dir = workspace.get(name)
  if (!dir) continue
  const target = path.join(profilePkgDir, 'node_modules', '@deepseek-ai', name.replace('@deepseek-ai/', ''))
  if (fs.existsSync(target)) continue
  fs.mkdirSync(path.dirname(target), { recursive: true })
  const winT = execSync('wslpath -w ' + JSON.stringify(target)).toString().trim()
  const winD = execSync('wslpath -w ' + JSON.stringify(dir)).toString().trim()
  const r = spawnSync('cmd.exe', ['/c', 'mklink', '/J', winT, winD])
  if (r.status === 0) { junctioned++; seen.add(name) } else console.log('[smoke] junction fail:', name)
  const j = JSON.parse(fs.readFileSync(path.join(dir, 'package.json'), 'utf8'))
  for (const depName of [...Object.keys(j.dependencies ?? {}), ...Object.keys(j.optionalDependencies ?? {})]) {
    if (workspace.has(depName) && !seen.has(depName)) queue.push(depName)
  }
}
console.log('[smoke] workspace junctions=', junctioned)
console.log('[smoke] home=', HOME)

// 2) 起 host；解析 stdout 两行协议
let port = 0, token = ''
const child = spawn('node', ['--import', 'tsx/esm', 'packages/cicada/cicada-launcher/bin/cicada.ts'], {
  cwd: here,
  env: { ...process.env, CICADA_HOME: HOME, DSH_HOME: HOME },
})
child.stdout.on('data', (d) => {
  const s = d.toString()
  process.stdout.write('[host] ' + s)
  const m = s.match(/cicada-editor: (\d+) ([^\s]+)/)
  if (m) { port = Number(m[1]); token = m[2] }
})
child.stderr.on('data', (d) => process.stdout.write('[host:err] ' + d.toString()))
child.on('exit', (c) => console.log('[smoke] host exit code=', c))

function sleep(ms) { return new Promise((r) => setTimeout(r, ms)) }

for (let i = 0; i < 120 && (!port || !token); i++) {
  if (i % 10 === 9) console.log('[smoke] waiting…', (i + 1) * 2, 's (port=', port, ')')
  await sleep(2000)
}
console.log('[smoke] port=', port, 'token=', token ? 'yes' : 'NO')
if (!port || !token) { child.kill(); process.exit(1) }

// 3) HTTP 路由
const base = 'http://127.0.0.1:' + port
const ping = await (await fetch(base + '/_cicada/ping')).json()
console.log('[smoke] ping=', JSON.stringify(ping))
const state = await fetch(base + '/cicada/editor/state', { headers: { authorization: 'Bearer ' + token } })
console.log('[smoke] state=', state.status, (await state.text()).slice(0, 300))
const sel = await fetch(base + '/cicada/editor/selection', {
  method: 'POST',
  headers: { authorization: 'Bearer ' + token, 'content-type': 'application/json' },
  body: JSON.stringify({ selection: [{ kind: 'wire', refdes: 'smoke' }] }),
})
console.log('[smoke] selection=', sel.status, (await sel.text()).slice(0, 200))

// 4) WS hello（ws 包在 editor-bridge 包内；createRequire 定位）
try {
  const { createRequire } = await import('node:module')
  const bridgeReq = createRequire(path.join(here, 'packages/cicada/cicada-editor-bridge/package.json'))
  const WebSocket = bridgeReq('ws')
  const ok = await new Promise((resolve, reject) => {
    const ws = new WebSocket('ws://127.0.0.1:' + port + '/cicada/editor/ws', { headers: { authorization: 'Bearer ' + token } })
    const to = setTimeout(() => { ws.close(); reject(new Error('ws timeout')) }, 8000)
    ws.on('message', (m) => { clearTimeout(to); console.log('[smoke] ws frame=', String(m).slice(0, 200)); resolve(true); ws.close() })
    ws.on('error', (e) => { clearTimeout(to); reject(e) })
  })
  console.log('[smoke] ws ok=', ok)
} catch (e) {
  console.log('[smoke] ws FAILED:', String(e).slice(0, 200))
}

child.kill()
await sleep(1500)
console.log('[smoke] done; host killed')
process.exit(0)
