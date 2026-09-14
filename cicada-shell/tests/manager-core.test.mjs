import { test } from 'node:test'
import assert from 'node:assert/strict'
import { join } from 'node:path'
import {
  checkPartName, directoryBytes, exportDatasheet, listDatasheets, readDatasheet, trashDatasheet, trashStamp,
} from '../src/manager/core.js'

const ROOT = join('/home', 'cicada')
const TRASH = join(ROOT, 'trash')

/** `GlobalDatasheetDb`-shaped fake: only the four readers the shell uses. */
function fakeDb(parts) {
  return {
    statusOf: (part) => parts[part]?.status ?? { found: false, part_number: part, haveIndex: false, haveDetail: false, haveShape: false, groups: 0, missingDetailGroups: [] },
    get: (part) => parts[part]?.entry,
    shapeOf: (part) => parts[part]?.entry?.shape,
  }
}

const NE555 = {
  status: { found: true, part_number: 'NE555P', haveIndex: true, haveDetail: true, haveShape: true, groups: 8, missingDetailGroups: [], audited: true, modified_at: 1789000000000 },
  entry: {
    index: { part_number: 'NE555P', package: 'PDIP-8', groups: [{ group_id: 'PIN-001', title: 'Pinout', pins: [{ physical_number: '1' }] }] },
    detail: { 'PIN-001': { group_id: 'PIN-001', pins: [{ physical_number: '1', canonical_name: 'GND', electrical: 'power_in', source_claim_ids: ['c1'] }] } },
    fullMd: '# NE555\n'.repeat(10),
    shape: { name: 'NE555P', pins: [{ number: '1', name: 'GND', electrical: 'power_in', side: 'left' }] },
  },
}

const dirs = (...names) => names.map((name) => ({ name, isDirectory: () => true }))

test('lists library entries through the db reader (not by parsing files here)', () => {
  const db = fakeDb({ NE555P: NE555 })
  const { entries, problems } = listDatasheets(ROOT, db, {
    readdir: (path) => (path === join(ROOT, 'datasheets') ? dirs('NE555P') : []),
    stat: () => ({ mtimeMs: 123, size: 10 }),
  })
  assert.deepEqual(problems, [])
  assert.equal(entries.length, 1)
  assert.deepEqual(
    { part: entries[0].part, package: entries[0].package, groups: entries[0].groups, pins: entries[0].pins, audited: entries[0].audited },
    { part: 'NE555P', package: 'PDIP-8', groups: 8, pins: 1, audited: true },
  )
})

test('an entry with an unreadable index is reported, not silently dropped', () => {
  // 'BROKEN' has a directory but no readable index.json; 'ok' is a real entry.
  const db = fakeDb({ ok: NE555 })
  const { entries, problems } = listDatasheets(ROOT, db, {
    readdir: (path) => (path === join(ROOT, 'datasheets') ? dirs('BROKEN', 'ok') : []),
    stat: () => ({ mtimeMs: 1, size: 1 }),
  })
  assert.deepEqual(entries.map((entry) => entry.part), ['ok'])
  assert.equal(problems.length, 1)
  assert.match(problems[0], /BROKEN/)
})

test('a missing library directory is an empty list, not an error', () => {
  const { entries, problems } = listDatasheets(ROOT, fakeDb({}), {
    readdir: () => { throw new Error('ENOENT') },
    stat: () => { throw new Error('ENOENT') },
  })
  assert.deepEqual({ entries, problems }, { entries: [], problems: [] })
})

test('reads one entry with group pins, claims and shape pins', () => {
  const view = readDatasheet(ROOT, fakeDb({ NE555P: NE555 }), 'NE555P')
  assert.equal(view.part, 'NE555P')
  assert.equal(view.groups[0].groupId, 'PIN-001')
  assert.deepEqual(view.groups[0].pins[0], { number: '1', name: 'GND', electrical: 'power_in', functions: [], claims: 1 })
  assert.deepEqual(view.shape[0], { number: '1', name: 'GND', electrical: 'power_in', side: 'left' })
  assert.equal(view.fullMdTruncated, false)
})

test('caps the full.md preview and reports the real size', () => {
  const big = 'x'.repeat(300 * 1024)
  const view = readDatasheet(ROOT, fakeDb({ BIG: { ...NE555, entry: { ...NE555.entry, fullMd: big } } }), 'BIG')
  assert.equal(view.fullMd.length, 256 * 1024)
  assert.equal(view.fullMdTruncated, true)
  assert.equal(view.fullMdBytes, 300 * 1024)
})

test('rejects path-shaped part numbers (wire input)', () => {
  for (const bad of ['', '..', '../etc', 'a/b', 'a\\b', 'C:', 'x'.repeat(129)]) {
    assert.notEqual(checkPartName(bad), '', `expected ${JSON.stringify(bad)} to be refused`)
  }
  assert.equal(checkPartName('STM32F103C8T6'), '')
  assert.equal(readDatasheet(ROOT, fakeDb({}), '../secrets'), undefined)
})

test('delete moves the entry into the trash and never unlinks it', () => {
  const calls = []
  const result = trashDatasheet(ROOT, TRASH, 'NE555P', {
    exists: () => true,
    mkdir: (path, options) => calls.push(['mkdir', path, options]),
    rename: (from, to) => calls.push(['rename', from, to]),
    cp: () => { throw new Error('must not copy on a same-volume move') },
    rm: () => { throw new Error('must not remove on a same-volume move') },
    now: new Date('2026-09-12T20:15:30'),
  })
  assert.equal(result.ok, true)
  assert.equal(result.movedTo, join(TRASH, '20260912-201530', 'datasheets', 'NE555P'))
  assert.deepEqual(calls, [
    ['mkdir', join(TRASH, '20260912-201530', 'datasheets'), { recursive: true }],
    ['rename', join(ROOT, 'datasheets', 'NE555P'), join(TRASH, '20260912-201530', 'datasheets', 'NE555P')],
  ])
})

test('delete falls back to copy-then-remove across volumes, and reports failures', () => {
  const calls = []
  const ok = trashDatasheet(ROOT, TRASH, 'NE555P', {
    exists: () => true,
    mkdir: () => {},
    rename: () => { throw new Error('EXDEV') },
    cp: (from, to, options) => calls.push(['cp', from, to, options.errorOnExist]),
    rm: (path) => calls.push(['rm', path]),
  })
  assert.equal(ok.ok, true)
  assert.deepEqual(calls, [
    ['cp', join(ROOT, 'datasheets', 'NE555P'), ok.movedTo, true],
    ['rm', join(ROOT, 'datasheets', 'NE555P')],
  ])
  const missing = trashDatasheet(ROOT, TRASH, 'NOPE', { exists: () => false })
  assert.equal(missing.ok, false)
  assert.match(missing.problem, /不存在/)
})

test('trash stamps sort chronologically', () => {
  assert.equal(trashStamp(new Date('2026-01-02T03:04:05')), '20260102-030405')
  assert.ok(trashStamp(new Date('2026-01-02T03:04:05')) < trashStamp(new Date('2026-01-02T03:04:06')))
})

test('export copies out and refuses to overwrite an existing target', () => {
  const copied = []
  const ok = exportDatasheet(ROOT, join('/tmp', 'out'), 'NE555P', {
    exists: (path) => path === join(ROOT, 'datasheets', 'NE555P'),
    mkdir: () => {},
    cp: (from, to, options) => copied.push([from, to, options.force]),
  })
  assert.equal(ok.ok, true)
  assert.deepEqual(copied, [[join(ROOT, 'datasheets', 'NE555P'), join('/tmp', 'out', 'NE555P'), false]])
  const clash = exportDatasheet(ROOT, join('/tmp', 'out'), 'NE555P', { exists: () => true })
  assert.equal(clash.ok, false)
  assert.match(clash.problem, /已存在/)
  const noTarget = exportDatasheet(ROOT, '', 'NE555P', { exists: () => true })
  assert.match(noTarget.problem, /未选择/)
})

test('directory size walks files and survives unreadable branches', () => {
  // Paths are built with join(): a POSIX literal would not match on Windows.
  const entry = join('/lib', 'NE555P')
  const detail = join(entry, 'detail')
  const size = directoryBytes(entry, (path) => {
    if (path === entry) return [{ name: 'a', isDirectory: () => false }, { name: 'detail', isDirectory: () => true }]
    if (path === detail) return [{ name: 'b', isDirectory: () => false }]
    throw new Error('ENOENT')
  }, (path) => ({ size: path.endsWith('a') ? 100 : 7 }))
  assert.equal(size, 107)
  assert.equal(directoryBytes(join('/missing'), () => { throw new Error('ENOENT') }, () => ({ size: 1 })), 0)
})
