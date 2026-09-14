import { test } from 'node:test'
import assert from 'node:assert/strict'

/**
 * The launcher renderer is a plain script with no exports and no Electron in the
 * test environment, so it had never been *executed* by a test — the syntax guard
 * only parses it. This file runs it against a minimal DOM stub: every page must
 * render, and the datasheet page must actually show the rows the manager
 * returned. It catches runtime errors (missing helpers, bad property access)
 * before the window is ever opened.
 */

/** Let the renderer's promise chain (refresh → renderPage) settle. */
const settle = () => new Promise((resolve) => setTimeout(resolve, 10))

/** Text content of a node tree built by the stub. */
function textOf(node) {
  if (node === null || node === undefined) return ''
  if (Array.isArray(node)) return node.map(textOf).join('')
  if (typeof node === 'string') return node
  return `${node.__text ?? ''}${textOf(node.__children ?? [])}`
}

/** Marker class so `child instanceof Node` works like the real DOM. */
class StubNode {}

/** One stub element (enough of the DOM for the renderer). */
function element(tag) {
  const node = new StubNode()
  const data = {}
  const attrs = {}
  Object.assign(node, {
    tagName: tag,
    __children: [],
    __listeners: {},
    __attrs: attrs,
    className: '',
    value: '',
    checked: false,
    scrollTop: 0,
    scrollHeight: 0,
    clientHeight: 0,
    __text: '',
    append(...children) { node.__children.push(...children) },
    prepend(...children) { node.__children.unshift(...children) },
    replaceChildren(...children) { node.__children = [...children] },
    setAttribute(name, value) { attrs[name] = String(value) },
    getAttribute: (name) => attrs[name],
    addEventListener(type, handler) { node.__listeners[type] = handler },
  })
  Object.defineProperty(node, 'textContent', {
    get: () => textOf(node),
    set: (value) => { node.__text = String(value) },
  })
  // `dataset` and `style` are read-only accessors in a real DOM: assigning the
  // whole object throws, which once rendered an empty page while this stub
  // happily accepted it. Mirror the real behaviour so the suite catches it.
  Object.defineProperty(node, 'dataset', {
    get: () => data,
    set: () => { throw new TypeError('Cannot set property dataset of #<HTMLElement> which has only a getter') },
  })
  Object.defineProperty(node, 'style', {
    get: () => ({}),
    set: () => { throw new TypeError('Cannot set property style of #<HTMLElement> which has only a getter') },
  })
  return node
}

/** Install the stub globals and load the renderer; returns its handles. */
async function loadRenderer(api) {
  const main = element('main')
  const navButtons = ['home', 'projects', 'datasheets', 'symbols', 'run', 'settings'].map((page) => {
    const button = element('button')
    button.dataset.page = page
    return button
  })
  const ids = { main, mark: element('img'), navHint: element('div'), phase: element('div') }
  globalThis.document = {
    createElement: (tag) => element(tag),
    createTextNode: (value) => String(value),
    getElementById: (id) => ids[id] ?? element('div'),
    querySelectorAll: (selector) => (selector.includes('nav button') ? navButtons : []),
  }
  globalThis.Node = StubNode
  globalThis.window = { cicada: api }
  await import(`../src/launcher-ui.js?stub=${String(Date.now())}`)
  await settle()
  return { main, navButtons, text: () => textOf(main) }
}

const PROJECTS = {
  projects: [
    { workspaceId: 'w1', title: 'blink', path: 'C:\\proj\\blink', registered: true, exists: true, schematic: true, components: 10, nets: 5, sessions: 2, modifiedAt: 1789000000000 },
    { workspaceId: '', title: 'fresh', path: 'C:\\proj\\fresh', registered: false, exists: true, schematic: false, components: 0, nets: 0, sessions: 0, modifiedAt: 1789000000001 },
  ],
  problems: [],
  config: { projectsDir: 'C:\\proj', home: 'C:\\home' },
}

const DATASHEETS = {
  ok: true,
  entries: [{ part: 'NE555P', package: 'PDIP-8', groups: 8, pins: 8, haveShape: true, haveDetail: true, missingDetailGroups: [], audited: true, modifiedAt: 1789000000000, bytes: 85598 }],
  problems: [],
}

const DETAIL = {
  ok: true,
  detail: {
    part: 'NE555P', package: 'PDIP-8', audited: true, modifiedAt: 1789000000000, bytes: 85598,
    groups: [{ groupId: 'PIN-001', title: 'Pinout', category: 'pinout', priority: 'required', brief: '', detail: true, pins: [{ number: '1', name: 'GND', electrical: 'ground', claims: 1 }], claims: [{ id: 'c1', ref: 'full.md:15', fact: 'Pin 1 is GND' }] }],
    shape: [{ number: '1', name: 'GND', electrical: 'power_in', side: 'left' }],
    fullMd: '# NE555', fullMdBytes: 7, fullMdTruncated: false,
  },
}

const SYMBOLS = {
  ok: true,
  engine: true,
  problems: [],
  curated: [{ key: 'R:R', category: 'R', name: 'R', source: 'curated', file: 'C:\\libs\\R\\R.kicad_sym', bytes: 900, modifiedAt: 1789000000000, pins: 2, registered: true }],
  user: [{ key: 'IC:NE555P', category: 'IC', name: 'NE555P', source: 'user', file: 'C:\\user\\IC\\NE555P.kicad_sym', bytes: 700, modifiedAt: 1789000000001, pins: 8, registered: true }],
}

const SYMBOL_DETAIL = {
  ok: true,
  symbol: SYMBOLS.curated[0],
  pins: [{ number: '1', name: 'A', x: -254, y: 0, angle: 0 }, { number: '2', name: 'B', x: 254, y: 0, angle: 180 }],
  engineKey: 'R:R',
  engineProblem: '',
}

let stateHandler = () => {}

const api = {
  markSrc: '',
  listProjects: async () => PROJECTS,
  settings: async () => ({ projectsDir: 'C:\\proj', home: 'C:\\home', rows: [['CICADA_HOME', 'C:\\home', true, 'C:\\home']] }),
  datasheets: async () => DATASHEETS,
  datasheet: async () => DETAIL,
  symbols: async () => SYMBOLS,
  symbol: async () => SYMBOL_DETAIL,
  trashSymbol: async () => ({ ok: true, movedTo: 'C:\\home\\trash\\x' }),
  exportSymbol: async () => ({ ok: true, target: 'C:\\out\\R.kicad_sym' }),
  saveSymbolToUser: async () => ({ ok: true, target: 'C:\\home\\cicada-user-lib\\R\\R.kicad_sym' }),
  importSymbol: async () => ({ ok: true, key: 'IC:MyPart', target: 'C:\\home\\cicada-user-lib\\IC\\MyPart.kicad_sym' }),
  rebuildSymbol: async () => ({ ok: true, key: 'IC:NE555P', warnings: [] }),
  createProject: async () => ({ ok: true, path: 'C:\\proj\\x', created: true, problem: '' }),
  startStack: async () => ({ ok: true, problem: '' }),
  stopStack: async () => ({ ok: true }),
  openPicker: async () => ({ ok: false, canceled: true, problem: '' }),
  exportDatasheet: async () => ({ ok: true, target: 'C:\\out\\NE555P' }),
  trashDatasheet: async () => ({ ok: true, movedTo: 'C:\\home\\trash\\x' }),
  openTrash: () => {},
  openProduct: () => {},
  reveal: () => {},
  openLogs: () => {},
  onState: (handler) => { stateHandler = handler },
  clearConsole: () => {},
}

test('every launcher page renders against the live API surface', async () => {
  const ui = await loadRenderer(api)
  assert.match(ui.text(), /开始/)
  for (const page of ['projects', 'datasheets', 'run', 'settings', 'home']) {
    const button = ui.navButtons.find((candidate) => candidate.dataset.page === page)
    button.__listeners.click()
    await settle()
    assert.ok(ui.text().length > 0, `page ${page} rendered nothing`)
  }
  // The project cards and the datasheet table carry real values end to end.
  const home = ui.navButtons.find((candidate) => candidate.dataset.page === 'home')
  home.__listeners.click()
  assert.match(ui.text(), /blink/)
  const datasheets = ui.navButtons.find((candidate) => candidate.dataset.page === 'datasheets')
  datasheets.__listeners.click()
  await settle()
  assert.match(ui.text(), /NE555P/)
  assert.match(ui.text(), /审计标记/)
  // Symbols: both libraries, engine pin counts, and the action set.
  const symbols = ui.navButtons.find((candidate) => candidate.dataset.page === 'symbols')
  symbols.__listeners.click()
  await settle()
  assert.match(ui.text(), /精选库（只读）/)
  assert.match(ui.text(), /用户库（可写）/)
  assert.match(ui.text(), /R:R/)
  assert.match(ui.text(), /IC:NE555P/)
  assert.match(ui.text(), /另存到用户库/)
  assert.match(ui.text(), /导入 .kicad_sym 到类别/)
})

test('the run page streams the console and reflects live state', async () => {
  const ui = await loadRenderer(api)
  const run = ui.navButtons.find((candidate) => candidate.dataset.page === 'run')
  // State arrives from the main process before the page is opened.
  stateHandler({
    phase: 'ready',
    message: '已就绪：产品窗口已打开',
    enginePort: 3123,
    hasWindow: true,
    project: 'C:\\proj\\blink',
    console: ['—— 启动 C:\\proj\\blink ——', 'cicada: 引擎端口 3123', 'dsh web: http://127.0.0.1:3123/?token=***'],
    stamp: '2026-09-13 00:20',
    problems: [],
    logTail: [],
  })
  run.__listeners.click()
  await settle()
  const text = ui.text()
  assert.match(text, /实时控制台/)
  assert.match(text, /cicada: 引擎端口 3123/)
  assert.match(text, /token=\*\*\*/, 'the console must show the redacted URL')
  assert.match(text, /C:\\proj\\blink/)
  assert.match(text, /已就绪/)
  assert.match(text, /2026-09-13 00:20/, 'the window must show which build it is running')
  assert.match(text, /2026-09-13 00:20/, 'the window must show which build it is running')
})

test('the datasheet page survives an empty library and a manager failure', async () => {
  const empty = await loadRenderer({ ...api, datasheets: async () => ({ ok: false, entries: [], problems: ['管理服务启动超时（无 announce 行）'] }) })
  const button = empty.navButtons.find((candidate) => candidate.dataset.page === 'datasheets')
  button.__listeners.click()
  await settle()
  assert.match(empty.text(), /管理服务启动超时/)
})
