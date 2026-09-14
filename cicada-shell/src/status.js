/**
 * Shell status model (docs/04 §2.3): parse the launcher's stdout protocol into
 * a small state object the tray/status window renders, and redact the launch
 * token before any of it is shown or logged (AGENTS §1-3).
 *
 * The launcher keeps sole ownership of the stack; the shell only observes it.
 */

/** Launch-token query values (`?token=…`, `#token=…`) never leave the process. */
const TOKEN_PATTERN = /([?&#](?:token|t|key)=)[^&\s]+/gi
/** Announce lines carry a token in their last field: `cicada-editor: <port> <token>`. */
const EDITOR_ANNOUNCE = /^(cicada-editor:\s+\d+\s+)\S+/
/** `cicada-engine: 127.0.0.1:<port> <token>`. */
const ENGINE_ANNOUNCE = /^(cicada-engine:\s+127\.0\.0\.1:\d+\s+)\S+/

/** `cicada: 引擎端口 <port>` (bin/cicada-app.ts step ①). */
const ENGINE_PORT = /^cicada:\s*引擎端口\s+(\d+)\s*$/
/** `cicada: 窗口地址 <url>` (step ③, printed just before the Edge spawn). */
const WINDOW_URL = /^cicada:\s*窗口地址\s+(\S+)\s*$/
/** `dsh web: <url>` forwarded from the host child. */
const WEB_URL = /^dsh web:\s+(\S+)\s*$/
/** Another instance owns the stack (preflight or lock contention). */
const ALREADY_RUNNING = /^cicada:\s*(?:已有实例在运行|检测到启动中的实例)/
/** The launcher could not open Edge; the page URL must be opened by hand. */
const EDGE_MISSING = /^cicada:\s*未找到 Edge/
/** Terminal launcher failures (its own messages, before a non-zero exit). */
const FAILURE = /^cicada:\s*(引擎未就绪|host 启动超时|引擎启动失败|等待启动中的实例超时|实例锁被占用)/

/**
 * Redact URL credentials from one text line.
 * @param text - raw line.
 * @returns the line with token values replaced by `***`.
 */
export function redactSecrets(text) {
  const trimmed = text.endsWith('\r') ? text.slice(0, -1) : text
  return trimmed
    .replace(TOKEN_PATTERN, '$1***')
    .replace(EDITOR_ANNOUNCE, '$1***')
    .replace(ENGINE_ANNOUNCE, '$1***')
}

/**
 * Parse one launcher stdout line.
 * @param line - one complete line (trailing `\r` tolerated).
 * @returns `{ kind }` plus the field that kind carries: `engine-port` (port),
 *   `window-url` (url), `already-running`, `edge-missing`, `failure` (message),
 *   or `log` (text).
 */
export function parseLauncherLine(line) {
  const trimmed = line.endsWith('\r') ? line.slice(0, -1) : line
  const port = ENGINE_PORT.exec(trimmed)
  if (port !== null) return { kind: 'engine-port', port: Number.parseInt(port[1], 10) }
  const window = WINDOW_URL.exec(trimmed) ?? WEB_URL.exec(trimmed)
  if (window !== null) return { kind: 'window-url', url: window[1] }
  if (ALREADY_RUNNING.test(trimmed)) return { kind: 'already-running' }
  if (EDGE_MISSING.test(trimmed)) return { kind: 'edge-missing' }
  if (FAILURE.test(trimmed)) return { kind: 'failure', message: trimmed.replace(/^cicada:\s*/, '') }
  return { kind: 'log', text: trimmed }
}

/** Status phases: preflight → starting → ready | failed | blocked | exited. */
export const INITIAL_STATE = Object.freeze({
  phase: 'starting',
  message: '正在启动本地引擎与 DSH 主机…',
  enginePort: null,
  windowUrl: null,
  problems: [],
  lines: [],
  exitCode: null,
})

/** How many launcher lines the status window keeps (redacted). */
export const MAX_LINES = 12

/**
 * Fresh status state.
 * @returns a mutable copy of {@link INITIAL_STATE}.
 */
export function initialState() {
  return { ...INITIAL_STATE, problems: [], lines: [] }
}

/**
 * Fold one parsed launcher line into the state.
 * @param state - current state (not mutated).
 * @param parsed - result of {@link parseLauncherLine}.
 * @returns the next state.
 */
export function applyLauncherLine(state, parsed) {
  switch (parsed.kind) {
    case 'engine-port':
      return { ...state, enginePort: parsed.port, message: `引擎已就绪（端口 ${parsed.port}），正在启动 DSH 主机…` }
    case 'window-url':
      return { ...state, windowUrl: parsed.url, phase: 'ready', message: '已就绪：产品窗口已打开（关闭窗口即退出整个应用）。' }
    case 'already-running':
      return { ...state, phase: 'blocked', message: '已有实例在运行：请使用已打开的窗口；本壳未启动新的引擎与主机。' }
    case 'edge-missing':
      return { ...state, message: '未找到 Edge：可在状态窗点「打开产品窗口」用默认浏览器打开页面。' }
    case 'failure':
      return { ...state, phase: 'failed', message: parsed.message }
    default:
      return state
  }
}

/**
 * Append one already-redacted line to the retained tail.
 * @param state - current state (not mutated).
 * @param line - redacted text.
 * @param max - retained line count (defaults to {@link MAX_LINES}).
 * @returns the next state.
 */
export function appendLogLine(state, line, max = MAX_LINES) {
  if (line.trim() === '') return state
  const lines = [...state.lines, line]
  return { ...state, lines: lines.length > max ? lines.slice(lines.length - max) : lines }
}

/**
 * Fold raw launcher stdout into the state (parse + redact + retain).
 * @param state - current state.
 * @param rawLine - one raw stdout line.
 * @returns the next state.
 */
export function consumeLine(state, rawLine) {
  const line = redactSecrets(rawLine)
  return appendLogLine(applyLauncherLine(state, parseLauncherLine(line)), line)
}

/**
 * State after the launcher child exited.
 * @param state - current state.
 * @param code - child exit code (null when killed by a signal).
 * @returns the next state; a clean exit after `ready` keeps `ready` (the shell
 *   is shutting down with the product window), anything else reports the code.
 */
export function applyLauncherExit(state, code) {
  if (state.phase === 'ready' && code === 0) {
    return { ...state, phase: 'exited', exitCode: 0, message: '产品窗口已关闭，正在退出…' }
  }
  if (state.phase === 'blocked') return { ...state, exitCode: code }
  return {
    ...state,
    phase: 'failed',
    exitCode: code,
    message: `${state.message}（launcher 退出码 ${code ?? '信号终止'}）`,
  }
}
