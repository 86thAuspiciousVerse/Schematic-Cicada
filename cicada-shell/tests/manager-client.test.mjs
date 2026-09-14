import { test } from 'node:test'
import assert from 'node:assert/strict'
import { EventEmitter } from 'node:events'
import { join } from 'node:path'
import { pathToFileURL } from 'node:url'
import { managerPreflight, managerRequest, managerSpawnSpec, managerToken, startManager } from '../src/manager-client.js'

const ROOT = join('/repo')
const CONFIG = {
  root: ROOT,
  home: join(ROOT, 'cicada-dev-home'),
  nodeBin: join('/nodejs', 'node.exe'),
}

/** Fake child process: emits an announce line on the next tick. */
function stream() {
  return Object.assign(new EventEmitter(), { setEncoding: () => {} })
}

function fakeChild(line) {
  const child = new EventEmitter()
  child.stdout = stream()
  child.stderr = stream()
  child.stdin = { end: () => {} }
  child.kill = () => { child.killed = true }
  setImmediate(() => child.stdout.emit('data', line))
  return child
}

test('spawn spec carries home, the knowledge module URL and a token, with no machine literal', () => {
  const spec = managerSpawnSpec(CONFIG, 'tok123')
  assert.equal(spec.command, CONFIG.nodeBin)
  assert.ok(spec.args.includes('--home') && spec.args.includes(CONFIG.home))
  assert.ok(spec.args.includes('--token') && spec.args.includes('tok123'))
  const knowledge = spec.args.find((value) => value.endsWith('database.ts'))
  // Compare through the same URL builder the code uses: a Windows path renders
  // as file:///C:/… and a POSIX one as file:///…, and both are correct.
  const expectedKnowledge = pathToFileURL(join(ROOT, 'cicada-harness', 'packages', 'cicada', 'cicada-knowledge', 'src', 'database.ts')).href
  assert.equal(knowledge, expectedKnowledge)
  assert.equal(spec.options.cwd, ROOT)
  assert.equal(spec.options.env.TSX_TSCONFIG_PATH, join(ROOT, 'cicada-harness', 'tsconfig.json'))
  assert.ok(!JSON.stringify(spec).includes('C:\\dsh'), 'no machine path may be baked in')
})

test('preflight names every missing piece instead of spawning a doomed process', () => {
  const problems = managerPreflight(CONFIG, () => false)
  // entry + knowledge reader + tsx loader (nodeBin is set in CONFIG)
  assert.equal(problems.length, 3)
  assert.ok(problems.some((line) => line.includes('管理服务入口缺失')))
  assert.ok(problems.some((line) => line.includes('数据手册读取器缺失')))
  assert.ok(problems.some((line) => line.includes('tsx')))
  assert.deepEqual(managerPreflight(CONFIG, () => true), [])
  assert.match(managerPreflight({ ...CONFIG, nodeBin: '' }, () => true)[0], /Node 运行时/)
})

test('tokens are 32 hex characters and differ per launch', () => {
  const token = managerToken()
  assert.match(token, /^[0-9a-f]{32}$/)
  assert.notEqual(managerToken(), managerToken())
})

test('startManager parses the announce line and exposes the endpoint', async () => {
  const session = await startManager(CONFIG, {
    preflight: () => [],
    spawnFn: () => fakeChild('cicada-manager: 127.0.0.1:51234\n'),
  })
  assert.equal(session.ok, true)
  assert.equal(session.endpoint, 'http://127.0.0.1:51234')
  assert.match(session.token, /^[0-9a-f]{32}$/)
})

test('a manager that never announces fails with a readable problem and is stopped', async () => {
  let killed = false
  const child = new EventEmitter()
  child.stdout = stream()
  child.stderr = stream()
  child.stdin = { end: () => {} }
  child.kill = () => { killed = true }
  const session = await startManager(CONFIG, { preflight: () => [], spawnFn: () => child, timeoutMs: 20 })
  assert.equal(session.ok, false)
  assert.match(session.problem, /启动超时/)
  assert.equal(killed, true)
})

test('managerRequest sends the token and unwraps the body', async () => {
  const seen = []
  const fetchFn = async (url, options) => {
    seen.push([url, options.method, options.headers['x-cicada-manager-token'], options.body])
    return { ok: true, status: 200, json: async () => ({ entries: [{ part: 'NE555P' }] }) }
  }
  const result = await managerRequest({ endpoint: 'http://127.0.0.1:1', token: 'tok' }, '/datasheets', { fetchFn })
  assert.equal(result.ok, true)
  assert.equal(result.data.entries[0].part, 'NE555P')
  assert.deepEqual(seen, [['http://127.0.0.1:1/datasheets', 'GET', 'tok', undefined]])
  await managerRequest({ endpoint: 'http://127.0.0.1:1', token: 'tok' }, '/datasheets/X/export', {
    method: 'POST', body: { targetDir: '/tmp' }, fetchFn,
  })
  assert.equal(seen[1][3], JSON.stringify({ targetDir: '/tmp' }))
})

test('managerRequest reports http and transport failures', async () => {
  const failing = async () => ({ ok: false, status: 404, json: async () => ({ error: 'not_found', message: '条目不存在' }) })
  const notFound = await managerRequest({ endpoint: 'http://127.0.0.1:1', token: 't' }, '/datasheets/X', { fetchFn: failing })
  assert.deepEqual([notFound.ok, notFound.status, notFound.problem], [false, 404, '条目不存在'])
  const down = await managerRequest({ endpoint: 'http://127.0.0.1:1', token: 't' }, '/datasheets', {
    fetchFn: async () => { throw new Error('ECONNREFUSED') },
  })
  assert.deepEqual([down.ok, down.status], [false, 0])
  assert.match(down.problem, /ECONNREFUSED/)
  const idle = await managerRequest(null, '/datasheets')
  assert.match(idle.problem, /未启动/)
})
