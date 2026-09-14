/**
 * Datasheet library operations (docs/04 §5.3/§5.6), kept free of HTTP and of the
 * DSH runtime so they are unit-testable: every function takes its filesystem
 * primitives and a `db` accessor (the shape of `GlobalDatasheetDb`) as
 * parameters.
 *
 * Two rules from the spec live here:
 *  - the library reader is the ONE authority (`GlobalDatasheetDb` in
 *    `cicada-knowledge`); this module never parses index/detail/shape itself;
 *  - every delete MOVES the entry into `<home>/trash/<stamp>/…` — nothing is
 *    ever unlinked, so the UI can promise "restorable".
 */

import { cpSync, existsSync, mkdirSync, readdirSync, renameSync, rmSync, statSync } from 'node:fs'
import { join } from 'node:path'

/** Largest `full.md` slice sent to the window (the file can be megabytes). */
export const FULL_MD_LIMIT = 256 * 1024

/**
 * Validate an entry name from the wire: it must be one path segment, so a
 * crafted request can never address outside the library.
 * @param part - candidate part number.
 * @returns a problem message, or `''` when usable.
 */
export function checkPartName(part) {
  if (typeof part !== 'string' || part.trim() === '') return '型号不能为空'
  if (part.length > 128) return '型号过长'
  if (part.includes('/') || part.includes('\\')) return '型号不能包含路径分隔符'
  if (part === '.' || part === '..' || part.includes('..')) return '型号不能包含 ".."'
  if (/[:*?"<>|]/.test(part)) return '型号不能包含 : * ? " < > |'
  return ''
}

/** `YYYYMMDD-HHMMSS` for one trash run (local time; newest sorts last). */
export function trashStamp(now = new Date()) {
  const pad = (value) => String(value).padStart(2, '0')
  return `${now.getFullYear()}${pad(now.getMonth() + 1)}${pad(now.getDate())}`
    + `-${pad(now.getHours())}${pad(now.getMinutes())}${pad(now.getSeconds())}`
}

/** Directory of the library root itself. */
function datasheetsDir(root) {
  return join(root, 'datasheets')
}

/** Directory of one entry. */
export function entryDir(root, part) {
  return join(datasheetsDir(root), part)
}

/**
 * Directory size in bytes (recursive), or 0 when it cannot be walked.
 * @param dir - directory path.
 * @param readdir - `readdirSync` override for tests.
 * @param stat - `statSync` override for tests.
 * @returns byte count.
 */
export function directoryBytes(dir, readdir = readdirSync, stat = statSync) {
  let total = 0
  let entries
  try {
    entries = readdir(dir, { withFileTypes: true })
  } catch {
    return 0
  }
  for (const entry of entries) {
    const path = join(dir, entry.name)
    if (entry.isDirectory()) total += directoryBytes(path, readdir, stat)
    else {
      try {
        total += stat(path).size
      } catch {
        // A file that vanished mid-walk contributes nothing; the number is
        // informational only.
      }
    }
  }
  return total
}

/**
 * List every datasheet entry in the shared library.
 * @param root - library root (`<CICADA_HOME>`; the DB appends `datasheets/`).
 * @param db - `GlobalDatasheetDb` instance (the single reader authority).
 * @param deps - `{ readdir, stat }` overrides for tests.
 * @returns `{ entries, problems }`; entries sorted by part number.
 */
export function listDatasheets(root, db, deps = {}) {
  const readdir = deps.readdir ?? readdirSync
  const stat = deps.stat ?? statSync
  const problems = []
  let names = []
  try {
    names = readdir(datasheetsDir(root), { withFileTypes: true })
      .filter((entry) => entry.isDirectory())
      .map((entry) => entry.name)
  } catch {
    // Library not created yet (no publish ever happened): an empty list, not a
    // failure the user should see.
    return { entries: [], problems }
  }
  const entries = []
  for (const part of names.sort((a, b) => a.localeCompare(b))) {
    if (checkPartName(part) !== '') {
      problems.push(`跳过可疑条目：${part}`)
      continue
    }
    const status = db.statusOf(part)
    if (!status.found) {
      problems.push(`跳过无法读取的条目：${part}（index.json 缺失或不可解析）`)
      continue
    }
    const dir = entryDir(root, part)
    let modifiedAt = 0
    try {
      modifiedAt = stat(dir).mtimeMs
    } catch {
      // Directory vanished between listing and stat: keep the row, no timestamp.
    }
    entries.push({
      part,
      package: db.get(part)?.index?.package ?? '',
      groups: status.groups,
      pins: db.shapeOf(part)?.pins?.length ?? 0,
      haveIndex: status.haveIndex,
      haveDetail: status.haveDetail,
      haveShape: status.haveShape,
      missingDetailGroups: status.missingDetailGroups,
      audited: status.audited === true,
      modifiedAt,
      bytes: directoryBytes(dir, readdir, stat),
    })
  }
  return { entries, problems }
}

/**
 * Read one entry for the detail view: shape pins, per-group detail and a capped
 * `full.md` (the source text is only a preview in the window).
 * @param root - library root.
 * @param db - `GlobalDatasheetDb` instance.
 * @param part - exact part number.
 * @returns the entry view, or undefined when the entry does not exist.
 */
export function readDatasheet(root, db, part) {
  if (checkPartName(part) !== '') return undefined
  const entry = db.get(part)
  if (entry === undefined) return undefined
  const status = db.statusOf(part)
  const fullMd = entry.fullMd ?? ''
  const groups = (entry.index.groups ?? []).map((group) => ({
    groupId: group.group_id,
    title: group.title ?? '',
    category: group.category ?? '',
    priority: group.priority ?? '',
    brief: group.brief ?? '',
    // v3（2026-09-13）：组自述 + 设计指导层要能在资料库页看到，否则页面对新契约是瞎的。
    description: group.description ?? '',
    designNotes: group.design_notes ?? [],
    externalComponents: (entry.detail[group.group_id]?.external_components ?? group.external_components ?? []).map((item) => ({
      kind: item.ref_kind ?? '',
      value: item.value ?? '',
      connection: item.connection ?? '',
      why: item.why ?? '',
    })),
    operatingLimits: (entry.detail[group.group_id]?.operating_limits ?? group.operating_limits ?? []).map((item) => ({
      name: item.name ?? '',
      value: item.value ?? '',
      condition: item.condition ?? '',
    })),
    location: group.location === undefined ? '' : `${String(group.location.line_start ?? '')}-${String(group.location.line_end ?? '')}`,
    detail: entry.detail[group.group_id] === undefined,
    pins: (entry.detail[group.group_id]?.pins ?? group.pins ?? []).map((pin) => ({
      number: pin.physical_number ?? '',
      name: pin.canonical_name ?? '',
      electrical: pin.electrical ?? '',
      functions: (pin.functions ?? []).map((fn) => fn.name ?? ''),
      claims: (pin.source_claim_ids ?? []).length,
    })),
    claims: (group.source_claims ?? []).map((claim) => ({
      id: claim.claim_id ?? '',
      ref: claim.source_ref ?? '',
      fact: claim.extracted_fact ?? '',
    })),
  }))
  return {
    part,
    package: entry.index.package ?? '',
    schemaVersion: entry.index.schema_version ?? '',
    audited: status.audited === true,
    modifiedAt: status.modified_at ?? 0,
    groups,
    shape: (entry.shape?.pins ?? []).map((pin) => ({
      number: pin.number ?? '',
      name: pin.name ?? '',
      electrical: pin.electrical ?? '',
      side: pin.side ?? '',
    })),
    bytes: directoryBytes(entryDir(root, part)),
    fullMd: fullMd.slice(0, FULL_MD_LIMIT),
    fullMdBytes: Buffer.byteLength(fullMd, 'utf8'),
    fullMdTruncated: Buffer.byteLength(fullMd, 'utf8') > FULL_MD_LIMIT,
  }
}

/**
 * Move one entry into the trash (docs/04 §5.6). Same-volume rename; a rename
 * across devices falls back to copy + remove.
 * @param root - library root.
 * @param trashDir - `<home>/trash`.
 * @param part - exact part number.
 * @param deps - `{ exists, mkdir, rename, cp, rm, now }` overrides for tests.
 * @returns `{ ok, movedTo, problem }`.
 */
export function trashDatasheet(root, trashDir, part, deps = {}) {
  const problem = checkPartName(part)
  if (problem !== '') return { ok: false, movedTo: '', problem }
  const exists = deps.exists ?? existsSync
  const mkdir = deps.mkdir ?? mkdirSync
  const rename = deps.rename ?? renameSync
  const cp = deps.cp ?? cpSync
  const rm = deps.rm ?? rmSync
  const source = entryDir(root, part)
  if (!exists(source)) return { ok: false, movedTo: '', problem: `条目不存在：${part}` }
  const target = join(trashDir, trashStamp(deps.now ?? new Date()), 'datasheets', part)
  try {
    mkdir(join(target, '..'), { recursive: true })
    try {
      rename(source, target)
    } catch {
      // Different volume (or the rename was refused): copy, then only remove the
      // source once the copy succeeded.
      cp(source, target, { recursive: true, force: false, errorOnExist: true })
      rm(source, { recursive: true, force: true })
    }
  } catch (error) {
    return { ok: false, movedTo: '', problem: `移入回收站失败：${String(error.message ?? error)}` }
  }
  return { ok: true, movedTo: target, problem: '' }
}

/**
 * Copy one entry out of the library (export): the library itself is untouched.
 * @param root - library root.
 * @param targetDir - destination directory chosen by the user.
 * @param part - exact part number.
 * @param deps - `{ exists, mkdir, cp }` overrides for tests.
 * @returns `{ ok, target, problem }`.
 */
export function exportDatasheet(root, targetDir, part, deps = {}) {
  const problem = checkPartName(part)
  if (problem !== '') return { ok: false, target: '', problem }
  if (typeof targetDir !== 'string' || targetDir.trim() === '') {
    return { ok: false, target: '', problem: '未选择导出目录' }
  }
  const exists = deps.exists ?? existsSync
  const mkdir = deps.mkdir ?? mkdirSync
  const cp = deps.cp ?? cpSync
  const source = entryDir(root, part)
  if (!exists(source)) return { ok: false, target: '', problem: `条目不存在：${part}` }
  const target = join(targetDir, part)
  if (exists(target)) return { ok: false, target, problem: `目标已存在同名目录：${target}` }
  try {
    mkdir(targetDir, { recursive: true })
    cp(source, target, { recursive: true, force: false, errorOnExist: true })
  } catch (error) {
    return { ok: false, target, problem: `导出失败：${String(error.message ?? error)}` }
  }
  return { ok: true, target, problem: '' }
}
