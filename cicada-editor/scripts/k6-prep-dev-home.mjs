// K6 环境预配：持久化隔离 DSH home（initProfile + 工作区 junction 闭包；可重复运行，已配则跳过）
// 产出：开发环境就绪 + 打印 exe 启动配方（三 env）。用法：
//   cd cicada-harness && node --import tsx/esm ../cicada-editor/scripts/k6-prep-dev-home.mjs
import fs from 'node:fs'
import path from 'node:path'
import { spawnSync, execSync } from 'node:child_process'

const here = path.resolve(process.cwd())
const HOME = process.env.CICADA_DEV_HOME ?? '/mnt/c/dsh/Schematic-Cicada/cicada-dev-home'

const { initProfile } = await import(path.join(here, 'packages/cicada/cicada-launcher/src/launcher.ts'))
initProfile(HOME)
const profileDir = path.join(HOME, 'profiles', 'cicada')
const nmDst = path.join(profileDir, 'node_modules')
if (!fs.existsSync(nmDst)) {
  const winDst = execSync('wslpath -w ' + JSON.stringify(nmDst)).toString().trim()
  const winSrc = execSync('wslpath -w ' + JSON.stringify(path.join(here, 'node_modules'))).toString().trim()
  spawnSync('cmd.exe', ['/c', 'mklink', '/J', winDst, winSrc])
}
// K6 关键：子进程 spawn cwd = home 根（host_spawner 设计）；根 node_modules 也必须可达
// （`--import tsx/esm` 从 cwd 解析 tsx——此前只 junction profile 层 → ERR_MODULE_NOT_FOUND）
const rootNm = path.join(HOME, 'node_modules')
if (!fs.existsSync(rootNm)) {
  const winDst = execSync('wslpath -w ' + JSON.stringify(rootNm)).toString().trim()
  const winSrc = execSync('wslpath -w ' + JSON.stringify(path.join(here, 'node_modules'))).toString().trim()
  spawnSync('cmd.exe', ['/c', 'mklink', '/J', winDst, winSrc])
}

function walk(dir, acc) {
  for (const e of fs.readdirSync(dir, { withFileTypes: true })) {
    if (e.name === 'node_modules' || e.name === '.git') continue
    const p = path.join(dir, e.name)
    if (!e.isDirectory()) continue
    if (fs.existsSync(path.join(p, 'package.json'))) {
      try {
        const j = JSON.parse(fs.readFileSync(path.join(p, 'package.json'), 'utf8'))
        if (j.name?.startsWith('@deepseek-ai/')) acc.set(j.name, p)
      } catch {}
    }
    walk(p, acc)
  }
}
const workspace = new Map()
walk(path.join(here, 'packages'), workspace)
console.log('[prep] workspace pkgs=', workspace.size)
const queue = ['@deepseek-ai/dsh-base', '@deepseek-ai/dsh-web-app', '@deepseek-ai/dsh-cicada-app']
// 清掉旧 @deepseek-ai 目录（可能残留坏链接/错误相对 symlink），全量重建 junction
fs.rmSync(path.join(profileDir, 'node_modules', '@deepseek-ai'), { recursive: true, force: true })
const seen = new Set()
let n = 0
while (queue.length) {
  const name = queue.shift()
  const dir = workspace.get(name)
  if (!dir || seen.has(name)) continue
  const target = path.join(profileDir, 'node_modules', '@deepseek-ai', name.replace('@deepseek-ai/', ''))
  if (!fs.existsSync(target)) {
    fs.mkdirSync(path.dirname(target), { recursive: true })
    const winT = execSync('wslpath -w ' + JSON.stringify(target)).toString().trim()
    const winD = execSync('wslpath -w ' + JSON.stringify(dir)).toString().trim()
    spawnSync('cmd.exe', ['/c', 'mklink', '/J', winT, winD])
  }
  seen.add(name); n++
  const j = JSON.parse(fs.readFileSync(path.join(dir, 'package.json'), 'utf8'))
  for (const depName of [...Object.keys(j.dependencies ?? {}), ...Object.keys(j.optionalDependencies ?? {})])
    if (workspace.has(depName) && !seen.has(depName)) queue.push(depName)
}
console.log('[prep] home=', HOME)
console.log('[prep] profile junctions=', n)
console.log('[prep] built entry 存在=', fs.existsSync(path.join(here, 'apps/cli/lib/bin.js')))

const winHome = execSync('wslpath -w ' + JSON.stringify(HOME)).toString().trim()
console.log('\n===== 启动配方（cmd 里设置后运行 cicada-editor.exe）=====')
console.log('set CICADA_DSH_NODE=C:\\Program Files\\nodejs\\node.exe')
console.log('set CICADA_DSH_ENTRY=C:\\dsh\\Schematic-Cicada\\cicada-harness\\apps\\cli\\src\\bin.ts')
console.log('set CICADA_DSH_PRELOAD=tsx/esm')
console.log('set CICADA_DSH_HOME=' + winHome)
console.log('set CICADA_DSH_CWD=C:\\dsh\\Schematic-Cicada\\cicada-harness')
console.log('cd C:\\dsh\\Schematic-Cicada\\cicada-editor\\build-msvc-kicad\\Release && cicada-editor.exe')
console.log('\n[注] 用 dev 源码入口+tsx/esm（tsx 走 tsconfig paths；built lib/bin.js 纯 ESM 解析\n找不到 @deepseek-ai/cordis-plugin-timer 等 vendored 依赖——不建议）')
