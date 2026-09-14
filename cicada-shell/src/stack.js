/**
 * Shell ↔ launcher process plumbing (docs/04 §2.1): spawn the existing
 * `cicada-app` launcher, stream its stdout/stderr line by line, and guarantee
 * the whole child tree (launcher + host + engine) dies with the shell —
 * Windows has no graceful child signal, so teardown is `taskkill /T`.
 */

import { spawn, spawnSync } from 'node:child_process'
import { readFileSync } from 'node:fs'
import { launcherEnv, preloadFor } from './config.js'
import { redactSecrets } from './status.js'

/**
 * Launcher argv: TypeScript entries get the tsx preload, built entries none.
 * @param config - resolved shell configuration.
 * @param preload - preload specifier (defaults to {@link preloadFor} of the entry).
 * @param extraArgs - appended launcher flags (the smoke run adds `--no-window`).
 * @returns argv for the Node executable (without the executable itself).
 */
export function buildLauncherArgv(config, preload = preloadFor(config.launcherEntry), extraArgs = []) {
  return [...(preload === undefined ? [] : ['--import', preload]), config.launcherEntry, ...extraArgs]
}

/**
 * Split streamed chunks into complete lines (the launcher flushes whole lines,
 * but chunk boundaries are arbitrary).
 * @param onLine - called once per complete line.
 * @returns `push(chunk)` and `flush()` (for the trailing partial line).
 */
export function createLineReader(onLine) {
  let buffer = ''
  const drain = (text) => {
    buffer += text
    let index = buffer.indexOf('\n')
    while (index >= 0) {
      onLine(buffer.slice(0, index))
      buffer = buffer.slice(index + 1)
      index = buffer.indexOf('\n')
    }
  }
  return {
    push: drain,
    flush: () => {
      if (buffer !== '') {
        onLine(buffer)
        buffer = ''
      }
    },
  }
}

/**
 * The command that kills a process tree.
 * @param pid - root process id.
 * @param platform - `process.platform` value (injected by tests).
 * @returns command + args for that platform.
 */
export function killTreeCommand(pid, platform = process.platform) {
  return platform === 'win32'
    ? { command: 'taskkill', args: ['/PID', String(pid), '/T', '/F'] }
    : { command: 'kill', args: ['-TERM', String(pid)] }
}

/**
 * Kill a child and its descendants, synchronously: the shell may be inside
 * `before-quit`, so teardown must complete before the process exits.
 * @param child - the launcher child (no-op when already gone).
 * @param options - platform and `spawnSync` override for tests.
 */
export function killTree(child, options = {}) {
  if (child === null || child === undefined || child.pid === undefined || child.exitCode !== null) return
  const { command, args } = killTreeCommand(child.pid, options.platform)
  try {
    ;(options.run ?? spawnSync)(command, args, { stdio: 'ignore', windowsHide: true })
  } catch {
    // taskkill missing or the pid vanished between check and call: fall back to
    // the direct child kill, which still ends the launcher itself.
    child.kill()
  }
}

/**
 * Spawn the launcher and wire its streams.
 * @param config - resolved shell configuration.
 * @param handlers - `onLine(rawLine)` per stdout/stderr line, `onExit(code)`,
 *   plus optional base `env` and extra launcher `extraArgs`.
 * @returns `{ pid, child, stop }`; `stop()` kills the tree.
 */
export function spawnLauncher(config, handlers) {
  const argv = buildLauncherArgv(config, undefined, handlers.extraArgs ?? [])
  const child = spawn(config.nodeBin, argv, {
    cwd: config.launcherCwd,
    env: launcherEnv(config, handlers.env ?? process.env),
    stdio: ['ignore', 'pipe', 'pipe'],
    windowsHide: true,
  })
  const reader = createLineReader(handlers.onLine)
  child.stdout?.setEncoding('utf8')
  child.stdout?.on('data', (chunk) => reader.push(chunk))
  child.stderr?.setEncoding('utf8')
  child.stderr?.on('data', (chunk) => reader.push(chunk))
  child.on('error', (error) => handlers.onLine(`cicada: 启动器启动失败: ${error.message}`))
  child.on('exit', (code) => {
    reader.flush()
    handlers.onExit(code)
  })
  return { pid: child.pid, child, stop: () => killTree(child) }
}

/**
 * Read the tail of `logs/launch.txt` with URL credentials redacted.
 * @param path - log file path.
 * @param maxLines - retained line count.
 * @param readFile - `readFileSync` override for tests.
 * @returns the last lines, oldest first; empty when the file is missing.
 */
export function readLogTail(path, maxLines = 8, readFile = readFileSync) {
  try {
    const lines = readFile(path, 'utf8').split(/\r?\n/).filter((line) => line.trim() !== '')
    return lines.slice(-maxLines).map(redactSecrets)
  } catch {
    return []
  }
}
