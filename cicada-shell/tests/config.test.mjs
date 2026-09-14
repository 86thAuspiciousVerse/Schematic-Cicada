import { test } from 'node:test'
import assert from 'node:assert/strict'
import { join } from 'node:path'
import {
  ENGINE_RUNTIME_DLLS, launcherEnv, missingEngineRuntime, preflight, preloadFor,
  resolveNodeBin, resolveShellConfig,
} from '../src/config.js'

const SHELL = join('/repo', 'cicada-shell')
/** Deterministic PATH probe: a POSIX-shaped PATH with one node in it. */
const NODE_OPTS = { exists: () => true, pathValue: '/usr/bin:/bin', separator: ':', exeName: 'node' }
const NODE = join('/usr/bin', 'node')

test('derives every path from the shell location (no machine literals)', () => {
  const config = resolveShellConfig({}, SHELL, NODE_OPTS)
  const root = join(SHELL, '..')
  assert.equal(config.root, root)
  assert.equal(config.home, join(root, 'cicada-dev-home'))
  assert.equal(config.launcherEntry, join(root, 'cicada-harness', 'packages', 'cicada', 'cicada-launcher', 'bin', 'cicada-app.ts'))
  assert.equal(config.launcherCwd, join(root, 'cicada-harness'))
  assert.equal(config.engineExe, join(root, 'cicada-editor', 'build-msvc-kicad', 'Release', 'cicada-engine.exe'))
  assert.equal(config.libDir, join(root, 'assets', 'cicada-libs'))
  assert.equal(config.userLibDir, join(config.home, 'cicada-user-lib'))
  assert.equal(config.brandIcon, join(root, 'assets', 'brand', 'cicada-64.png'))
  assert.equal(config.trayIcon, join(root, 'assets', 'brand', 'cicada.ico'))
  assert.equal(config.launchLog, join(config.home, 'logs', 'launch.txt'))
  assert.ok(!JSON.stringify(config).includes('C:\\dsh'), 'derived config must not embed a machine path')
})

test('environment overrides win over derived defaults', () => {
  const config = resolveShellConfig({
    CICADA_ROOT: '/elsewhere',
    DSH_HOME: '/home-dsh',
    CICADA_ENGINE_EXE: '/e/engine.exe',
    CICADA_LIB_DIR: '/e/libs',
    CICADA_USER_LIB_DIR: '/e/user',
    CICADA_BRAND_ICON: '/e/icon.png',
    CICADA_BRAND_MARK: '/e/mark.png',
    CICADA_TRAY_ICON: '/e/tray.ico',
    CICADA_LAUNCHER_ENTRY: '/e/launcher.js',
    CICADA_LAUNCHER_CWD: '/e/cwd',
  }, SHELL, NODE_OPTS)
  assert.equal(config.root, '/elsewhere')
  assert.equal(config.home, '/home-dsh')
  assert.equal(config.engineExe, '/e/engine.exe')
  assert.equal(config.libDir, '/e/libs')
  assert.equal(config.userLibDir, '/e/user')
  assert.equal(config.brandMark, '/e/mark.png')
  assert.equal(config.trayIcon, '/e/tray.ico')
  assert.equal(config.launcherEntry, '/e/launcher.js')
  assert.equal(config.launcherCwd, '/e/cwd')
})

test('CICADA_HOME takes precedence over DSH_HOME', () => {
  const config = resolveShellConfig({ CICADA_HOME: '/cicada-home', DSH_HOME: '/dsh-home' }, SHELL, NODE_OPTS)
  assert.equal(config.home, '/cicada-home')
})

test('node runtime: PATH probe, then the CICADA_NODE_BIN override', () => {
  const probed = resolveNodeBin({ PATH: '/usr/bin:/bin' }, NODE_OPTS)
  assert.deepEqual(probed, { bin: NODE, source: 'PATH' })
  assert.deepEqual(resolveShellConfig({}, SHELL, NODE_OPTS).nodeBin, NODE)

  const overridden = resolveShellConfig({ CICADA_NODE_BIN: '/bundle/node.exe' }, SHELL, NODE_OPTS)
  assert.equal(overridden.nodeBin, '/bundle/node.exe')
  assert.equal(overridden.nodeSource, 'CICADA_NODE_BIN')
})

test('node runtime: an empty PATH entry is skipped and a miss is reported, not guessed', () => {
  assert.deepEqual(resolveNodeBin({ PATH: ';;/opt/bin' }, { exists: () => false, separator: ':', exeName: 'node' }), { bin: '', source: 'missing' })
  const config = resolveShellConfig({ PATH: '' }, SHELL, { exists: () => false, separator: ':', exeName: 'node' })
  assert.equal(config.nodeBin, '')
  assert.equal(config.nodeSource, 'missing')
})

test('launcherEnv injects the resolved product variables without mutating the base', () => {
  const base = { PATH: '/bin', CICADA_LIB_DIR: '/stale' }
  const config = resolveShellConfig({}, SHELL, NODE_OPTS)
  const env = launcherEnv(config, base)
  assert.equal(env.PATH, '/bin')
  assert.equal(env.CICADA_HOME, config.home)
  assert.equal(env.DSH_HOME, config.home)
  assert.equal(env.CICADA_LIB_DIR, config.libDir)
  assert.equal(env.CICADA_ENGINE_EXE, config.engineExe)
  assert.equal(env.CICADA_USER_LIB_DIR, config.userLibDir)
  assert.equal(env.CICADA_BRAND_ICON, config.brandIcon)
  assert.equal(env.CICADA_BRAND_MARK, config.brandMark)
  assert.equal('ELECTRON_RUN_AS_NODE' in env, false, 'the host must run on stock Node, never Electron-as-Node')
  assert.equal(base.CICADA_LIB_DIR, '/stale', 'base environment is left untouched')
})

test('preloadFor: TypeScript entries load through tsx, built entries through none', () => {
  assert.equal(preloadFor('/x/cicada-app.ts'), 'tsx/esm')
  assert.equal(preloadFor('/x/cicada-app.mts'), 'tsx/esm')
  assert.equal(preloadFor('/x/cicada-app.js'), undefined)
  assert.equal(preloadFor('/x/cicada-app.mjs'), undefined)
})

test('preflight passes when every prerequisite exists', () => {
  const config = resolveShellConfig({}, SHELL, NODE_OPTS)
  assert.deepEqual(preflight(config, () => true), [])
})

test('preflight reports each missing piece with its path', () => {
  const config = resolveShellConfig({}, SHELL, NODE_OPTS)
  const problems = preflight(config, (path) => path !== config.engineExe)
  assert.equal(problems.length, 1)
  assert.match(problems[0], /引擎二进制缺失/)
  assert.ok(problems[0].includes(config.engineExe))
})

test('missingEngineRuntime: app-local first, then System32', () => {
  const config = resolveShellConfig({}, SHELL, NODE_OPTS)
  assert.deepEqual(missingEngineRuntime(config, () => true, ''), [])
  assert.deepEqual(missingEngineRuntime(config, () => false, ''), [...ENGINE_RUNTIME_DLLS])

  const appLocal = join(config.engineDir, 'MSVCP140.dll')
  const system32 = join('/windows', 'System32', 'VCRUNTIME140.dll')
  const exists = (path) => path === appLocal || path === system32
  assert.deepEqual(missingEngineRuntime(config, exists, '/windows'), ['MSVCP140_ATOMIC_WAIT.dll', 'VCRUNTIME140_1.dll'])
})

test('preflight reports engine runtime DLLs that are reachable nowhere', () => {
  const config = resolveShellConfig({}, SHELL, NODE_OPTS)
  const problems = preflight(config, (path) => path !== join(config.engineDir, 'MSVCP140.dll'), '')
  assert.equal(problems.length, 1)
  assert.match(problems[0], /引擎运行库缺失：MSVCP140\.dll/)
  assert.match(problems[0], /stage-engine-runtime\.bat/)
})

test('preflight reports a missing node runtime and a dangling CICADA_NODE_BIN', () => {
  const missing = preflight(resolveShellConfig({ PATH: '' }, SHELL, { exists: () => false, separator: ':', exeName: 'node' }), () => true)
  assert.equal(missing.length, 1)
  assert.match(missing[0], /未找到 Node 运行时/)

  const dangling = preflight(resolveShellConfig({ CICADA_NODE_BIN: '/bundle/node.exe' }, SHELL, NODE_OPTS), () => false)
  assert.equal(dangling.length, 3)
  assert.match(dangling[0], /CICADA_NODE_BIN 指向的文件不存在/)
})
