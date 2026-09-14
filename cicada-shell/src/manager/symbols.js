/**
 * Symbol library operations (docs/04 §5.4), the filesystem half of the symbol
 * page: enumerate the curated (read-only) and user (writable) libraries, and
 * move/copy `.kicad_sym` files. Pin geometry and the canonical keys come from the
 * ENGINE (`/lib/list`, `/lib/get`) — this module never parses a symbol file, so
 * there is exactly one authority for what a symbol contains.
 *
 * Layout (same rule as the engine's loader): `<dir>/<CATEGORY>/<name>.kicad_sym`
 * with a flat `<dir>/<name>.kicad_sym` accepted as `<stem>:<stem>`.
 */

import { copyFileSync, existsSync, mkdirSync, readFileSync, readdirSync, renameSync, rmSync, statSync } from 'node:fs'
import { join } from 'node:path'

import { checkPartName, trashStamp } from './core.js'

/** One path segment, so a request can never address outside the library. */
export function safeSegment(value) {
  if (typeof value !== 'string' || value.trim() === '') return '名称不能为空'
  if (value.length > 128) return '名称过长'
  if (value.includes('/') || value.includes('\\') || value.includes('..')) return '名称不能包含路径分隔符'
  if (/[:*?"<>|]/.test(value)) return '名称不能包含 : * ? " < > |'
  return ''
}

/**
 * Validate a `category:name` library key from the wire.
 * @param key - candidate key.
 * @returns a problem message, or `''` when usable.
 */
export function checkSymbolKey(key) {
  if (typeof key !== 'string' || key === '') return '库键不能为空'
  const parts = key.split(':')
  if (parts.length !== 2) return '库键必须形如 category:name'
  return safeSegment(parts[0]) === '' && safeSegment(parts[1]) === '' ? '' : '库键的类别或名称不合法'
}

/** The engine key of one library file. */
function keyOfFile(category, fileName) {
  const stem = fileName.replace(/\.kicad_sym$/i, '')
  return `${category}:${stem}`
}

/**
 * Enumerate one library directory.
 * @param dir - library root (`<libDir>` or `<userLibDir>`).
 * @param source - `'curated'` or `'user'` (carried into every entry).
 * @param deps - `{ readdir, stat, exists }` overrides for tests.
 * @returns `{ entries, problems }`.
 */
export function scanLibrary(dir, source, deps = {}) {
  const readdir = deps.readdir ?? readdirSync
  const stat = deps.stat ?? statSync
  const problems = []
  const entries = []
  let top
  try {
    top = readdir(dir, { withFileTypes: true })
  } catch {
    // Library directory absent (user lib before the first synthesis): empty.
    return { entries, problems }
  }
  for (const item of top.sort((a, b) => a.name.localeCompare(b.name))) {
    const path = join(dir, item.name)
    if (item.isDirectory()) {
      if (safeSegment(item.name) !== '') {
        problems.push(`跳过可疑目录：${item.name}`)
        continue
      }
      let files = []
      try {
        files = readdir(path, { withFileTypes: true })
      } catch {
        problems.push(`无法读取目录：${path}`)
        continue
      }
      for (const file of files) {
        if (!file.isFile() || !/\.kicad_sym$/i.test(file.name)) continue
        entries.push(entryFor(join(path, file.name), item.name, file.name, source, stat))
      }
      continue
    }
    if (item.isFile() && /\.kicad_sym$/i.test(item.name)) {
      // Flat layout: the engine registers `<stem>:<stem>` for these.
      const stem = item.name.replace(/\.kicad_sym$/i, '')
      entries.push({ ...entryFor(path, stem, item.name, source, stat), flat: true })
    }
  }
  return { entries, problems }
}

/** One library file descriptor. */
function entryFor(file, category, fileName, source, stat) {
  let bytes = 0
  let modifiedAt = 0
  try {
    const info = stat(file)
    bytes = info.size
    modifiedAt = info.mtimeMs
  } catch {
    // File vanished mid-scan: keep the row, no size/time.
  }
  return { key: keyOfFile(category, fileName), category, name: fileName.replace(/\.kicad_sym$/i, ''), file, source, bytes, modifiedAt }
}

/**
 * Both libraries at once.
 * @param libDir - curated library root.
 * @param userLibDir - user library root (writable).
 * @param deps - {@link scanLibrary} overrides.
 * @returns `{ curated, user, problems }`.
 */
export function listSymbols(libDir, userLibDir, deps = {}) {
  const curated = scanLibrary(libDir, 'curated', deps)
  const user = scanLibrary(userLibDir, 'user', deps)
  return {
    curated: curated.entries,
    user: user.entries,
    problems: [...curated.problems, ...user.problems],
  }
}

/**
 * Locate one symbol file by source and key.
 * @param libDir - curated library root.
 * @param userLibDir - user library root.
 * @param source - `'curated'` or `'user'`.
 * @param key - `category:name`.
 * @param deps - {@link scanLibrary} overrides.
 * @returns the entry, or undefined.
 */
export function findSymbol(libDir, userLibDir, source, key, deps = {}) {
  if (checkSymbolKey(key) !== '') return undefined
  const listed = source === 'user' ? scanLibrary(userLibDir, 'user', deps) : scanLibrary(libDir, 'curated', deps)
  return listed.entries.find((entry) => entry.key === key)
}

/**
 * Move a USER-library symbol into the trash (the curated library is read-only,
 * docs/04 §5.4/§5.6).
 * @param userLibDir - user library root.
 * @param trashDir - `<home>/trash`.
 * @param key - `category:name`.
 * @param deps - `{ exists, mkdir, rename, copy, rm, now }` overrides for tests.
 * @returns `{ ok, movedTo, problem }`.
 */
export function trashSymbol(userLibDir, trashDir, key, deps = {}) {
  const problem = checkSymbolKey(key)
  if (problem !== '') return { ok: false, movedTo: '', problem }
  const [category, name] = key.split(':')
  const exists = deps.exists ?? existsSync
  const mkdir = deps.mkdir ?? mkdirSync
  const rename = deps.rename ?? renameSync
  const copy = deps.copy ?? copyFileSync
  const rm = deps.rm ?? rmSync
  const source = join(userLibDir, category, `${name}.kicad_sym`)
  if (!exists(source)) return { ok: false, movedTo: '', problem: `用户库里没有 ${key}` }
  const target = join(trashDir, trashStamp(deps.now ?? new Date()), 'cicada-user-lib', category, `${name}.kicad_sym`)
  try {
    mkdir(join(target, '..'), { recursive: true })
    try {
      rename(source, target)
    } catch {
      copy(source, target)
      rm(source, { force: true })
    }
  } catch (error) {
    return { ok: false, movedTo: '', problem: `移入回收站失败：${String(error.message ?? error)}` }
  }
  return { ok: true, movedTo: target, problem: '' }
}

/**
 * Copy one symbol file out of a library.
 * @param entry - library entry from {@link listSymbols}.
 * @param targetDir - destination directory chosen by the user.
 * @param deps - `{ exists, mkdir, copy }` overrides for tests.
 * @returns `{ ok, target, problem }`.
 */
export function exportSymbol(entry, targetDir, deps = {}) {
  if (entry === undefined) return { ok: false, target: '', problem: '符号不存在' }
  if (typeof targetDir !== 'string' || targetDir.trim() === '') return { ok: false, target: '', problem: '未选择导出目录' }
  const exists = deps.exists ?? existsSync
  const mkdir = deps.mkdir ?? mkdirSync
  const copy = deps.copy ?? copyFileSync
  const target = join(targetDir, `${entry.name}.kicad_sym`)
  if (exists(target)) return { ok: false, target, problem: `目标已存在同名文件：${target}` }
  try {
    mkdir(targetDir, { recursive: true })
    copy(entry.file, target)
  } catch (error) {
    return { ok: false, target, problem: `导出失败：${String(error.message ?? error)}` }
  }
  return { ok: true, target, problem: '' }
}

/**
 * "Save a copy into the user library" for a curated symbol (docs/04 §5.4).
 * @param entry - curated entry.
 * @param userLibDir - user library root.
 * @param deps - `{ exists, mkdir, copy }` overrides for tests.
 * @returns `{ ok, target, problem }`.
 */
export function saveSymbolToUserLib(entry, userLibDir, deps = {}) {
  if (entry === undefined) return { ok: false, target: '', problem: '符号不存在' }
  const exists = deps.exists ?? existsSync
  const mkdir = deps.mkdir ?? mkdirSync
  const copy = deps.copy ?? copyFileSync
  const target = join(userLibDir, entry.category, `${entry.name}.kicad_sym`)
  if (exists(target)) return { ok: false, target, problem: `用户库已有同名符号：${entry.key}` }
  try {
    mkdir(join(target, '..'), { recursive: true })
    copy(entry.file, target)
  } catch (error) {
    return { ok: false, target, problem: `另存失败：${String(error.message ?? error)}` }
  }
  return { ok: true, target, problem: '' }
}

/** A `.kicad_sym` must at least look like one before it enters the library. */
const SYMBOL_FILE_MARKERS = ['(kicad_symbol_lib', '(symbol "']

/**
 * Screen an imported file: it must be readable, carry the library header and at
 * least one symbol. Deliberately shallow — the ENGINE is what really parses a
 * symbol, and a file that fails there shows up as "引擎未列出" in the table
 * rather than as a silent success here.
 * @param file - candidate `.kicad_sym` path.
 * @param deps - `{ exists, readFile }` overrides for tests.
 * @returns `{ ok, problem, text }`.
 */
export function checkSymbolFile(file, deps = {}) {
  const exists = deps.exists ?? existsSync
  const read = deps.readFile ?? readFileSync
  if (typeof file !== 'string' || file.trim() === '') return { ok: false, problem: '未选择文件', text: '' }
  if (!/\.kicad_sym$/i.test(file)) return { ok: false, problem: '只接受 .kicad_sym 文件', text: '' }
  if (!exists(file)) return { ok: false, problem: `文件不存在：${file}`, text: '' }
  let text = ''
  try {
    text = read(file, 'utf8')
  } catch (error) {
    return { ok: false, problem: `无法读取文件：${String(error.message ?? error)}`, text: '' }
  }
  const missing = SYMBOL_FILE_MARKERS.filter((marker) => !text.includes(marker))
  if (missing.length > 0) return { ok: false, problem: `不像 KiCad 符号库文件（缺少 ${missing.join(' / ')}）`, text }
  return { ok: true, problem: '', text }
}

/**
 * Import a user's `.kicad_sym` into the writable user library (docs/04 §5.4/L4).
 * The key is `category:stem`, so the category is the caller's choice and the
 * name comes from the file — and an existing target is refused unless the caller
 * explicitly asks to overwrite.
 * @param sourceFile - the chosen file.
 * @param userLibDir - user library root.
 * @param category - target category (one path segment).
 * @param deps - `{ exists, readFile, mkdir, copy }` + `overwrite` for tests.
 * @returns `{ ok, key, target, problem, conflict }`.
 */
export function importSymbol(sourceFile, userLibDir, category, deps = {}) {
  const categoryProblem = safeSegment(category)
  if (categoryProblem !== '') return { ok: false, key: '', target: '', problem: `类别不合法：${categoryProblem}`, conflict: false }
  const checked = checkSymbolFile(sourceFile, deps)
  if (!checked.ok) return { ok: false, key: '', target: '', problem: checked.problem, conflict: false }
  const name = String(sourceFile).replace(/^.*[\\/]/, '').replace(/\.kicad_sym$/i, '')
  if (safeSegment(name) !== '') return { ok: false, key: '', target: '', problem: '文件名不能作为符号名', conflict: false }
  const key = `${category}:${name}`
  const target = join(userLibDir, category, `${name}.kicad_sym`)
  const exists = deps.exists ?? existsSync
  const mkdir = deps.mkdir ?? mkdirSync
  const copy = deps.copy ?? copyFileSync
  if (exists(target) && deps.overwrite !== true) {
    return { ok: false, key, target, problem: `用户库已有 ${key}（需要显式覆盖）`, conflict: true }
  }
  try {
    mkdir(join(target, '..'), { recursive: true })
    copy(sourceFile, target)
  } catch (error) {
    return { ok: false, key, target, problem: `导入失败：${String(error.message ?? error)}`, conflict: false }
  }
  return { ok: true, key, target, problem: '', conflict: false }
}

/**
 * Build the engine `/lib/synthesize` body for one datasheet entry's shape block
 * (docs/04 §5.4 "从 shape 重建符号"). The pin table is sorted by physical number
 * so the engine's side assignment is deterministic.
 * @param part - exact part number.
 * @param db - `GlobalDatasheetDb` instance (the library reader authority).
 * @param toEngineElectrical - datasheet→engine vocabulary mapping (injected so
 *   this module does not re-implement it; see `cicada-symbols`).
 * @returns `{ ok, block, problem }`.
 */
export function rebuildBlock(part, db, toEngineElectrical) {
  const problem = checkPartName(part)
  if (problem !== '') return { ok: false, block: undefined, problem }
  const shape = db.shapeOf(part)
  if (shape === undefined) return { ok: false, block: undefined, problem: `${part} 还没有 shape.json（先让数据手册代理产出形状块）` }
  const pins = (shape.pins ?? [])
    .map((pin) => ({
      number: String(pin.number ?? ''),
      name: String(pin.name ?? ''),
      electrical: toEngineElectrical(pin.electrical),
      ...(pin.side === undefined || pin.side === '' ? {} : { side: pin.side }),
    }))
    .filter((pin) => pin.number !== '')
    .sort((a, b) => Number(a.number) - Number(b.number))
  if (pins.length === 0) return { ok: false, block: undefined, problem: `${part} 的 shape.json 没有引脚` }
  return {
    ok: true,
    block: {
      name: part,
      ...(shape.refPrefix === undefined || shape.refPrefix === '' ? {} : { refPrefix: shape.refPrefix }),
      ...(shape.description === undefined || shape.description === '' ? {} : { description: shape.description }),
      pins,
    },
    problem: '',
  }
}
