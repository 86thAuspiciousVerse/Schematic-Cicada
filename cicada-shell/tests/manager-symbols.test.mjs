import { test } from 'node:test'
import assert from 'node:assert/strict'
import { join } from 'node:path'
import {
  checkSymbolFile, checkSymbolKey, exportSymbol, findSymbol, importSymbol, listSymbols, rebuildBlock, safeSegment,
  saveSymbolToUserLib, scanLibrary, trashSymbol,
} from '../src/manager/symbols.js'

const LIB = join('/repo', 'assets', 'cicada-libs')
const USER = join('/home', 'cicada', 'cicada-user-lib')
const TRASH = join('/home', 'cicada', 'trash')

// Dirent-shaped: the scanner asks both questions.
const dir = (name) => ({ name, isDirectory: () => true, isFile: () => false })
const file = (name) => ({ name, isDirectory: () => false, isFile: () => true })

/** Two-level layout: <dir>/<CATEGORY>/<name>.kicad_sym. */
const tree = {
  [LIB]: [dir('R'), dir('POWER')],
  [join(LIB, 'R')]: [file('R.kicad_sym'), file('R_Small.kicad_sym'), file('notes.txt')],
  [join(LIB, 'POWER')]: [file('GND.kicad_sym')],
}
const readdir = (path) => {
  if (tree[path] === undefined) throw new Error(`ENOENT ${path}`)
  return tree[path]
}
const stat = (path) => ({ size: path.endsWith('R.kicad_sym') ? 900 : 100, mtimeMs: 1789000000000 })
const deps = { readdir, stat }

test('scans a two-level library into category:name keys', () => {
  const { entries, problems } = scanLibrary(LIB, 'curated', deps)
  assert.deepEqual(problems, [])
  assert.deepEqual(entries.map((entry) => entry.key), ['POWER:GND', 'R:R', 'R:R_Small'])
  assert.deepEqual(entries.map((entry) => entry.source), ['curated', 'curated', 'curated'])
  assert.equal(entries.find((entry) => entry.key === 'R:R').bytes, 900)
})

test('accepts the flat layout the engine also accepts', () => {
  const flat = { [LIB]: [file('R.kicad_sym')] }
  const { entries } = scanLibrary(LIB, 'curated', { readdir: (path) => flat[path] ?? [], stat })
  assert.deepEqual(entries.map((entry) => entry.key), ['R:R'])
  assert.equal(entries[0].flat, true)
})

test('a missing library directory is empty, bad or unreadable ones are reported', () => {
  assert.deepEqual(scanLibrary(join('/nope'), 'user', deps), { entries: [], problems: [] })
  // A category name that is not a safe path segment, plus a directory that
  // cannot be listed: both are reported instead of silently dropped.
  const broken = { [LIB]: [dir('bad:dir'), dir('OK')] }
  const { problems } = scanLibrary(LIB, 'curated', {
    readdir: (path) => {
      if (broken[path] !== undefined) return broken[path]
      throw new Error(`ENOENT ${path}`)
    },
    stat,
  })
  assert.equal(problems.length, 2)
  assert.match(problems[0], /可疑目录/)
  assert.match(problems[1], /无法读取目录/)
})

test('lists both libraries and finds one entry by source + key', () => {
  const listed = listSymbols(LIB, USER, deps)
  assert.equal(listed.curated.length, 3)
  assert.deepEqual(listed.user, [])
  assert.equal(findSymbol(LIB, USER, 'curated', 'R:R', deps).name, 'R')
  assert.equal(findSymbol(LIB, USER, 'user', 'R:R', deps), undefined)
  assert.equal(findSymbol(LIB, USER, 'curated', '../etc', deps), undefined)
})

test('validates keys and path segments', () => {
  assert.equal(checkSymbolKey('R:R_Small'), '')
  assert.equal(safeSegment('R_Small'), '')
  for (const bad of ['', 'R', 'R:R:R', 'a/b:c', 'R:a\\b', 'R:..', 'R:*']) {
    assert.notEqual(checkSymbolKey(bad), '', `expected ${JSON.stringify(bad)} to be refused`)
  }
})

test('delete moves a USER symbol to the trash and refuses the curated library', () => {
  const calls = []
  const ok = trashSymbol(USER, TRASH, 'IC:NE555P', {
    exists: () => true,
    mkdir: (path, options) => calls.push(['mkdir', path, options]),
    rename: (from, to) => calls.push(['rename', from, to]),
    now: new Date('2026-09-13T01:02:03'),
  })
  assert.equal(ok.ok, true)
  assert.equal(ok.movedTo, join(TRASH, '20260913-010203', 'cicada-user-lib', 'IC', 'NE555P.kicad_sym'))
  assert.deepEqual(calls[1], ['rename', join(USER, 'IC', 'NE555P.kicad_sym'), ok.movedTo])
  // The wire cannot even express "delete from curated": the route refuses it,
  // and this function only ever looks inside the user library.
  const missing = trashSymbol(USER, TRASH, 'IC:NOPE', { exists: () => false })
  assert.equal(missing.ok, false)
  assert.match(missing.problem, /用户库里没有/)
})

test('export and save-to-user copy files and refuse to overwrite', () => {
  const entry = { key: 'R:R', name: 'R', category: 'R', file: join(LIB, 'R', 'R.kicad_sym') }
  const copies = []
  const exported = exportSymbol(entry, join('/tmp', 'out'), {
    exists: () => false, mkdir: () => {}, copy: (from, to) => copies.push([from, to]),
  })
  assert.equal(exported.ok, true)
  assert.equal(exported.target, join('/tmp', 'out', 'R.kicad_sym'))
  const saved = saveSymbolToUserLib(entry, USER, {
    exists: () => false, mkdir: () => {}, copy: (from, to) => copies.push([from, to]),
  })
  assert.equal(saved.ok, true)
  assert.equal(saved.target, join(USER, 'R', 'R.kicad_sym'))
  assert.deepEqual(copies, [
    [join(LIB, 'R', 'R.kicad_sym'), join('/tmp', 'out', 'R.kicad_sym')],
    [join(LIB, 'R', 'R.kicad_sym'), join(USER, 'R', 'R.kicad_sym')],
  ])
  const clash = saveSymbolToUserLib(entry, USER, { exists: () => true })
  assert.match(clash.problem, /已有同名符号/)
  assert.match(exportSymbol(undefined, '/tmp').problem, /不存在/)
  assert.match(exportSymbol(entry, '').problem, /未选择/)
})

const SYM_TEXT = '(kicad_symbol_lib (version 20231120) (generator "kicad_symbol_editor") (symbol "MyPart" (property "Reference" "U" (at 0 0 0))))'

test('import screens the file before it can enter the library', () => {
  assert.match(checkSymbolFile('').problem, /未选择文件/)
  assert.match(checkSymbolFile(join('/tmp', 'a.txt')).problem, /只接受/)
  assert.match(checkSymbolFile(join('/tmp', 'a.kicad_sym'), { exists: () => false }).problem, /文件不存在/)
  assert.match(checkSymbolFile(join('/tmp', 'a.kicad_sym'), { exists: () => true, readFile: () => 'hello' }).problem, /不像 KiCad/)
  const good = checkSymbolFile(join('/tmp', 'MyPart.kicad_sym'), { exists: () => true, readFile: () => SYM_TEXT })
  assert.equal(good.ok, true)
})

test('import writes <category>/<stem>.kicad_sym and refuses silent overwrite', () => {
  const copies = []
  const source = join('/tmp', 'MyPart.kicad_sym')
  const deps = {
    // Path-aware: the imported SOURCE exists, the target in the user library
    // does not (production passes existsSync, which answers both correctly).
    exists: (path) => path === source,
    readFile: () => SYM_TEXT,
    mkdir: () => {},
    copy: (from, to) => copies.push([from, to]),
  }
  const ok = importSymbol(join('/tmp', 'MyPart.kicad_sym'), USER, 'IC', deps)
  assert.equal(ok.ok, true)
  assert.equal(ok.key, 'IC:MyPart')
  assert.equal(ok.target, join(USER, 'IC', 'MyPart.kicad_sym'))
  assert.deepEqual(copies, [[join('/tmp', 'MyPart.kicad_sym'), join(USER, 'IC', 'MyPart.kicad_sym')]])

  const clash = importSymbol(join('/tmp', 'MyPart.kicad_sym'), USER, 'IC', { ...deps, exists: () => true })
  assert.deepEqual([clash.ok, clash.conflict], [false, true])
  assert.match(clash.problem, /显式覆盖/)
  const forced = importSymbol(join('/tmp', 'MyPart.kicad_sym'), USER, 'IC', { ...deps, exists: () => true, overwrite: true })
  assert.equal(forced.ok, true)

  const badCategory = importSymbol(join('/tmp', 'MyPart.kicad_sym'), USER, 'a/b', deps)
  assert.match(badCategory.problem, /类别不合法/)
})

test('rebuildBlock maps the datasheet shape to an engine synthesize body', () => {
  const db = {
    shapeOf: () => ({
      name: 'AMS1117-3.3', refPrefix: 'U', description: 'LDO',
      pins: [
        { number: '3', name: 'VIN', electrical: 'in', side: 'left' },
        { number: '1', name: 'GND', electrical: 'ground', side: 'bottom' },
        { number: '2', name: 'VOUT', electrical: 'power_out', side: 'right' },
      ],
    }),
  }
  const built = rebuildBlock('AMS1117-3.3', db, (value) => (value === 'in' ? 'input' : value === 'ground' ? 'power_in' : value))
  assert.equal(built.ok, true)
  assert.equal(built.block.name, 'AMS1117-3.3')
  assert.deepEqual(built.block.pins.map((pin) => pin.number), ['1', '2', '3'], 'pins must be sorted for deterministic sides')
  assert.deepEqual(built.block.pins.map((pin) => pin.electrical), ['power_in', 'power_out', 'input'])

  const missing = rebuildBlock('NOPE', { shapeOf: () => undefined }, (value) => value)
  assert.equal(missing.ok, false)
  assert.match(missing.problem, /还没有 shape.json/)
  const noPins = rebuildBlock('X', { shapeOf: () => ({ name: 'X', pins: [] }) }, (value) => value)
  assert.match(noPins.problem, /没有引脚/)
  assert.match(rebuildBlock('../x', db, (value) => value).problem, /不能包含/)
})
