/**
 * Schematic-Cicada product shell — Electron main process (docs/04 §2).
 *
 * The shell does NOT re-implement the launcher: it spawns the existing
 * `cicada-app` launcher (single-instance lock → engine → host → Edge `--app`
 * window → teardown), observes its stdout protocol, and adds what a windowless
 * launcher cannot: a single-instance tray app, a status window for the startup
 * phase and failures, and a guaranteed `taskkill /T` of the whole child tree on
 * exit (Windows has no graceful child signal, and an orphaned host keeps port
 * 3123 forever — AGENTS §4-3).
 *
 * This batch deliberately does not embed the page: the product window is still
 * the Edge `--app` window opened by the launcher (docs/04 §2, §2.5).
 */

import { app, BrowserWindow, Menu, Tray, dialog, ipcMain, nativeImage, shell } from 'electron'
import { readdirSync, readFileSync, statSync } from 'node:fs'
import { join } from 'node:path'
import { preflight, resolveShellConfig, SHELL_DIR } from './config.js'
import { managerRequest, startManager } from './manager-client.js'
import { createProject, listProjects } from './projects.js'
import { runSmoke } from './smoke.js'
import { collectUiSmoke, defaultUiReportPath } from './ui-smoke.js'
import { applyLauncherExit, consumeLine, initialState, redactSecrets } from './status.js'
import { readLogTail, spawnLauncher } from './stack.js'

/** How long the status window lingers after the stack becomes ready. */
const READY_LINGER_MS = 2500

/** Declared here so the single-instance early exit below stays readable. */
const config = resolveShellConfig()

/** Current observed status (launcher is the authority; this is a projection). */
let state = initialState()
/** @type {BrowserWindow | null} */
let statusWindow = null
/** Main launcher window (docs/04 §5) — the app's primary surface. */
let launcherWindow = null
/** Project directory the current stack was started for ('' when idle). */
let activeProject = ''
/** @type {Tray | null} */
let tray = null
/** @type {{ pid?: number, child: import('node:child_process').ChildProcess, stop: () => void } | null} */
let launcher = null
/** True once quitting: window close becomes a real close, not a hide. */
let quitting = false

/**
 * `--smoke[=<report.json>]`: headless stack verification — no tray, no window,
 * the launcher runs with `--no-window`, the report goes to a file because
 * Electron's stdout is detached from the console on Windows (docs/06 §3).
 */
const smokeArg = process.argv.find((arg) => arg === '--smoke' || arg.startsWith('--smoke='))
/**
 * `--smoke-ui[=<report.json>]`: render every launcher page in a HIDDEN real
 * window and read the DOM back. The unit suite can only exercise a stub, so a
 * real-DOM-only failure (an empty page) would otherwise reach the user first.
 */
const uiSmokeArg = process.argv.find((arg) => arg === '--smoke-ui' || arg.startsWith('--smoke-ui='))

if (uiSmokeArg !== undefined) {
  const equals = uiSmokeArg.indexOf('=')
  const target = equals < 0 ? defaultUiReportPath(SHELL_DIR) : uiSmokeArg.slice(equals + 1)
  app.whenReady().then(async () => {
    const report = await collectUiSmoke(async () => {
      createLauncherWindow({ visible: false })
      const window = launcherWindow
      await new Promise((resolve) => window.webContents.once('did-finish-load', resolve))
      return {
        window,
        clickPage: async (page) => {
          await window.webContents.executeJavaScript(
            `document.querySelector('nav button[data-page="${page}"]').click(); true`,
          )
          await new Promise((resolve) => setTimeout(resolve, 250))
        },
        readPage: () => window.webContents.executeJavaScript("document.getElementById('main').innerText"),
        close: () => { quitting = true; window.destroy() },
      }
    }, target)
    process.stdout.write(`${JSON.stringify(report, null, 2)}\n`)
    app.exit(report.ok ? 0 : 1)
  }).catch((error) => {
    process.stderr.write(`ui-smoke failed: ${String(error)}\n`)
    app.exit(2)
  })
} else if (smokeArg !== undefined) {
  const equals = smokeArg.indexOf('=')
  const target = equals < 0 ? join(SHELL_DIR, 'logs', 'shell-smoke.json') : smokeArg.slice(equals + 1)
  void runSmoke(config, target).then((report) => { app.exit(report.ok ? 0 : 1) })
} else if (!app.requestSingleInstanceLock()) {
  // A second launch focuses the running shell instead of racing for port 3123.
  app.quit()
} else {
  // Re-launching is also how a user applies an updated UI to a shell that has
  // been sitting in the tray: focus it AND reload the page from disk.
  app.on('second-instance', () => {
    showLauncher()
    launcherWindow?.webContents.reload()
  })
  app.whenReady().then(start).catch((error) => {
    fail(`壳启动失败：${error instanceof Error ? error.message : String(error)}`)
  })
  // Tray app: closing a window must not end the process.
  app.on('window-all-closed', () => {})
  app.on('before-quit', () => {
    quitting = true
    stopManager()
    stopLauncher()
  })
}

/**
 * Boot the launcher window and the tray. The stack itself starts only when the
 * user picks a project (docs/04 §5.2): the launcher window is the mandatory
 * first step, not an observer of an already-running stack.
 */
function start() {
  app.setName('Schematic-Cicada')
  app.setAppUserModelId('com.schematic-cicada.shell')
  createLauncherWindow()
  createTray()
  const problems = preflight(config)
  if (problems.length > 0) {
    update({ phase: 'failed', problems, message: '启动前检查未通过（缺少的文件见下）' })
  }
}

/** One launcher stdout/stderr line → state (token-redacted) → both windows. */
function onLauncherLine(rawLine) {
  const wasReady = state.phase === 'ready'
  pushConsole(rawLine)
  state = consumeLine(state, rawLine)
  publish()
  if (!wasReady && state.phase === 'ready') {
    // The product window is up: step out of the way (docs/04 §5.2) by
    // MINIMIZING — hiding to the tray made the shell look like it had
    // disappeared (user feedback 2026-09-12). The tray entry stays.
    setTimeout(() => { statusWindow?.hide(); launcherWindow?.minimize() }, READY_LINGER_MS)
  }
}

/**
 * Launcher exit. A clean close of the product window ends only the STACK: with
 * a launcher window in front, the user must be able to start another project
 * (docs/04 §5.2 拍板①), so the shell stays alive and shows the launcher again.
 */
function onLauncherExit(code) {
  launcher = null
  const failed = state.phase === 'failed' || (code !== 0 && code !== null)
  activeProject = ''
  state = applyLauncherExit(state, code)
  publish()
  showLauncher()
  if (failed) showStatus()
}

/** Merge a patch into the status state and republish. */
function update(patch) {
  state = { ...state, ...patch }
  publish()
}

/** Report a shell-level failure (no launcher involved). */
function fail(message) {
  update({ phase: 'failed', message })
  showLauncher()
}

/** Renderer-facing view: never expose the launch URL (it carries the token). */
function view() {
  return {
    phase: state.phase,
    message: state.message,
    enginePort: state.enginePort,
    hasWindow: state.windowUrl !== null,
    problems: [...state.problems],
    console: [...consoleLines],
    project: activeProject,
    stamp: STAMP,
    exitCode: state.exitCode,
    logTail: readLogTail(config.launchLog),
  }
}

/**
 * Build stamp of the launcher UI: the newest mtime among this directory's
 * sources. A shell that keeps running while the files change is the normal case
 * (single instance, tray resident), and its window would otherwise look
 * identical to a freshly started one — this makes "you are looking at an old
 * instance" visible instead of mysterious.
 */
function sourceStamp() {
  let newest = 0
  try {
    for (const name of readdirSync(import.meta.dirname)) {
      if (!/\.(js|cjs|html|css)$/.test(name)) continue
      newest = Math.max(newest, statSync(join(import.meta.dirname, name)).mtimeMs)
    }
  } catch {
    // A stamp is diagnostics; an unreadable directory must not break the shell.
  }
  if (newest === 0) return 'unknown'
  const when = new Date(newest)
  const pad = (value) => String(value).padStart(2, '0')
  return `${when.getFullYear()}-${pad(when.getMonth() + 1)}-${pad(when.getDate())} ${pad(when.getHours())}:${pad(when.getMinutes())}`
}

/** Cached UI build stamp. */
const STAMP = sourceStamp()

/** Lines the run console keeps (the tray panel keeps its own short tail). */
const CONSOLE_MAX = 1000

/** Streamed, already-redacted launcher output for the run page. */
let consoleLines = []

/** Append one line to the console buffer (bounded). */
function pushConsole(line) {
  consoleLines.push(redactSecrets(line))
  if (consoleLines.length > CONSOLE_MAX) consoleLines = consoleLines.slice(-CONSOLE_MAX)
}

/** Push the current view to whichever windows exist. */
function publish() {
  const snapshot = view()
  // The tray panel renders one log area: it keeps the short tail it always had,
  // while the launcher window gets the full console stream.
  statusWindow?.webContents.send('shell:state', { ...snapshot, problems: [...state.problems, ...state.lines] })
  launcherWindow?.webContents.send('shell:state', snapshot)
}

/**
 * Start the supervised stack for one project directory (docs/04 §5.2). The
 * project path travels as `--workspace`; the launcher makes it the host's cwd
 * and the client's initial workspace, and DSH registers it on first use (the
 * launcher never writes the workspace registry itself).
 * @param projectPath - absolute project directory.
 * @returns `{ ok, problem }`.
 */
function startStack(projectPath) {
  if (launcher !== null) return { ok: false, problem: '已有栈在运行，先停止再启动' }
  const problems = preflight(config)
  if (problems.length > 0) {
    update({ phase: 'failed', problems, message: '启动前检查未通过（缺少的文件见下）' })
    return { ok: false, problem: problems.join('；') }
  }
  activeProject = projectPath
  consoleLines = []
  pushConsole(`—— 启动 ${projectPath === '' ? '（launcher 默认 cwd）' : projectPath} ——`)
  update({ phase: 'starting', message: '正在启动本地引擎与 DSH 主机…', problems: [] })
  try {
    launcher = spawnLauncher(config, {
      onLine: onLauncherLine,
      onExit: onLauncherExit,
      ...(projectPath === '' ? {} : { extraArgs: ['--workspace', projectPath] }),
    })
    return { ok: true, problem: '' }
  } catch (error) {
    const message = error instanceof Error ? error.message : String(error)
    update({ phase: 'failed', message: `启动器进程创建失败：${message}` })
    return { ok: false, problem: message }
  }
}

/** Kill the stack (taskkill /T through the launcher child). */
function stopStack() {
  stopLauncher()
  update({ phase: 'exited', message: '已停止' })
}

/** Read-only configuration rows for the settings page. */
function settingsRows() {
  return [
    ['启动器界面版本', STAMP, true, join(SHELL_DIR, 'src')],
    ['CICADA_HOME', config.home, true, config.home],
    ['项目根目录', config.projectsDir, true, config.projectsDir],
    ['工作区注册表（只读）', config.workspaceRegistry, true, join(config.workspaceRegistry, '..')],
    ['回收站', config.trashDir, true, config.trashDir],
    ['精选符号库（只读）', config.libDir, true, config.libDir],
    ['用户符号库（可写）', config.userLibDir, true, config.userLibDir],
    ['引擎', config.engineExe, true, config.engineDir],
    ['Node 运行时', `${config.nodeBin || '（未找到）'}（${config.nodeSource}）`, true, config.nodeBin || config.root],
    ['启动器入口', config.launcherEntry, true, join(config.launcherEntry, '..')],
    ['日志', config.launchLog, true, join(config.home, 'logs')],
  ]
}

/** Small always-available status panel (tray-owned; never in the taskbar). */
function createStatusWindow() {
  statusWindow = new BrowserWindow({
    width: 520,
    height: 420,
    show: false,
    skipTaskbar: true,
    autoHideMenuBar: true,
    title: 'Schematic-Cicada',
    icon: config.trayIcon,
    webPreferences: {
      preload: join(import.meta.dirname, 'status-preload.cjs'),
      contextIsolation: true,
      nodeIntegration: false,
      sandbox: false,
      devTools: false,
    },
  })
  statusWindow.setMenuBarVisibility(false)
  statusWindow.on('close', (event) => {
    if (quitting) return
    event.preventDefault()
    statusWindow?.hide()
  })
  statusWindow.on('closed', () => { statusWindow = null })
  void statusWindow.loadFile(join(import.meta.dirname, 'status.html'))
  statusWindow.webContents.on('did-finish-load', () => publish())
}

/** The launcher: the app's primary window (docs/04 §5.1). */
function createLauncherWindow(options = {}) {
  const visible = options.visible !== false
  launcherWindow = new BrowserWindow({
    width: 1000,
    height: 640,
    minWidth: 720,
    minHeight: 480,
    show: false,
    autoHideMenuBar: true,
    backgroundColor: '#14161a',
    title: `Schematic-Cicada 启动器 · ${STAMP}`,
    icon: config.trayIcon,
    webPreferences: {
      preload: join(import.meta.dirname, 'launcher-preload.cjs'),
      contextIsolation: true,
      nodeIntegration: false,
      sandbox: false,
      devTools: false,
      additionalArguments: [`--cicada-mark=${markDataUrl()}`],
    },
  })
  launcherWindow.setMenuBarVisibility(false)
  launcherWindow.on('close', (event) => {
    if (quitting) return
    event.preventDefault()
    launcherWindow?.hide()
  })
  launcherWindow.on('closed', () => { launcherWindow = null })
  void launcherWindow.loadFile(join(import.meta.dirname, 'launcher.html'))
  launcherWindow.webContents.on('did-finish-load', () => {
    publish()
    if (visible) launcherWindow?.show()
  })
}

/** Brand mark as a data URL: the renderer has no file:// access by policy. */
function markDataUrl() {
  try {
    const bytes = readFileSync(config.brandMark)
    return `data:image/png;base64,${bytes.toString('base64')}`
  } catch {
    // Missing brand asset is cosmetic; the window renders without the mark.
    return ''
  }
}

/** Tray icon + menu (background presence while the product window runs). */
function createTray() {
  const image = iconImage()
  tray = new Tray(image)
  tray.setToolTip('Schematic-Cicada')
  tray.setContextMenu(Menu.buildFromTemplate([
    { label: '打开启动器', click: () => showLauncher() },
    { label: '打开产品窗口', click: () => openProductWindow() },
    { label: '停止栈', click: () => stopStack() },
    { type: 'separator' },
    { label: '快捷状态…', click: () => showStatus() },
    { label: '打开日志目录', click: () => openLogs() },
    { label: '退出', click: () => quit() },
  ]))
  tray.on('click', () => showLauncher())
}

/** Product icon, falling back to the brand PNG when the `.ico` is missing. */
function iconImage() {
  const ico = nativeImage.createFromPath(config.trayIcon)
  if (!ico.isEmpty()) return ico
  const png = nativeImage.createFromPath(config.brandIcon)
  return png.isEmpty() ? nativeImage.createEmpty() : png
}

/** Show (and republish to) the launcher window. */
function showLauncher() {
  if (launcherWindow === null) {
    if (!quitting) createLauncherWindow()
    return
  }
  launcherWindow.show()
  launcherWindow.focus()
  publish()
}

/** Show (and republish to) the tray's quick status panel. */
function showStatus() {
  if (statusWindow === null) {
    if (!quitting) createStatusWindow()
  }
  statusWindow?.show()
  statusWindow?.focus()
  publish()
}

/** Open the product page in the default browser (Edge-missing fallback). */
function openProductWindow() {
  if (state.windowUrl !== null) {
    void shell.openExternal(state.windowUrl)
    return
  }
  showLauncher()
}

/** Reveal `logs/` (launch.txt) in Explorer. */
function openLogs() {
  void shell.openPath(join(config.home, 'logs'))
}

/** Quit: `before-quit` tears the child tree down. */
function quit() {
  quitting = true
  app.quit()
}

/** Kill launcher + host + engine synchronously (safe to call twice). */
function stopLauncher() {
  if (launcher === null) return
  const running = launcher
  launcher = null
  running.stop()
}

/**
 * The datasheet manager (docs/04 §5.5) exists only while a management page asks
 * for it: lazy start, killed with the shell. `starting` de-duplicates concurrent
 * first calls (two page loads must not spawn two services).
 */
let manager = null
let managerStarting = null

/** Start (once) and return the manager session, or a readable problem. */
async function ensureManager() {
  if (manager !== null && manager.ok) return manager
  if (managerStarting !== null) return managerStarting
  managerStarting = startManager(config, {
    onExit: () => { manager = null; managerStarting = null },
  }).then((session) => {
    manager = session
    managerStarting = null
    return session
  })
  return managerStarting
}

/** Stop the manager process (idempotent). */
function stopManager() {
  manager?.stop()
  manager = null
  managerStarting = null
}

/**
 * Run one manager call for the renderer.
 * @param path - manager path.
 * @param options - `{ method, body }`.
 * @returns `{ ok, data, problem }`.
 */
async function callManager(path, options = {}) {
  const session = await ensureManager()
  if (!session.ok) return { ok: false, data: undefined, problem: session.problem }
  const result = await managerRequest(session, path, options)
  if (!result.ok) {
    // A dead service must not stay cached: the next call restarts it.
    if (result.status === 0) stopManager()
    return { ok: false, data: result.data, problem: result.problem }
  }
  return { ok: true, data: result.data, problem: '' }
}

ipcMain.on('shell:open', () => openProductWindow())
ipcMain.on('shell:logs', () => openLogs())
ipcMain.on('shell:quit', () => quit())

// ── launcher window surface (docs/04 §5.1) ─────────────────────────────────
ipcMain.handle('launcher:list', () => {
  const listed = listProjects(config)
  return { ...listed, config: { projectsDir: config.projectsDir, home: config.home } }
})
ipcMain.handle('launcher:create', (_event, name) => createProject(config, String(name ?? '')))
ipcMain.handle('launcher:settings', () => ({
  projectsDir: config.projectsDir,
  home: config.home,
  rows: settingsRows(),
}))
ipcMain.handle('launcher:start', (_event, path) => startStack(typeof path === 'string' ? path : ''))
ipcMain.handle('launcher:stop', () => { stopStack(); return { ok: true } })
ipcMain.handle('launcher:pick', async () => {
  const picked = await dialog.showOpenDialog(launcherWindow ?? undefined, {
    title: '选择项目目录',
    properties: ['openDirectory', 'createDirectory'],
  })
  if (picked.canceled || picked.filePaths.length === 0) return { ok: false, canceled: true, problem: '' }
  return { ok: true, canceled: false, path: picked.filePaths[0], problem: '' }
})
ipcMain.on('launcher:product', () => openProductWindow())
ipcMain.on('launcher:logs', () => openLogs())
ipcMain.on('launcher:reveal', (_event, path) => {
  if (typeof path === 'string' && path !== '') void shell.openPath(path)
})

// ── datasheet library page (docs/04 §5.3) ──────────────────────────────────
ipcMain.handle('launcher:datasheets', async () => {
  const result = await callManager('/datasheets')
  return result.ok
    ? { ok: true, entries: result.data?.entries ?? [], problems: result.data?.problems ?? [] }
    : { ok: false, entries: [], problems: [result.problem] }
})
ipcMain.handle('launcher:datasheet', async (_event, part) => {
  const result = await callManager(`/datasheets/${encodeURIComponent(String(part ?? ''))}`)
  return result.ok ? { ok: true, detail: result.data } : { ok: false, problems: [result.problem] }
})
ipcMain.handle('launcher:datasheet-trash', async (_event, part) => {
  const name = String(part ?? '')
  const confirmed = await dialog.showMessageBox(launcherWindow ?? undefined, {
    type: 'warning',
    buttons: ['移入回收站', '取消'],
    defaultId: 1,
    cancelId: 1,
    title: '删除数据手册条目',
    message: `把「${name}」移入回收站？`,
    detail: `条目会被移动到 ${config.trashDir}，可以随时手工还原——不会被真正删除。`,
  })
  if (confirmed.response !== 0) return { ok: false, canceled: true, problem: '' }
  const result = await callManager(`/datasheets/${encodeURIComponent(name)}/trash`, { method: 'POST' })
  return result.ok
    ? { ok: true, canceled: false, movedTo: result.data?.movedTo ?? '', problem: '' }
    : { ok: false, canceled: false, problem: result.problem }
})
ipcMain.handle('launcher:datasheet-export', async (_event, part) => {
  const picked = await dialog.showOpenDialog(launcherWindow ?? undefined, {
    title: '导出到目录',
    properties: ['openDirectory', 'createDirectory'],
  })
  if (picked.canceled || picked.filePaths.length === 0) return { ok: false, canceled: true, problem: '' }
  const result = await callManager(`/datasheets/${encodeURIComponent(String(part ?? ''))}/export`, {
    method: 'POST',
    body: { targetDir: picked.filePaths[0] },
  })
  return result.ok
    ? { ok: true, canceled: false, target: result.data?.target ?? '', problem: '' }
    : { ok: false, canceled: false, problem: result.problem }
})
ipcMain.on('launcher:trash-open', () => { void shell.openPath(config.trashDir) })
ipcMain.on('launcher:console-clear', () => {
  consoleLines = []
  publish()
})

// ── symbol library page (docs/04 §5.4) ─────────────────────────────────────
ipcMain.handle('launcher:symbols', async () => {
  const result = await callManager('/symbols')
  return result.ok
    ? { ok: true, curated: result.data?.curated ?? [], user: result.data?.user ?? [], problems: result.data?.problems ?? [], engine: result.data?.engine === true }
    : { ok: false, curated: [], user: [], problems: [result.problem], engine: false }
})
ipcMain.handle('launcher:symbol', async (_event, source, key) => {
  const path = `/symbols/${source === 'user' ? 'user' : 'curated'}/${encodeURIComponent(String(key ?? ''))}`
  const result = await callManager(path)
  return result.ok ? { ok: true, detail: result.data } : { ok: false, problems: [result.problem] }
})
ipcMain.handle('launcher:symbol-trash', async (_event, key) => {
  const confirmed = await dialog.showMessageBox(launcherWindow ?? undefined, {
    type: 'warning',
    buttons: ['移入回收站', '取消'],
    defaultId: 1,
    cancelId: 1,
    title: '删除用户库符号',
    message: `把用户库符号「${key}」移入回收站？`,
    detail: `文件会被移动到 ${config.trashDir}，可以随时手工还原——不会被真正删除。精选库是只读的。`,
  })
  if (confirmed.response !== 0) return { ok: false, canceled: true, problem: '' }
  const result = await callManager(`/symbols/user/${encodeURIComponent(String(key ?? ''))}/trash`, { method: 'POST' })
  return result.ok
    ? { ok: true, canceled: false, movedTo: result.data?.movedTo ?? '', problem: '' }
    : { ok: false, canceled: false, problem: result.problem }
})
ipcMain.handle('launcher:symbol-export', async (_event, source, key) => {
  const picked = await dialog.showOpenDialog(launcherWindow ?? undefined, {
    title: '导出符号文件',
    properties: ['openDirectory', 'createDirectory'],
  })
  if (picked.canceled || picked.filePaths.length === 0) return { ok: false, canceled: true, problem: '' }
  const result = await callManager(`/symbols/${source === 'user' ? 'user' : 'curated'}/${encodeURIComponent(String(key ?? ''))}/export`, {
    method: 'POST',
    body: { targetDir: picked.filePaths[0] },
  })
  return result.ok
    ? { ok: true, canceled: false, target: result.data?.target ?? '', problem: '' }
    : { ok: false, canceled: false, problem: result.problem }
})
ipcMain.handle('launcher:symbol-import', async (_event, category) => {
  const picked = await dialog.showOpenDialog(launcherWindow ?? undefined, {
    title: '导入 .kicad_sym',
    properties: ['openFile'],
    filters: [{ name: 'KiCad 符号库', extensions: ['kicad_sym'] }],
  })
  if (picked.canceled || picked.filePaths.length === 0) return { ok: false, canceled: true, problem: '' }
  let result = await callManager('/symbols/import', {
    method: 'POST',
    body: { sourceFile: picked.filePaths[0], category: String(category ?? 'IC') },
  })
  if (!result.ok && String(result.problem).includes('需要显式覆盖')) {
    const confirmed = await dialog.showMessageBox(launcherWindow ?? undefined, {
      type: 'question',
      buttons: ['覆盖', '取消'],
      defaultId: 1,
      cancelId: 1,
      title: '覆盖用户库符号',
      message: String(result.problem),
      detail: '覆盖会用导入的文件替换用户库里已有的同名符号（原文件不备份，请自行确认）。',
    })
    if (confirmed.response !== 0) return { ok: false, canceled: true, problem: '' }
    result = await callManager('/symbols/import', {
      method: 'POST',
      body: { sourceFile: picked.filePaths[0], category: String(category ?? 'IC'), overwrite: true },
    })
  }
  return result.ok
    ? { ok: true, canceled: false, key: result.data?.key ?? '', target: result.data?.target ?? '', problem: '' }
    : { ok: false, canceled: false, problem: result.problem }
})
ipcMain.handle('launcher:symbol-rebuild', async (_event, part) => {
  const result = await callManager(`/datasheets/${encodeURIComponent(String(part ?? ''))}/rebuild`, { method: 'POST' })
  return result.ok
    ? { ok: true, key: result.data?.key ?? '', warnings: result.data?.warnings ?? [], problem: '' }
    : { ok: false, problem: result.problem }
})
ipcMain.handle('launcher:symbol-save-to-user', async (_event, key) => {
  const result = await callManager(`/symbols/curated/${encodeURIComponent(String(key ?? ''))}/save-to-user`, { method: 'POST' })
  return result.ok
    ? { ok: true, target: result.data?.target ?? '', problem: '' }
    : { ok: false, problem: result.problem }
})
