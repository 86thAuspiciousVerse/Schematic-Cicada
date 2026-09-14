/**
 * Headless shell smoke (docs/04 §2, docs/06 §3): exercise everything the shell
 * does to the stack — config derivation, preflight, spawning the launcher with
 * its real environment, the stdout line protocol, redaction, and process-tree
 * teardown — with `--no-window`, so no product window is opened.
 *
 * The report is written as JSON to a file because Electron's stdout is not
 * attached to the parent console on Windows (AGENTS §4-1: log to a file, read
 * the file).
 */

import { mkdirSync, writeFileSync } from 'node:fs'
import { dirname, join } from 'node:path'
import { preflight } from './config.js'
import { consumeLine, initialState, redactSecrets } from './status.js'
import { killTree, spawnLauncher } from './stack.js'

/** Phases that end a smoke run (`ready` needs no human, `blocked` needs none either). */
const TERMINAL = new Set(['ready', 'failed', 'blocked'])

/** Wait after the stack reports ready, so teardown sees the settled tree. */
const SETTLE_MS = 1500

/**
 * Run the shell's stack path headlessly and report the outcome.
 * @param config - resolved shell configuration.
 * @param reportPath - JSON report destination.
 * @param options - `timeoutMs` (default 90000), `workspace` (passed to the
 *   launcher as `--workspace`, i.e. the project the launcher window would have
 *   started for — the path that exercises the host's project cwd), and
 *   `onReport` (called before exit).
 * @returns the report object (also written to `reportPath`).
 */
export function runSmoke(config, reportPath, options = {}) {
  const timeoutMs = options.timeoutMs ?? 90_000
  // Default to a scratch project under `<home>/projects`: the launcher window
  // ALWAYS starts the stack for a project, so the smoke must exercise that path
  // (the host runs with the project directory as its cwd). `CICADA_SMOKE_WORKSPACE=''`
  // opts back into the no-workspace run.
  const workspace = options.workspace ?? process.env.CICADA_SMOKE_WORKSPACE ?? join(config.home, 'projects', '_smoke')
  if (workspace !== '') {
    try {
      mkdirSync(workspace, { recursive: true })
    } catch {
      // A workspace that cannot be created is not fatal here: the launcher logs
      // it and keeps its own cwd, and the run still proves the stack boots.
    }
  }
  const started = Date.now()
  return new Promise((resolve) => {
    const problems = preflight(config)
    const lines = []
    let state = initialState()
    let launcher = null
    let done = false

    const finish = (exitCode, note) => {
      if (done) return
      done = true
      if (launcher !== null) killTree(launcher)
      const report = {
        ok: problems.length === 0 && state.phase === 'ready',
        note,
        electron: process.versions.electron,
        shellNode: process.versions.node,
        chrome: process.versions.chrome,
        platform: process.platform,
        nodeBin: config.nodeBin,
        nodeSource: config.nodeSource,
        entry: config.launcherEntry,
        engineExe: config.engineExe,
        home: config.home,
        workspace,
        problems,
        phase: state.phase,
        message: state.message,
        enginePort: state.enginePort,
        hasWindow: state.windowUrl !== null,
        launcherExitCode: exitCode,
        elapsedMs: Date.now() - started,
        lines,
      }
      try {
        mkdirSync(dirname(reportPath), { recursive: true })
        writeFileSync(reportPath, `${JSON.stringify(report, null, 2)}\n`)
      } catch {
        // The report is diagnostics: a write failure must not mask the result.
      }
      options.onReport?.(report)
      resolve(report)
    }

    if (problems.length > 0) {
      finish(null, 'preflight failed')
      return
    }

    launcher = spawnLauncher(config, {
      extraArgs: workspace === '' ? ['--no-window'] : ['--no-window', '--workspace', workspace],
      onLine: (line) => {
        lines.push(redactSecrets(line))
        state = consumeLine(state, line)
        if (TERMINAL.has(state.phase)) setTimeout(() => finish(0, `terminal phase ${state.phase}`), SETTLE_MS)
      },
      onExit: (code) => finish(code, 'launcher exited before a terminal phase'),
    })

    setTimeout(() => finish(null, `timeout after ${timeoutMs}ms`), timeoutMs)
  })
}
