/**
 * Engine access for the symbol page (docs/04 §5.4/§5.5): the pin table and the
 * canonical keys come from `cicada-engine` (`/lib/list`, `/lib/get`) — the shell
 * never parses a `.kicad_sym` itself.
 *
 * The process is started on demand from the manager and stopped
 * {@link IDLE_STOP_MS} after the last request, so browsing symbols does not hold
 * a second engine for the whole session.
 */

import { spawn } from 'node:child_process'

/** How long the browse engine stays alive after the last request. */
export const IDLE_STOP_MS = 60_000
/** Engine announce timeout (the same budget the launcher uses). */
export const ANNOUNCE_TIMEOUT_MS = 30_000

/**
 * Engine argv for a read-only browse instance: no document, both libraries.
 * @param config - resolved shell configuration.
 * @returns argv (without the executable).
 */
export function engineArgv(config) {
  return [
    '--port', '0',
    '--lib-dir', config.libDir,
    '--user-lib-dir', config.userLibDir,
  ]
}

/** Parse the announce line the engine prints on stdout. */
export function parseAnnounce(line) {
  const match = /^cicada-engine:\s+127\.0\.0\.1:(\d+)\s+(\S+)\s*$/.exec(line)
  return match === null ? undefined : { endpoint: `http://127.0.0.1:${match[1]}`, token: match[2] }
}

/** Split chunks into complete lines. */
function lineReader(onLine) {
  let buffer = ''
  return (chunk) => {
    buffer += chunk
    let index = buffer.indexOf('\n')
    while (index >= 0) {
      onLine(buffer.slice(0, index))
      buffer = buffer.slice(index + 1)
      index = buffer.indexOf('\n')
    }
  }
}

/**
 * Create a lazy engine session: nothing runs until the first {@link request}.
 * @param config - resolved shell configuration.
 * @param handlers - `{ spawnFn, setTimeoutFn, clearTimeoutFn, idleMs }` for tests.
 * @returns `{ request, stop, started }` — `started()` reports whether a process
 *   is currently alive.
 */
export function createEngineSession(config, handlers = {}) {
  const spawnFn = handlers.spawnFn ?? spawn
  const setTimer = handlers.setTimeoutFn ?? setTimeout
  const clearTimer = handlers.clearTimeoutFn ?? clearTimeout
  const idleMs = handlers.idleMs ?? IDLE_STOP_MS
  let child = null
  let info = null
  let starting = null
  let idle = null

  const stop = () => {
    if (idle !== null) { clearTimer(idle); idle = null }
    const running = child
    child = null
    info = null
    starting = null
    if (running !== null) {
      try {
        running.kill()
      } catch {
        // Already gone.
      }
    }
  }

  const bumpIdle = () => {
    if (idle !== null) clearTimer(idle)
    idle = setTimer(() => stop(), idleMs)
  }

  const ensure = () => {
    if (info !== null) return Promise.resolve(info)
    if (starting !== null) return starting
    starting = new Promise((resolve) => {
      const proc = spawnFn(config.engineExe, engineArgv(config), {
        cwd: config.engineDir,
        stdio: ['ignore', 'pipe', 'pipe'],
        windowsHide: true,
      })
      // `finish` must exist before the timeout is armed: the timer callback
      // closes over it, and a `const` below the timer would be in its temporal
      // dead zone when a slow engine times out.
      let timer = null
      const finish = (value) => {
        if (timer !== null) clearTimer(timer)
        starting = null
        if (value === null) {
          try {
            proc.kill()
          } catch {
            // Already gone.
          }
          resolve(null)
          return
        }
        child = proc
        info = value
        bumpIdle()
        resolve(value)
      }
      timer = setTimer(() => finish(null), ANNOUNCE_TIMEOUT_MS)
      const read = lineReader((line) => {
        const parsed = parseAnnounce(line)
        if (parsed !== undefined) finish(parsed)
      })
      proc.stdout?.setEncoding('utf8')
      proc.stdout?.on('data', (chunk) => read(chunk))
      proc.stderr?.setEncoding('utf8')
      proc.stderr?.on('data', () => {})
      proc.on('exit', () => { if (info === null) finish(null); else { child = null; info = null } })
    })
    return starting
  }

  const request = async (path, body, options = {}) => {
    const session = await ensure()
    if (session === null) return { ok: false, data: undefined, problem: '引擎未就绪（announce 超时）' }
    bumpIdle()
    const fetchFn = options.fetchFn ?? fetch
    try {
      const response = await fetchFn(`${session.endpoint}${path}`, {
        method: options.method ?? (body === undefined ? 'GET' : 'POST'),
        headers: { 'X-Cicada-Token': session.token, 'Content-Type': 'application/json' },
        ...(body === undefined ? {} : { body: JSON.stringify(body) }),
      })
      const data = await response.json().catch(() => undefined)
      if (!response.ok) return { ok: false, data, problem: data?.error?.message ?? `HTTP ${response.status}` }
      return { ok: true, data, problem: '' }
    } catch (error) {
      return { ok: false, data: undefined, problem: String(error?.message ?? error) }
    }
  }

  return { request, stop, started: () => info !== null }
}
