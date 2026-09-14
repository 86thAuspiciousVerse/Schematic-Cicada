/**
 * Shell side of the datasheet manager (docs/04 §5.5): spawn the service on
 * demand, keep its endpoint, and forward requests. Kept separate from `main.js`
 * so the lifecycle is testable with an injected spawn/announce pair.
 */

import { spawn } from 'node:child_process'
import { join } from 'node:path'
import { existsSync } from 'node:fs'
import { pathToFileURL } from 'node:url'
import { createLineReader } from './stack.js'

/** Where the manager entry lives (next to this module). */
export const MANAGER_ENTRY = join(import.meta.dirname, 'manager', 'server.js')

/** Options handed to the manager process. */
export function managerSpawnSpec(config, token) {
  return {
    command: config.nodeBin,
    args: [
      '--import', pathToFileURL(join(config.root, 'cicada-harness', 'node_modules', 'tsx', 'dist', 'esm', 'index.mjs')).href,
      MANAGER_ENTRY,
      '--home', config.home,
      '--knowledge', pathToFileURL(join(config.root, 'cicada-harness', 'packages', 'cicada', 'cicada-knowledge', 'src', 'database.ts')).href,
      '--token', token,
      '--port', '0',
      // Symbol page: the manager needs both libraries and the engine binary to
      // read pin geometry (docs/04 §5.4). Absent paths make the page report
      // "not configured" rather than guess.
      '--lib-dir', config.libDir,
      '--user-lib-dir', config.userLibDir,
      '--engine', config.engineExe,
      // The datasheet→engine electrical mapping lives in cicada-symbols; the
      // service imports that one implementation instead of restating it.
      '--electrical', pathToFileURL(join(config.root, 'cicada-harness', 'packages', 'cicada', 'cicada-symbols', 'src', 'datasheet.ts')).href,
    ],
    options: {
      cwd: config.root,
      env: { ...process.env, TSX_TSCONFIG_PATH: join(config.root, 'cicada-harness', 'tsconfig.json') },
      stdio: ['pipe', 'pipe', 'pipe'],
      windowsHide: true,
    },
  }
}

/**
 * Everything the manager needs must exist before spawning it, so a missing
 * harness gives a readable message instead of a process that dies silently.
 * @param config - resolved shell configuration.
 * @param exists - existence probe override for tests.
 * @returns problem messages; empty means "startable".
 */
export function managerPreflight(config, exists = existsSync) {
  const problems = []
  if (!exists(MANAGER_ENTRY)) problems.push(`管理服务入口缺失：${MANAGER_ENTRY}`)
  const knowledge = join(config.root, 'cicada-harness', 'packages', 'cicada', 'cicada-knowledge', 'src', 'database.ts')
  if (!exists(knowledge)) problems.push(`数据手册读取器缺失：${knowledge}`)
  const tsx = join(config.root, 'cicada-harness', 'node_modules', 'tsx', 'dist', 'esm', 'index.mjs')
  if (!exists(tsx)) problems.push(`tsx 加载器缺失：${tsx}（先在 cicada-harness 里 pnpm install）`)
  if (config.nodeBin === '') problems.push('未找到 Node 运行时（CICADA_NODE_BIN 可覆盖）')
  return problems
}

/** Random one-launch token (32 hex chars). */
export function managerToken(random = Math.random) {
  let token = ''
  while (token.length < 32) token += Math.floor(random() * 16).toString(16)
  return token.slice(0, 32)
}

/**
 * Spawn the manager and wait for its announce line.
 * @param config - resolved shell configuration.
 * @param handlers - `{ onExit(code), timeoutMs, spawnFn, preflight }` (the last
 *   two exist so a test can drive the lifecycle without a real workspace).
 * @returns `{ ok, endpoint, token, stop, problem }`; `stop()` ends the process.
 */
export async function startManager(config, handlers = {}) {
  const problems = (handlers.preflight ?? managerPreflight)(config)
  if (problems.length > 0) return { ok: false, endpoint: null, token: '', stop: () => {}, problem: problems.join('；') }
  const token = managerToken()
  const spec = managerSpawnSpec(config, token)
  const spawnFn = handlers.spawnFn ?? spawn
  const child = spawnFn(spec.command, spec.args, spec.options)
  const endpoint = await new Promise((resolve) => {
    const timer = setTimeout(() => resolve(null), handlers.timeoutMs ?? 20_000)
    const reader = createLineReader((line) => {
      const match = /^cicada-manager:\s+127\.0\.0\.1:(\d+)\s*$/.exec(line)
      if (match !== null) {
        clearTimeout(timer)
        resolve(`http://127.0.0.1:${match[1]}`)
      }
    })
    child.stdout?.setEncoding('utf8')
    child.stdout?.on('data', (chunk) => reader.push(chunk))
    child.stderr?.setEncoding('utf8')
    child.stderr?.on('data', (chunk) => process.stderr.write(`[manager] ${chunk}`))
    child.on('exit', (code) => {
      clearTimeout(timer)
      resolve(null)
      handlers.onExit?.(code)
    })
  })
  const stop = () => {
    try {
      child.stdin?.end()
      child.kill()
    } catch {
      // Already gone: nothing to stop.
    }
  }
  if (endpoint === null) {
    stop()
    return { ok: false, endpoint: null, token, stop, problem: '管理服务启动超时（无 announce 行）' }
  }
  return { ok: true, endpoint, token, stop, problem: '' }
}

/**
 * One manager request.
 * @param session - `{ endpoint, token }` from {@link startManager}.
 * @param path - path under the manager root (e.g. `/datasheets`).
 * @param options - `{ method, body, timeoutMs, fetchFn }`.
 * @returns `{ ok, status, data, problem }`.
 */
export async function managerRequest(session, path, options = {}) {
  if (session === null || session.endpoint === null) {
    return { ok: false, status: 0, data: undefined, problem: '管理服务未启动' }
  }
  const fetchFn = options.fetchFn ?? fetch
  const controller = new AbortController()
  const timer = setTimeout(() => controller.abort(), options.timeoutMs ?? 15_000)
  try {
    const response = await fetchFn(`${session.endpoint}${path}`, {
      method: options.method ?? 'GET',
      headers: { 'x-cicada-manager-token': session.token, 'Content-Type': 'application/json' },
      ...(options.body === undefined ? {} : { body: JSON.stringify(options.body) }),
      signal: controller.signal,
    })
    const data = await response.json().catch(() => undefined)
    if (!response.ok) {
      return { ok: false, status: response.status, data, problem: data?.message ?? data?.error ?? `HTTP ${response.status}` }
    }
    return { ok: true, status: response.status, data, problem: '' }
  } catch (error) {
    return { ok: false, status: 0, data: undefined, problem: String(error?.message ?? error) }
  } finally {
    clearTimeout(timer)
  }
}
