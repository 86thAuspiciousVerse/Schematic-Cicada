import { test } from 'node:test'
import assert from 'node:assert/strict'
import {
  appendLogLine, applyLauncherExit, applyLauncherLine, consumeLine, initialState,
  parseLauncherLine, redactSecrets,
} from '../src/status.js'

const TOKEN_URL = 'http://127.0.0.1:3123/?token=K4kHtdbpOwsb42IhmjMs8doRxELHQo-JHB8OZwBosJE'

test('redactSecrets removes launch-token values and nothing else', () => {
  assert.equal(redactSecrets(`dsh web: ${TOKEN_URL}`), 'dsh web: http://127.0.0.1:3123/?token=***')
  assert.equal(redactSecrets('http://h/#token=abc&x=1'), 'http://h/#token=***&x=1')
  assert.equal(redactSecrets('plain engine announce: 127.0.0.1:2334 deadbeef'), 'plain engine announce: 127.0.0.1:2334 deadbeef')
})

test('redactSecrets strips the announce tokens the host prints', () => {
  assert.equal(redactSecrets('cicada-editor: 3123 680TgPsVExh9pfFKVnsXm8MlimuWi3O3J9pt1A_CA9k'), 'cicada-editor: 3123 ***')
  assert.equal(redactSecrets('cicada-engine: 127.0.0.1:2334 906cf76cd3184412d3513bc42ee98e45'), 'cicada-engine: 127.0.0.1:2334 ***')
  assert.equal(redactSecrets('cicada: 引擎端口 2334'), 'cicada: 引擎端口 2334')
})

test('parseLauncherLine: engine port', () => {
  assert.deepEqual(parseLauncherLine('cicada: 引擎端口 2334'), { kind: 'engine-port', port: 2334 })
})

test('parseLauncherLine: window url from the launcher and from the host line', () => {
  assert.deepEqual(parseLauncherLine(`cicada: 窗口地址 ${TOKEN_URL}`), { kind: 'window-url', url: TOKEN_URL })
  assert.deepEqual(parseLauncherLine(`dsh web: ${TOKEN_URL}`), { kind: 'window-url', url: TOKEN_URL })
})

test('parseLauncherLine: instance-already-running and Edge-missing', () => {
  assert.equal(parseLauncherLine('cicada: 已有实例在运行 (pid 42)。请聚焦既有窗口').kind, 'already-running')
  assert.equal(parseLauncherLine('cicada: 检测到启动中的实例 (pid 42)，等待其就绪…').kind, 'already-running')
  assert.equal(parseLauncherLine('cicada: 未找到 Edge（CICADA_EDGE 可指定路径）。请手动打开上面的地址').kind, 'edge-missing')
})

test('parseLauncherLine: launcher failures keep their message', () => {
  const parsed = parseLauncherLine('cicada: 引擎未就绪（30s 无宣告行）')
  assert.equal(parsed.kind, 'failure')
  assert.equal(parsed.message, '引擎未就绪（30s 无宣告行）')
  assert.equal(parseLauncherLine('cicada: host 启动超时（120s 未出 dsh web: 行），已清理').kind, 'failure')
  assert.equal(parseLauncherLine('cicada: 实例锁被占用 (pid 7)，请稍后重试').kind, 'failure')
})

test('parseLauncherLine: anything else is a plain log line (CRLF tolerated)', () => {
  assert.deepEqual(parseLauncherLine('cicada: 引擎端口 99\r'), { kind: 'engine-port', port: 99 })
  assert.deepEqual(parseLauncherLine('some host log'), { kind: 'log', text: 'some host log' })
})

test('applyLauncherLine drives the phases', () => {
  let state = initialState()
  assert.equal(state.phase, 'starting')

  state = applyLauncherLine(state, parseLauncherLine('cicada: 引擎端口 42590'))
  assert.equal(state.enginePort, 42590)
  assert.equal(state.phase, 'starting')

  state = applyLauncherLine(state, parseLauncherLine(`cicada: 窗口地址 ${TOKEN_URL}`))
  assert.equal(state.phase, 'ready')
  assert.equal(state.windowUrl, TOKEN_URL)

  const blocked = applyLauncherLine(initialState(), parseLauncherLine('cicada: 已有实例在运行 (pid 42)'))
  assert.equal(blocked.phase, 'blocked')

  const failed = applyLauncherLine(initialState(), parseLauncherLine('cicada: host 启动超时（120s 未出 dsh web: 行），已清理'))
  assert.equal(failed.phase, 'failed')
  assert.match(failed.message, /host 启动超时/)

  const warned = applyLauncherLine(initialState(), parseLauncherLine('cicada: 未找到 Edge（CICADA_EDGE 可指定路径）'))
  assert.equal(warned.phase, 'starting')
  assert.match(warned.message, /未找到 Edge/)
})

test('consumeLine retains a redacted tail and never stores the token', () => {
  let state = initialState()
  state = consumeLine(state, `dsh web: ${TOKEN_URL}`)
  state = consumeLine(state, 'cicada: 引擎端口 2334')
  assert.equal(state.phase, 'ready')
  assert.equal(state.lines.length, 2)
  assert.ok(state.lines[0].includes('token=***'))
  assert.ok(!state.lines.join('\n').includes('K4kHtdbpOwsb42IhmjMs8doRxELHQo-JHB8OZwBosJE'))
  assert.ok(state.lines[1].includes('引擎端口'))
})

test('appendLogLine keeps only the newest lines and drops blanks', () => {
  let state = initialState()
  for (let i = 0; i < 5; i += 1) state = appendLogLine(state, `line ${i}`, 3)
  assert.deepEqual(state.lines, ['line 2', 'line 3', 'line 4'])
  assert.equal(appendLogLine(state, '   ', 3), state)
})

test('applyLauncherExit: clean exit after ready means the product window closed', () => {
  const ready = applyLauncherLine(initialState(), parseLauncherLine(`cicada: 窗口地址 ${TOKEN_URL}`))
  const exited = applyLauncherExit(ready, 0)
  assert.equal(exited.phase, 'exited')
  assert.equal(exited.exitCode, 0)
})

test('applyLauncherExit: a failure code stays failed, a blocked instance stays blocked', () => {
  const failed = applyLauncherExit(initialState(), 1)
  assert.equal(failed.phase, 'failed')
  assert.match(failed.message, /退出码 1/)

  const blocked = applyLauncherExit(applyLauncherLine(initialState(), parseLauncherLine('cicada: 已有实例在运行 (pid 42)')), 0)
  assert.equal(blocked.phase, 'blocked')
  assert.equal(blocked.exitCode, 0)

  const killed = applyLauncherExit(readyAfterCrash(), null)
  assert.equal(killed.phase, 'failed')
  assert.match(killed.message, /信号终止/)
})

/** Ready-but-not-clean-exit state used by the exit tests. */
function readyAfterCrash() {
  return applyLauncherLine(initialState(), parseLauncherLine('cicada: 引擎端口 1'))
}
