import { test } from 'node:test'
import assert from 'node:assert/strict'
import { EventEmitter } from 'node:events'
import { join } from 'node:path'
import { createEngineSession, engineArgv, parseAnnounce } from '../src/engine-client.js'

const CONFIG = {
  engineExe: join('/repo', 'cicada-editor', 'Release', 'cicada-engine.exe'),
  engineDir: join('/repo', 'cicada-editor', 'Release'),
  libDir: join('/repo', 'assets', 'cicada-libs'),
  userLibDir: join('/home', 'cicada', 'cicada-user-lib'),
}

const stream = () => Object.assign(new EventEmitter(), { setEncoding: () => {} })

/** Engine process stub that announces itself on the next tick. */
function fakeEngine(announce = 'cicada-engine: 127.0.0.1:41234 tok123\n') {
  const child = new EventEmitter()
  child.stdout = stream()
  child.stderr = stream()
  child.kill = () => { child.killed = true }
  setImmediate(() => child.stdout.emit('data', announce))
  return child
}

test('engine argv addresses both libraries and no document', () => {
  assert.deepEqual(engineArgv(CONFIG), ['--port', '0', '--lib-dir', CONFIG.libDir, '--user-lib-dir', CONFIG.userLibDir])
})

test('announce parsing accepts only the engine line', () => {
  assert.deepEqual(parseAnnounce('cicada-engine: 127.0.0.1:9 abc'), { endpoint: 'http://127.0.0.1:9', token: 'abc' })
  assert.equal(parseAnnounce('cicada: 引擎端口 9'), undefined)
  assert.equal(parseAnnounce('cicada-engine: 127.0.0.1:9'), undefined)
})

test('the session starts lazily, sends the token, and stops when idle', async () => {
  const spawned = []
  let idleCallback = null
  const session = createEngineSession(CONFIG, {
    spawnFn: (exe, argv) => { spawned.push([exe, argv]); return fakeEngine() },
    setTimeoutFn: (fn) => { idleCallback = fn; return 1 },
    clearTimeoutFn: () => { idleCallback = null },
  })
  assert.equal(session.started(), false, 'nothing may run before the first request')
  assert.deepEqual(spawned, [])

  const seen = []
  const result = await session.request('/lib/list', undefined, {
    fetchFn: async (url, options) => {
      seen.push([url, options.method, options.headers['X-Cicada-Token']])
      return { ok: true, status: 200, json: async () => ({ symbols: [{ libId: 'R:R', pins: 2 }] }) }
    },
  })
  assert.equal(result.ok, true)
  assert.equal(result.data.symbols[0].libId, 'R:R')
  assert.equal(session.started(), true)
  assert.deepEqual(seen, [['http://127.0.0.1:41234/lib/list', 'GET', 'tok123']])
  assert.equal(spawned.length, 1)

  // Second request reuses the same process (no respawn).
  await session.request('/lib/get', { libId: 'R:R' }, { fetchFn: async (url, options) => ({ ok: true, status: 200, json: async () => JSON.parse(options.body) }) })
  assert.equal(spawned.length, 1)

  // Idle timer fires → the process is killed.
  assert.equal(typeof idleCallback, 'function')
  idleCallback()
  assert.equal(session.started(), false)
})

test('a failed request reports the engine message, and a missing engine reports a timeout', async () => {
  const session = createEngineSession(CONFIG, { spawnFn: () => fakeEngine() })
  const missing = await session.request('/lib/get', { libId: 'IC:NOPE' }, {
    fetchFn: async () => ({ ok: false, status: 404, json: async () => ({ error: { code: 'not_found', message: 'symbol not found' } }) }),
  })
  assert.deepEqual([missing.ok, missing.problem], [false, 'symbol not found'])

  const dead = createEngineSession(CONFIG, {
    spawnFn: () => { const child = fakeEngine(''); child.stdout.emit = () => {}; return child },
    setTimeoutFn: (fn) => { fn(); return 1 },
    clearTimeoutFn: () => {},
  })
  const result = await dead.request('/lib/list')
  assert.equal(result.ok, false)
  assert.match(result.problem, /引擎未就绪/)
})
