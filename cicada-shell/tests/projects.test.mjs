import { test } from 'node:test'
import assert from 'node:assert/strict'
import { join } from 'node:path'
import {
  checkProjectName, createProject, listProjects, projectPath, readWorkspaceRegistry, summarizeProject,
} from '../src/projects.js'

const HOME = join('/home', 'cicada')
const CONFIG = {
  home: HOME,
  projectsDir: join(HOME, 'projects'),
  workspaceRegistry: join(HOME, 'storages', 'workspace.json'),
}

/** Registry payload shaped like DSH's workspace storage. */
function registry(workspaces) {
  return JSON.stringify({
    unit: { name: 'workspace', version: 2 },
    global: { initialized: true, workspaceIds: Object.keys(workspaces), archivedSessionIds: [] },
    tables: { workspaces },
  })
}

test('reads DSH workspace registry entries (newest first)', () => {
  const file = registry({
    a: { path: 'C:\\proj\\alpha', title: 'alpha', sessionIds: ['s1', 's2'], updatedAt: '2026-09-01T00:00:00.000Z' },
    b: { path: 'C:\\proj\\beta', title: 'beta', sessionIds: [], updatedAt: '2026-09-10T00:00:00.000Z' },
  })
  const result = readWorkspaceRegistry(CONFIG.workspaceRegistry, () => file)
  assert.equal(result.ok, true)
  assert.deepEqual(result.workspaces.map((w) => w.title), ['beta', 'alpha'])
  assert.equal(result.workspaces[1].sessions, 2)
})

test('a missing registry is "no projects yet", a malformed one is reported', () => {
  const missing = readWorkspaceRegistry(CONFIG.workspaceRegistry, () => { throw new Error('ENOENT') })
  assert.deepEqual(missing, { ok: true, workspaces: [], problem: '' })
  const broken = readWorkspaceRegistry(CONFIG.workspaceRegistry, () => '{not json')
  assert.equal(broken.ok, false)
  assert.match(broken.problem, /无法解析/)
})

test('titles fall back to the path and path-less rows are dropped', () => {
  const file = registry({
    a: { path: 'C:\\proj\\alpha', sessionIds: [] },
    b: { title: 'orphan', sessionIds: [] },
  })
  const result = readWorkspaceRegistry(CONFIG.workspaceRegistry, () => file)
  assert.deepEqual(result.workspaces.map((w) => w.title), ['C:\\proj\\alpha'])
})

test('summarizes a project from the runtime view.json', () => {
  const readFile = (path) => {
    assert.equal(path, join('C:\\proj\\alpha', '.cicada', 'view.json'))
    return JSON.stringify({ savedAt: 1789227213478, components: [{}, {}, {}], nets: [{}, {}] })
  }
  const summary = summarizeProject('C:\\proj\\alpha', readFile, () => true)
  assert.deepEqual(summary, { schematic: true, components: 3, wires: 0, nets: 2, savedAt: 1789227213478 })
})

test('a project without a schematic or view.json reports empty, not an error', () => {
  assert.deepEqual(summarizeProject('C:\\proj\\alpha', () => { throw new Error('ENOENT') }, () => false),
    { schematic: false, components: 0, wires: 0, nets: 0, savedAt: 0 })
  assert.deepEqual(summarizeProject('C:\\proj\\alpha', () => { throw new Error('ENOENT') }, () => true),
    { schematic: true, components: 0, wires: 0, nets: 0, savedAt: 0 })
})

test('merges registered workspaces with unregistered directories under projectsDir', () => {
  const registered = join(CONFIG.projectsDir, 'alpha')
  const unregistered = join(CONFIG.projectsDir, 'beta')
  const deps = {
    readFile: () => registry({ a: { path: registered, title: 'alpha', sessionIds: ['s1'], updatedAt: '2026-09-01T00:00:00.000Z' } }),
    exists: () => true,
    readdir: () => [{ name: 'alpha', isDirectory: () => true }, { name: 'beta', isDirectory: () => true }],
    stat: (path) => ({ mtimeMs: path === unregistered ? 200 : 100 }),
  }
  const { projects, problems } = listProjects(CONFIG, deps)
  assert.deepEqual(problems, [])
  assert.deepEqual(projects.map((p) => [p.title, p.registered]), [['beta', false], ['alpha', true]])
})

test('a missing projectsDir is the normal first run, not a problem', () => {
  const deps = {
    readFile: () => { throw new Error('ENOENT') },
    exists: () => false,
    readdir: () => { throw new Error('ENOENT') },
    stat: () => { throw new Error('ENOENT') },
  }
  assert.deepEqual(listProjects(CONFIG, deps), { projects: [], problems: [] })
})

test('rejects unusable project names and accepts ordinary ones', () => {
  assert.equal(checkProjectName('ne555-blink'), '')
  assert.equal(checkProjectName(' 放大器 '), '')
  assert.match(checkProjectName(''), /不能为空/)
  assert.match(checkProjectName('a/b'), /不能包含/)
  assert.match(checkProjectName('a:b'), /不能包含/)
  assert.match(checkProjectName('..'), /不能是/)
  assert.match(checkProjectName('x.'), /结尾/)
  assert.match(checkProjectName('x'.repeat(65)), /64/)
})

test('createProject makes <projectsDir>/<name> and is idempotent', () => {
  const made = []
  const first = createProject(CONFIG, ' blink ', { exists: () => false, mkdir: (p, o) => made.push([p, o]) })
  assert.equal(first.ok, true)
  assert.equal(first.created, true)
  assert.equal(first.path, projectPath(CONFIG, 'blink'))
  assert.deepEqual(made, [[join(CONFIG.projectsDir, 'blink'), { recursive: true }]])
  const again = createProject(CONFIG, 'blink', { exists: () => true, mkdir: () => {} })
  assert.equal(again.created, false)
  const bad = createProject(CONFIG, 'a/b', { exists: () => false, mkdir: () => {} })
  assert.equal(bad.ok, false)
  assert.match(bad.problem, /不能包含/)
})
