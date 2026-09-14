/**
 * Shell configuration (docs/04 §2.2): every path is derived from the shell's
 * own location (`cicada-shell/`) or an environment override — never from a
 * machine-specific literal (AGENTS §2.2). The derived defaults mirror the dev
 * values of `cicada-editor/scripts/start-cicada-app.bat` so the shell is
 * self-sufficient when launched without that wrapper.
 */

import { existsSync } from 'node:fs'
import { join } from 'node:path'

/** `cicada-shell/` directory (this module lives in `cicada-shell/src`). */
export const SHELL_DIR = join(import.meta.dirname, '..')

/** Dev preload used when the launcher entry is TypeScript source (tsx). */
export const TS_PRELOAD = 'tsx/esm'

/** Node executable name per platform (PATH probe + preflight messages). */
export const NODE_EXE = process.platform === 'win32' ? 'node.exe' : 'node'

/**
 * Locate the Node runtime for the launcher and the DSH host.
 *
 * The host CANNOT run on Electron's bundled Node: the harness loader needs
 * either `--expose-internals` in the host's own execArgv or its native addon
 * (`node-addon-require-builtin`), and a native addon built for Node's ABI does
 * not load inside Electron — measured 2026-09-09: Electron 39 (Node 22.22.1)
 * failed the `cordis-plugin-hmr` entry with `--expose-internals is required
 * for HMR service`, while the same stack passes on the system Node 24.19.
 * Hence the product ships a standalone `node.exe` (user decision ②) and dev
 * resolves `node` from PATH (docs/04 §2.1).
 * @param env - environment variables (defaults to `process.env`).
 * @param options - `exists`, `pathValue`, `exeName` and `separator` overrides for tests.
 * @returns `{ bin, source }`; `bin` is `''` (and `source` `'missing'`) when no
 *   Node runtime is found, which preflight reports.
 */
export function resolveNodeBin(env = process.env, options = {}) {
  const override = env.CICADA_NODE_BIN
  if (override !== undefined && override !== '') return { bin: override, source: 'CICADA_NODE_BIN' }
  const exists = options.exists ?? existsSync
  const separator = options.separator ?? (process.platform === 'win32' ? ';' : ':')
  const pathValue = options.pathValue ?? env.PATH ?? env.Path ?? ''
  for (const dir of pathValue.split(separator)) {
    if (dir === '') continue
    const candidate = join(dir, options.exeName ?? NODE_EXE)
    if (exists(candidate)) return { bin: candidate, source: 'PATH' }
  }
  return { bin: '', source: 'missing' }
}

/**
 * Resolve the shell configuration.
 * @param env - environment variables (defaults to `process.env`).
 * @param shellDir - shell directory (defaults to {@link SHELL_DIR}; injected by tests).
 * @param nodeOptions - {@link resolveNodeBin} overrides (injected by tests).
 * @returns resolved configuration.
 */
export function resolveShellConfig(env = process.env, shellDir = SHELL_DIR, nodeOptions = {}) {
  const root = env.CICADA_ROOT ?? join(shellDir, '..')
  const home = env.CICADA_HOME ?? env.DSH_HOME ?? join(root, 'cicada-dev-home')
  const node = resolveNodeBin(env, nodeOptions)
  const engineExe = env.CICADA_ENGINE_EXE ?? join(root, 'cicada-editor', 'build-msvc-kicad', 'Release', 'cicada-engine.exe')
  return {
    root,
    shellDir,
    home,
    nodeBin: node.bin,
    nodeSource: node.source,
    launcherEntry: env.CICADA_LAUNCHER_ENTRY
      ?? join(root, 'cicada-harness', 'packages', 'cicada', 'cicada-launcher', 'bin', 'cicada-app.ts'),
    launcherCwd: env.CICADA_LAUNCHER_CWD ?? join(root, 'cicada-harness'),
    engineExe,
    engineDir: join(engineExe, '..'),
    libDir: env.CICADA_LIB_DIR ?? join(root, 'assets', 'cicada-libs'),
    userLibDir: env.CICADA_USER_LIB_DIR ?? join(home, 'cicada-user-lib'),
    brandIcon: env.CICADA_BRAND_ICON ?? join(root, 'assets', 'brand', 'cicada-64.png'),
    brandMark: env.CICADA_BRAND_MARK ?? join(root, 'assets', 'brand', 'cicada-mark.png'),
    trayIcon: env.CICADA_TRAY_ICON ?? join(root, 'assets', 'brand', 'cicada.ico'),
    launchLog: join(home, 'logs', 'launch.txt'),
    // docs/04 §5: launcher project model. `workspaceRegistry` is DSH's own
    // registry (read-only for us); `projectsDir` is where new projects are made.
    projectsDir: env.CICADA_PROJECTS_DIR ?? join(home, 'projects'),
    workspaceRegistry: join(home, 'storages', 'workspace.json'),
    trashDir: env.CICADA_TRASH_DIR ?? join(home, 'trash'),
  }
}

/**
 * The `--import` preload a launcher entry needs: TypeScript entries run through
 * tsx, built JavaScript entries need none (docs/04 §2.2).
 * @param entry - launcher entry path.
 * @returns the preload specifier, or undefined for plain JavaScript.
 */
export function preloadFor(entry) {
  return /\.[cm]?ts$/i.test(entry) ? TS_PRELOAD : undefined
}

/**
 * VC++ runtime DLLs `cicada-engine.exe` imports (`dumpbin /DEPENDENTS`,
 * measured 2026-09-09). UCRT (`api-ms-win-crt-*`) ships with Windows and is
 * deliberately not checked.
 */
export const ENGINE_RUNTIME_DLLS = Object.freeze([
  'MSVCP140.dll',
  'MSVCP140_ATOMIC_WAIT.dll',
  'VCRUNTIME140.dll',
  'VCRUNTIME140_1.dll',
])

/**
 * Which engine runtime DLLs are reachable neither app-local (next to the exe,
 * where Windows looks first — the packaged product's layout, docs/04 §2.5) nor
 * in `System32` (a machine with the redistributable installed).
 * @param config - resolved shell configuration.
 * @param exists - file-existence probe (injected by tests).
 * @param systemRoot - `SystemRoot` value (empty disables the System32 probe).
 * @returns the missing DLL names; empty means "loadable".
 */
export function missingEngineRuntime(config, exists = existsSync, systemRoot = process.env.SystemRoot ?? '') {
  const system32 = systemRoot === '' ? '' : join(systemRoot, 'System32')
  return ENGINE_RUNTIME_DLLS.filter((dll) =>
    !exists(join(config.engineDir, dll)) && !(system32 !== '' && exists(join(system32, dll))))
}

/**
 * Preflight checks before spawning the stack (docs/04 §2.1-4): a missing piece
 * becomes a readable startup error in the status window instead of a child
 * process that dies without explanation.
 * @param config - resolved shell configuration.
 * @param exists - file-existence probe (injected by tests).
 * @param systemRoot - `SystemRoot` value (see {@link missingEngineRuntime}).
 * @returns one message per missing prerequisite; empty means "go".
 */
export function preflight(config, exists = existsSync, systemRoot = process.env.SystemRoot ?? '') {
  const problems = []
  if (config.nodeBin === '') {
    problems.push(`未找到 Node 运行时（CICADA_NODE_BIN 可覆盖；开发机装 Node ≥22，产品随包 node.exe）：${NODE_EXE}`)
  } else if (config.nodeSource === 'CICADA_NODE_BIN' && !exists(config.nodeBin)) {
    problems.push(`CICADA_NODE_BIN 指向的文件不存在：${config.nodeBin}`)
  }
  if (!exists(config.launcherEntry)) {
    problems.push(`启动器入口缺失：${config.launcherEntry}（CICADA_LAUNCHER_ENTRY 可覆盖）`)
  }
  if (!exists(config.engineExe)) {
    problems.push(`引擎二进制缺失：${config.engineExe}（CICADA_ENGINE_EXE 可覆盖；先跑 cicada-editor/scripts/m1b-rebuild.bat）`)
  } else {
    const missing = missingEngineRuntime(config, exists, systemRoot)
    if (missing.length > 0) {
      problems.push(`引擎运行库缺失：${missing.join('、')}（exe 同目录与系统都没有）——先跑 cicada-editor/scripts/stage-engine-runtime.bat，或安装 VC++ 2015-2022 运行库`)
    }
  }
  return problems
}

/**
 * Environment handed to the launcher child: the resolved product variables are
 * injected (never written back to the user environment, docs/04 §2.2).
 * @param config - resolved shell configuration.
 * @param base - base environment (defaults to `process.env`).
 * @returns the child environment.
 */
export function launcherEnv(config, base = process.env) {
  return {
    ...base,
    CICADA_HOME: config.home,
    DSH_HOME: config.home,
    CICADA_ENGINE_EXE: config.engineExe,
    CICADA_LIB_DIR: config.libDir,
    CICADA_USER_LIB_DIR: config.userLibDir,
    CICADA_BRAND_ICON: config.brandIcon,
    CICADA_BRAND_MARK: config.brandMark,
  }
}
