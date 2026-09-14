/**
 * Launcher project model (docs/04 §5.2).
 *
 * A "project" is a DSH Workspace: the single authority is DSH's own registry at
 * `<home>/storages/workspace.json`, which the launcher only READS. Creating or
 * renaming a workspace goes through the host's `WorkspaceCommands` once the
 * stack runs — writing that file here would create a second authority over a
 * versioned format.
 *
 * What the launcher adds on top is presentation: project cards (title, path,
 * schematic summary) and the `<home>/projects/<name>/` convention for new ones.
 * Every function takes its filesystem probes as parameters so the whole module
 * is testable without touching a real disk.
 */

import { readFileSync, existsSync, mkdirSync, readdirSync, statSync } from 'node:fs'
import { join } from 'node:path'

/** Characters Windows refuses in a path segment, plus path separators. */
const ILLEGAL_NAME = /[\\/:*?"<>|]/

/**
 * Validate a new project name (it becomes one directory segment).
 * @param name - proposed project name.
 * @returns a problem message, or `''` when the name is usable.
 */
export function checkProjectName(name) {
  const trimmed = name.trim()
  if (trimmed === '') return '项目名不能为空'
  if (trimmed === '.' || trimmed === '..') return '项目名不能是 "." 或 ".."'
  if (ILLEGAL_NAME.test(trimmed)) return '项目名不能包含 \\ / : * ? " < > |'
  if (/[. ]$/.test(trimmed)) return '项目名不能以点或空格结尾'
  if (trimmed.length > 64) return '项目名最多 64 个字符'
  return ''
}

/**
 * Absolute directory of a project created through the launcher.
 * @param config - resolved shell configuration.
 * @param name - project name (already validated by {@link checkProjectName}).
 * @returns the directory path.
 */
export function projectPath(config, name) {
  return join(config.projectsDir, name.trim())
}

/**
 * Read DSH's workspace registry. Missing file = no projects yet; a malformed
 * file is reported rather than swallowed (the launcher shows it instead of
 * silently pretending there are no projects).
 * @param path - registry file path (`<home>/storages/workspace.json`).
 * @param readFile - `readFileSync` override for tests.
 * @returns `{ ok, workspaces, problem }`; workspaces are sorted newest first.
 */
export function readWorkspaceRegistry(path, readFile = readFileSync) {
  let raw
  try {
    raw = readFile(path, 'utf8')
  } catch {
    // No registry yet: the host writes it on first use. An absent file and an
    // unreadable one are indistinguishable here, and both mean "no projects".
    return { ok: true, workspaces: [], problem: '' }
  }
  let parsed
  try {
    parsed = JSON.parse(raw)
  } catch (error) {
    return { ok: false, workspaces: [], problem: `工作区注册表无法解析：${String(error.message ?? error)}` }
  }
  const table = parsed?.tables?.workspaces
  if (table === undefined || table === null || typeof table !== 'object') {
    return { ok: true, workspaces: [], problem: '' }
  }
  const workspaces = Object.entries(table)
    .filter(([, value]) => value !== null && typeof value === 'object')
    .map(([workspaceId, value]) => ({
      workspaceId,
      path: typeof value.path === 'string' ? value.path : '',
      title: typeof value.title === 'string' && value.title !== '' ? value.title : value.path,
      sessions: Array.isArray(value.sessionIds) ? value.sessionIds.length : 0,
      updatedAt: typeof value.updatedAt === 'string' ? value.updatedAt : '',
    }))
    .filter((workspace) => workspace.path !== '')
  workspaces.sort((a, b) => b.updatedAt.localeCompare(a.updatedAt))
  return { ok: true, workspaces, problem: '' }
}

/**
 * Summarize the schematic of one project directory by reading the runtime's
 * `view.json` (the semantic view the producer keeps current). Absent = the
 * project has no schematic yet.
 * @param dir - project directory.
 * @param readFile - `readFileSync` override for tests.
 * @param exists - existence probe override for tests.
 * @returns `{ schematic, components, wires, nets, savedAt }`.
 */
export function summarizeProject(dir, readFile = readFileSync, exists = existsSync) {
  const empty = { schematic: false, components: 0, wires: 0, nets: 0, savedAt: 0 }
  if (!exists(join(dir, '.cicada', 'schematic.cicada_sch'))) return empty
  try {
    const view = JSON.parse(readFile(join(dir, '.cicada', 'view.json'), 'utf8'))
    return {
      schematic: true,
      components: Array.isArray(view.components) ? view.components.length : 0,
      nets: Array.isArray(view.nets) ? view.nets.length : 0,
      wires: 0,
      savedAt: typeof view.savedAt === 'number' ? view.savedAt : 0,
    }
  } catch {
    // A schematic without a readable view.json is normal right after a hand
    // edit; report "has a schematic" without counts.
    return { ...empty, schematic: true }
  }
}

/**
 * Last-modified instant of a directory (ms), or 0 when it cannot be read.
 * @param dir - directory path.
 * @param stat - `statSync` override for tests.
 * @returns epoch milliseconds.
 */
export function modifiedAt(dir, stat = statSync) {
  try {
    return stat(dir).mtimeMs
  } catch {
    return 0
  }
}

/**
 * List the launcher's projects: registered workspaces (DSH registry) merged with
 * unregistered directories under `<home>/projects/`.
 * @param config - resolved shell configuration.
 * @param deps - `{ readFile, exists, readdir, stat }` overrides for tests.
 * @returns `{ projects, problems }` — projects sorted by last activity.
 */
export function listProjects(config, deps = {}) {
  const readFile = deps.readFile ?? readFileSync
  const exists = deps.exists ?? existsSync
  const readdir = deps.readdir ?? readdirSync
  const stat = deps.stat ?? statSync
  const registry = readWorkspaceRegistry(config.workspaceRegistry, readFile)
  const problems = registry.problem === '' ? [] : [registry.problem]

  const byPath = new Map()
  for (const workspace of registry.workspaces) {
    byPath.set(workspace.path.toLowerCase(), {
      ...workspace,
      registered: true,
      exists: exists(workspace.path),
      ...summarizeProject(workspace.path, readFile, exists),
      modifiedAt: modifiedAt(workspace.path, stat),
    })
  }
  let names = []
  try {
    names = readdir(config.projectsDir, { withFileTypes: true })
      .filter((entry) => entry.isDirectory())
      .map((entry) => entry.name)
  } catch {
    // `<home>/projects` is created with the first project; no directory yet is
    // the normal first-run state, not a problem worth reporting.
    names = []
  }
  for (const name of names) {
    const dir = join(config.projectsDir, name)
    if (byPath.has(dir.toLowerCase())) continue
    byPath.set(dir.toLowerCase(), {
      workspaceId: '',
      title: name,
      path: dir,
      sessions: 0,
      updatedAt: '',
      registered: false,
      exists: true,
      ...summarizeProject(dir, readFile, exists),
      modifiedAt: modifiedAt(dir, stat),
    })
  }
  const projects = [...byPath.values()].sort((a, b) =>
    (b.modifiedAt - a.modifiedAt) || a.title.localeCompare(b.title))
  return { projects, problems }
}

/**
 * Create `<home>/projects/<name>/` (idempotent). Registration into DSH's
 * registry happens when the stack starts for that project, never here.
 * @param config - resolved shell configuration.
 * @param name - project name.
 * @param deps - `{ exists, mkdir }` overrides for tests.
 * @returns `{ ok, path, problem, created }`.
 */
export function createProject(config, name, deps = {}) {
  const problem = checkProjectName(name)
  if (problem !== '') return { ok: false, path: '', problem, created: false }
  const target = projectPath(config, name)
  const exists = deps.exists ?? existsSync
  const mkdir = deps.mkdir ?? mkdirSync
  const created = !exists(target)
  try {
    mkdir(target, { recursive: true })
  } catch (error) {
    return { ok: false, path: target, problem: `无法创建项目目录：${String(error.message ?? error)}`, created: false }
  }
  return { ok: true, path: target, problem: '', created }
}
