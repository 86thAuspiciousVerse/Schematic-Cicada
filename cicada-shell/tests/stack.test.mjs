import { test } from 'node:test'
import assert from 'node:assert/strict'
import { join } from 'node:path'
import { resolveShellConfig } from '../src/config.js'
import { buildLauncherArgv, createLineReader, killTree, killTreeCommand, readLogTail } from '../src/stack.js'

const SHELL = join('/repo', 'cicada-shell')
/** Deterministic PATH probe so tests never depend on the host machine. */
const NODE_OPTS = { exists: () => true, pathValue: '/usr/bin', separator: ':', exeName: 'node' }

test('buildLauncherArgv adds the tsx preload for source entries only', () => {
  const ts = resolveShellConfig({}, SHELL, NODE_OPTS)
  assert.deepEqual(buildLauncherArgv(ts), ['--import', 'tsx/esm', ts.launcherEntry])

  const built = resolveShellConfig({ CICADA_LAUNCHER_ENTRY: '/e/cicada-app.js' }, SHELL, NODE_OPTS)
  assert.deepEqual(buildLauncherArgv(built), ['/e/cicada-app.js'])
})

test('buildLauncherArgv appends launcher flags (smoke runs --no-window)', () => {
  const config = resolveShellConfig({ CICADA_LAUNCHER_ENTRY: '/e/cicada-app.js' }, SHELL, NODE_OPTS)
  assert.deepEqual(buildLauncherArgv(config, undefined, ['--no-window']), ['/e/cicada-app.js', '--no-window'])
})

test('createLineReader reassembles lines across chunk boundaries', () => {
  const lines = []
  const reader = createLineReader((line) => lines.push(line))
  reader.push('cicada: 引擎')
  reader.push('端口 2334\ncicada: 窗口地址 http://x/\npar')
  assert.deepEqual(lines, ['cicada: 引擎端口 2334', 'cicada: 窗口地址 http://x/'])
  reader.push('tial')
  reader.flush()
  assert.deepEqual(lines.at(-1), 'partial')
})

test('killTreeCommand is a process-tree kill on Windows and a plain signal elsewhere', () => {
  assert.deepEqual(killTreeCommand(123, 'win32'), { command: 'taskkill', args: ['/PID', '123', '/T', '/F'] })
  assert.deepEqual(killTreeCommand(123, 'darwin'), { command: 'kill', args: ['-TERM', '123'] })
})

test('killTree kills the tree once and is a no-op for an exited child', () => {
  const calls = []
  const child = { pid: 4242, exitCode: null, kill: () => calls.push('child.kill') }
  killTree(child, { platform: 'win32', run: (command, args) => calls.push([command, ...args].join(' ')) })
  assert.deepEqual(calls, ['taskkill /PID 4242 /T /F'])

  killTree({ pid: 4242, exitCode: 0, kill: () => calls.push('child.kill') }, { platform: 'win32', run: () => calls.push('ran') })
  killTree(null, { platform: 'win32', run: () => calls.push('ran') })
  killTree({ pid: undefined, exitCode: null, kill: () => {} }, { platform: 'win32', run: () => calls.push('ran') })
  assert.deepEqual(calls, ['taskkill /PID 4242 /T /F'])
})

test('killTree falls back to the direct child kill when taskkill throws', () => {
  const calls = []
  killTree(
    { pid: 9, exitCode: null, kill: () => calls.push('child.kill') },
    { platform: 'win32', run: () => { throw new Error('taskkill missing') } },
  )
  assert.deepEqual(calls, ['child.kill'])
})

test('readLogTail returns the newest redacted lines', () => {
  const text = [
    '[t] launcher start',
    '[t] engine announce: cicada-engine: 127.0.0.1:2334 deadbeef',
    '[t] dsh web: http://127.0.0.1:3123/?token=secret-token-value',
    '',
  ].join('\r\n')
  const tail = readLogTail('/home/logs/launch.txt', 2, () => text)
  assert.deepEqual(tail, [
    '[t] engine announce: cicada-engine: 127.0.0.1:2334 deadbeef',
    '[t] dsh web: http://127.0.0.1:3123/?token=***',
  ])
})

test('readLogTail tolerates a missing log file', () => {
  assert.deepEqual(readLogTail('/nope/launch.txt', 8, () => { throw new Error('ENOENT') }), [])
})
