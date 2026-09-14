/**
 * Launcher window renderer (docs/04 §5.1). Plain DOM, no framework: the window
 * is a projection of `window.cicada` (preload) — every process action goes back
 * through the main process, which owns the launcher child and the registry
 * reads. The brand mark image is fetched through the same preload channel so the
 * page needs no network or file access of its own.
 */

const api = window.cicada
const $ = (tag, props = {}, ...children) => {
  const element = document.createElement(tag)
  for (const [key, value] of Object.entries(props)) {
    // `dataset` and `style` are read-only accessors on real DOM nodes: assigning
    // the whole object throws (it silently rendered an empty page once), so the
    // two structured properties are written field by field.
    if (key === 'dataset') for (const [k, v] of Object.entries(value)) element.dataset[k] = String(v)
    else if (key === 'style') Object.assign(element.style, value)
    else element[key] = value
  }
  for (const child of children.flat()) {
    if (child === undefined || child === null) continue
    element.append(child instanceof Node ? child : document.createTextNode(String(child)))
  }
  return element
}
const PHASES = { starting: '启动中', ready: '已就绪', failed: '启动失败', blocked: '已有实例', exited: '已退出', idle: '未启动' }

let state = { phase: 'idle', message: '未启动', enginePort: null, hasWindow: false, problems: [], logTail: [] }
let projects = []
let problems = []
let selected = ''
let page = 'home'
let settings = { rows: [] }

/** Derive one project card. */
function card(project, options = {}) {
  const isSelected = project.path === selected
  const summary = project.schematic
    ? `${project.components} 元件 / ${project.nets} 网络`
    : '空项目'
  const when = project.modifiedAt > 0 ? new Date(project.modifiedAt).toLocaleString() : '—'
  const tags = [
    project.registered
      ? $('span', { className: 'tag ok' }, '已注册')
      : $('span', { className: 'tag warn' }, '未注册'),
    project.exists ? undefined : $('span', { className: 'tag bad' }, '目录不存在'),
    options.badge,
  ]
  return $('div', { className: `card${isSelected ? ' sel' : ''}` },
    $('h3', {}, project.title, ...tags),
    $('div', { className: 'path' }, project.path),
    $('div', { className: 'meta' }, `${summary}　·　${project.sessions} 会话　·　${when}`),
    $('div', { className: 'acts' },
      $('button', { className: 'act small primary', onclick: () => start(project.path), disabled: !project.exists }, '启动'),
      $('button', { className: 'act small', onclick: () => api.reveal(project.path) }, '打开目录'),
    ),
  )
}

function projectList() {
  const list = $('div', { className: 'cards' })
  for (const project of projects) list.append(card(project))
  return list
}

function emptyProjects() {
  return $('div', { className: 'empty' },
    '还没有项目。用「新建项目」在 ',
    $('span', { className: 'mono' }, settings.projectsDir ?? '<home>/projects'),
    ' 下建一个，或「打开目录」把已有目录注册进来。')
}

/** A simple key/value table (settings, run facts). */
function table(rows) {
  const body = $('tbody')
  for (const row of rows) {
    body.append($('tr', {},
      $('th', {}, row[0]),
      $('td', { className: row[2] === true ? 'mono' : '' }, row[1] ?? '—'),
      row[3] === undefined ? $('td') : $('td', {},
        $('button', { className: 'act small', onclick: () => api.reveal(row[3]) }, '打开')),
    ))
  }
  return $('table', {}, $('thead', {}, $('tr', {}, $('th', {}, '项'), $('th', {}, '值'), $('th', {}, ''))), body)
}

/** One page body. */
function renderPage() {
  const main = document.getElementById('main')
  main.replaceChildren()
  const head = (title, sub) => [ $('h2', {}, title), $('div', { className: 'sub' }, sub) ]

  if (page === 'home') {
    main.append(...head('开始', '选一个项目，点启动；产品窗口关闭后回到这里。'))
    main.append($('div', { className: 'row' },
      $('button', { className: 'act primary', onclick: () => start(startTarget()), disabled: projects.length === 0 }, '启动'),
      $('button', { className: 'act', onclick: () => go('projects') }, '管理项目'),
      $('button', { className: 'act', onclick: () => go('run') }, '运行状态'),
    ))
    main.append(projects.length > 0 ? projectList() : emptyProjects())
    return
  }

  if (page === 'projects') {
    main.append(...head('项目', `默认根目录 ${settings.projectsDir ?? '—'}；列表同时显示 DSH 注册表里的工作区。`))
    const nameInput = $('input', { type: 'text', placeholder: '新项目名，例如 ne555-blink' })
    const message = $('div', { className: 'msg' })
    main.append($('div', { className: 'row' },
      $('button', { className: 'act', onclick: () => void refresh() }, '刷新'),
      $('button', {
        className: 'act',
        onclick: async () => {
          // Picking an existing directory = "open this world": start it. DSH
          // registers the path on first use, so no registry write happens here.
          const picked = await api.openPicker()
          if (picked.ok) await start(picked.path)
        },
      }, '打开已有目录…'),
    ))
    main.append($('div', { className: 'field' },
      $('label', {}, '新建项目'),
      $('div', { className: 'row' }, nameInput,
        $('button', {
          className: 'act primary',
          onclick: async () => {
            const result = await api.createProject(nameInput.value)
            message.className = `msg ${result.ok ? 'good' : 'bad'}`
            message.textContent = result.ok
              ? `${result.created ? '已创建' : '已存在'}：${result.path}（启动后会自动注册进工作区列表）`
              : result.problem
            if (result.ok) { nameInput.value = ''; selected = result.path; await refresh() }
          },
        }, '创建'),
      ),
    ))
    main.append(message)
    main.append(projects.length > 0 ? projectList() : emptyProjects())
    return
  }

  if (page === 'run') {
    main.append(...head('运行', '实时控制台：启动器/引擎/主机 的输出按行流入，token 已脱敏。'))
    const follow = $('input', { type: 'checkbox', checked: true })
    runRefs = {
      badge: $('span', { className: 'badge', dataset: { phase: state.phase } }, PHASES[state.phase] ?? state.phase),
      port: $('b', {}, state.enginePort === null ? '—' : String(state.enginePort)),
      window: $('b', {}, state.hasWindow ? '已打开' : '未打开'),
      project: $('b', {}, state.project === '' ? '（未选项目）' : state.project),
      message: $('div', { className: 'msg' }, state.message),
      console: $('pre', { className: 'console' }),
    }
    main.append($('div', { className: 'facts' },
      '状态：', runRefs.badge, '　引擎端口：', runRefs.port,
      '　产品窗口：', runRefs.window, '　项目：', runRefs.project,
      '　界面版本：', $('b', {}, state.stamp ?? '—')))
    main.append($('div', { className: 'row' },
      $('button', { className: 'act', onclick: () => api.openProduct(), disabled: !state.hasWindow }, '打开产品窗口'),
      $('button', { className: 'act', onclick: () => api.stopStack() }, '停止栈'),
      $('button', { className: 'act', onclick: () => api.openLogs() }, '打开日志目录'),
      $('button', { className: 'act', onclick: () => { api.clearConsole(); } }, '清空控制台'),
      $('label', { className: 'inline' }, follow, '自动滚动'),
    ))
    main.append(runRefs.message)
    main.append(runRefs.console)
    follow.addEventListener('change', () => { follow.checked = follow.checked })
    runFollow = () => follow.checked
    paintRun()
    return
  }

  if (page === 'datasheets') {
    main.append(...head('资料库', '共享数据手册库（单一权威 = cicada-knowledge 读取器）。删除只移入回收站。'))
    const detailHost = $('div')
    const query = $('input', { type: 'text', placeholder: '按型号过滤…' })
    const listHost = $('div')
    const message = $('div', { className: 'msg' })
    if (!datasheetsLoaded || datasheets.length === 0) {
      main.append($('div', { className: 'row' },
        $('button', { className: 'act', onclick: () => void refreshDatasheets() }, '刷新'),
        $('button', { className: 'act', onclick: () => api.openTrash() }, '打开回收站'),
      ))
      main.append(message)
      main.append($('div', { className: 'empty' },
        !datasheetsLoaded
          ? '正在读取资料库…'
          : (datasheetProblems.length > 0 ? datasheetProblems.join('；') : '库还是空的（还没有发布过数据手册条目）。')))
      return
    }
    const paint = () => {
      listHost.replaceChildren(datasheetTable(query.value.trim()))
      renderDatasheetDetail(detailHost)
    }
    query.addEventListener('input', paint)
    main.append($('div', { className: 'row' },
      query,
      $('button', { className: 'act', onclick: () => void refreshDatasheets() }, '刷新'),
      $('button', { className: 'act', onclick: () => api.openTrash() }, '打开回收站'),
    ))
    main.append(message)
    main.append(listHost)
    main.append(detailHost)
    paint()
    return
  }

  if (page === 'symbols') {
    main.append(...head('符号库', '精选库（只读）与用户库（可写）；引脚几何来自引擎，删除只进回收站。'))
    const query = $('input', { type: 'text', placeholder: '按键 / 名称过滤…' })
    const listHost = $('div')
    const detailHost = $('div')
    const message = $('div', { className: 'msg' })
    const category = $('input', { type: 'text', value: 'IC', placeholder: '类别' })
    main.append($('div', { className: 'row' },
      query,
      $('button', { className: 'act', onclick: () => void refreshSymbols() }, '刷新'),
      $('button', { className: 'act', onclick: () => api.openTrash() }, '打开回收站'),
      $('span', { className: 'sep' }, '导入 .kicad_sym 到类别'),
      category,
      $('button', {
        className: 'act',
        onclick: async () => {
          const result = await api.importSymbol(category.value.trim())
          reportSymbol(result, result.ok ? `已导入 ${result.key} → ${result.target}` : '')
          if (result.ok) await refreshSymbols()
        },
      }, '导入…'),
    ))
    main.append(message)
    main.append(listHost)
    main.append(detailHost)
    const paint = () => {
      listHost.replaceChildren(symbolTables(query.value.trim()))
      renderSymbolDetail(detailHost)
    }
    query.addEventListener('input', paint)
    paint()
    return
  }

  if (page === 'settings') {
    main.append(...head('设置', '只读展示当前解析出的路径与环境；改路径用环境变量覆盖。'))
    main.append(table(settings.rows))
    return
  }
}

/** The project the big "启动" button targets: the selection, else the newest. */
function startTarget() {
  if (selected !== '') return selected
  return projects[0]?.path ?? ''
}

// ── run console (docs/04 §5.1) ─────────────────────────────────────────────

/** Nodes of the mounted run page (null while another page is shown). */
let runRefs = null
/** Whether the console should stick to the newest line. */
let runFollow = () => true

/** Repaint the mounted run page from `state` without rebuilding the DOM. */
function paintRun() {
  if (runRefs === null || page !== 'run') return
  const next = PHASES[state.phase] ?? state.phase
  if (runRefs.badge.textContent !== next) runRefs.badge.textContent = next
  runRefs.badge.dataset.phase = state.phase
  runRefs.port.textContent = state.enginePort === null ? '—' : String(state.enginePort)
  runRefs.window.textContent = state.hasWindow ? '已打开' : '未打开'
  runRefs.project.textContent = state.project === '' ? '（未选项目）' : state.project
  runRefs.message.textContent = state.message
  const text = (state.console ?? []).join('\n')
  const consoleNode = runRefs.console
  const atBottom = consoleNode.scrollHeight - consoleNode.scrollTop - consoleNode.clientHeight < 24
  if (consoleNode.textContent !== text) consoleNode.textContent = text
  if (runFollow() && (atBottom || consoleNode.scrollTop === 0)) consoleNode.scrollTop = consoleNode.scrollHeight
}

// ── datasheet library (docs/04 §5.3) ───────────────────────────────────────

let datasheets = []
let datasheetProblems = []
let datasheetsLoaded = false
let datasheet = null

const bytes = (value) => (value > 1024 * 1024
  ? `${(value / 1024 / 1024).toFixed(1)} MB`
  : `${Math.max(1, Math.round(value / 1024))} KB`)
const stamp = (value) => (value > 0 ? new Date(value).toLocaleString() : '—')

/** Library table; clicking a row opens the detail below it. */
function datasheetTable(filter) {
  const rows = datasheets.filter((entry) => filter === '' || entry.part.toLowerCase().includes(filter.toLowerCase()))
  if (rows.length === 0) return $('div', { className: 'empty' }, '没有匹配的型号。')
  const body = $('tbody')
  for (const entry of rows) {
    const gaps = entry.missingDetailGroups.length
    body.append($('tr', { className: 'clickable' },
      $('td', {}, $('b', {}, entry.part)),
      $('td', {}, entry.package || '—'),
      $('td', {}, String(entry.groups)),
      $('td', {}, String(entry.pins)),
      $('td', {}, entry.haveShape ? $('span', { className: 'tag ok' }, '有') : $('span', { className: 'tag warn' }, '缺')),
      $('td', {}, gaps === 0
        ? $('span', { className: 'tag ok' }, '齐')
        : $('span', { className: 'tag bad' }, `缺 ${gaps}`)),
      $('td', {}, entry.audited
        ? $('span', { className: 'tag ok', title: '发布写入时记录的审计标记；契约版本见详情页（v3 起锚点与 description 由审计强制）' }, '审计标记')
        : $('span', { className: 'tag' }, '未审计')),
      $('td', {}, stamp(entry.modifiedAt)),
      $('td', {}, bytes(entry.bytes)),
      $('td', {},
        $('button', { className: 'act small', onclick: (event) => { event.stopPropagation(); void openDatasheet(entry.part) } }, '查看'),
        $('button', {
          className: 'act small',
          onclick: async (event) => {
            event.stopPropagation()
            const result = await api.exportDatasheet(entry.part)
            reportDatasheet(result, result.ok ? `已导出到 ${result.target}` : '')
          },
        }, '导出'),
        $('button', {
          className: 'act small',
          onclick: async (event) => {
            event.stopPropagation()
            const result = await api.trashDatasheet(entry.part)
            if (result.ok) datasheet = null
            reportDatasheet(result, result.ok ? `已移入回收站：${result.movedTo}` : '')
            if (result.ok) await refreshDatasheets()
          },
        }, '删除'),
      ),
    ))
  }
  return table0(['型号', '封装', '组', '引脚', 'shape', 'detail', '审计', '更新时间', '体积', ''], body)
}

/** Generic table wrapper (body supplied). */
function table0(headers, body) {
  const head = $('tr')
  for (const label of headers) head.append($('th', {}, label))
  return $('table', { className: 'grid' }, $('thead', {}, head), body)
}

/** Show the transient message line of the datasheet page. */
function reportDatasheet(result, okText) {
  const message = document.querySelector('#main .msg')
  if (message === null) return
  const problem = result.problem ?? ''
  const canceled = result.canceled === true
  message.className = `msg ${result.ok ? 'good' : canceled ? 'note' : 'bad'}`
  message.textContent = result.ok ? okText : (canceled ? '已取消' : problem)
}

async function refreshDatasheets() {
  const listed = await api.datasheets()
  datasheetsLoaded = true
  datasheets = listed.entries ?? []
  datasheetProblems = listed.problems ?? []
  if (page === 'datasheets') renderPage()
}

async function openDatasheet(part) {
  const result = await api.datasheet(part)
  if (!result.ok) {
    reportDatasheet(result, '')
    return
  }
  datasheet = result.detail
  renderPage()
}

/** Detail panel: groups → pins, claims, shape pins, full.md preview. */
function renderDatasheetDetail(host) {
  host.replaceChildren()
  if (datasheet === null) return
  const entry = datasheet
  host.append($('h3', { className: 'detail-title' },
    `${entry.part}${entry.package === '' ? '' : ` · ${entry.package}`}`, ' ',
    entry.audited ? $('span', { className: 'tag ok' }, '已审计') : $('span', { className: 'tag' }, '未审计'),
    ' ', $('span', { className: 'tag' }, `${bytes(entry.bytes)}`)))
  host.append($('div', { className: 'row' },
    $('button', {
      className: 'act small',
      disabled: entry.shape.length === 0,
      onclick: async () => {
        const result = await api.rebuildSymbol(entry.part)
        reportDatasheet(result, result.ok
          ? `已按 shape 重建符号 ${result.key}${(result.warnings ?? []).length > 0 ? `（${result.warnings.join('；')}）` : ''}`
          : '')
      },
    }, '按 shape 生成 / 重建符号'),
    $('span', { className: 'sub' }, entry.shape.length === 0
      ? '该件没有 shape.json，无法生成符号'
      : '写入用户库（走引擎 /lib/synthesize），随后可在符号库页看到'),
  ))
  for (const group of entry.groups) {
    const rows = $('tbody')
    for (const pin of group.pins) {
      rows.append($('tr', {},
        $('td', {}, pin.number),
        $('td', {}, pin.name || '—'),
        $('td', {}, pin.electrical || '—'),
        $('td', {}, (pin.functions || []).join('、') || '—'),
        $('td', {}, pin.claims > 0 ? String(pin.claims) : '—'),
      ))
    }
    const claimList = $('ul', { className: 'claims' })
    for (const claim of group.claims) {
      claimList.append($('li', {}, $('span', { className: 'mono' }, claim.ref || claim.id), ' ', claim.fact))
    }
    host.append($('details', { className: 'group' },
      $('summary', {},
        `${group.groupId} · ${group.title || '（无标题）'}`,
        group.category === '' ? '' : ` · ${group.category}`,
        group.pins.length > 0 ? ` · ${group.pins.length} 引脚` : ' · 纯文字组',
        group.detail ? '' : ' · 缺 detail'),
      group.brief === '' ? '' : $('div', { className: 'sub' }, group.brief),
      group.description === '' ? '' : $('div', { className: 'sub' }, group.description),
      group.location === '' ? '' : $('div', { className: 'sub mono' }, `锚点 full.md:${group.location}`),
      (group.externalComponents || []).length === 0 ? '' : table0(
        ['外接元件', '取值', '连接方式', '为什么'],
        (() => {
          const rows = $('tbody')
          for (const item of group.externalComponents) {
            rows.append($('tr', {},
              $('td', {}, item.kind || '—'),
              $('td', {}, item.value || '—'),
              $('td', {}, item.connection || '—'),
              $('td', {}, item.why || '—'),
            ))
          }
          return rows
        })(),
      ),
      (group.operatingLimits || []).length === 0 ? '' : table0(
        ['工作限值', '取值', '条件'],
        (() => {
          const rows = $('tbody')
          for (const item of group.operatingLimits) {
            rows.append($('tr', {}, $('td', {}, item.name || '—'), $('td', {}, item.value || '—'), $('td', {}, item.condition || '—')))
          }
          return rows
        })(),
      ),
      group.pins.length > 0
        ? table0(['物理号', '规范名', '电气类型', '功能', 'claims'], rows)
        : '',
      group.claims.length > 0 ? claimList : '',
    ))
  }
  if (entry.shape.length > 0) {
    const rows = $('tbody')
    for (const pin of entry.shape) {
      rows.append($('tr', {}, $('td', {}, pin.number), $('td', {}, pin.name || '—'),
        $('td', {}, pin.electrical || '—'), $('td', {}, pin.side || '—')))
    }
    host.append($('details', { className: 'group' },
      $('summary', {}, `shape.json · ${entry.shape.length} 引脚（引擎合成符号的输入）`),
      table0(['物理号', '名称', '电气类型', '边'], rows)))
  }
  host.append($('details', { className: 'group' },
    $('summary', {}, `full.md 预览 · ${bytes(entry.fullMdBytes)}${entry.fullMdTruncated ? '（已截断）' : ''}`),
    $('pre', {}, entry.fullMd || '（空）')))
}

// ── symbol library (docs/04 §5.4) ──────────────────────────────────────────

let symbols = { curated: [], user: [], problems: [], engine: false }
let symbolsLoaded = false
let symbolsLoading = false
let symbolDetail = null

/** Both library tables (curated first, then the writable user library). */
function symbolTables(filter) {
  const host = $('div')
  const curated = symbols.curated.filter((entry) => matches(entry, filter))
  const user = symbols.user.filter((entry) => matches(entry, filter))
  host.append($('h3', { className: 'detail-title' }, `精选库（只读）· ${curated.length} 个符号`))
  host.append(curated.length === 0 ? $('div', { className: 'empty' }, '没有匹配的符号。') : symbolTable(curated, 'curated'))
  host.append($('h3', { className: 'detail-title' }, `用户库（可写）· ${user.length} 个符号`))
  host.append(user.length === 0
    ? $('div', { className: 'empty' }, symbols.engine ? '用户库还是空的（AI 合成 IC 或从精选库另存会写到这里）。' : '引擎未就绪，暂不能读取用户库。')
    : symbolTable(user, 'user'))
  return host
}

const matches = (entry, filter) => filter === '' || entry.key.toLowerCase().includes(filter.toLowerCase())

/** One library table. */
function symbolTable(entries, source) {
  const body = $('tbody')
  for (const entry of entries) {
    const actions = [
      $('button', { className: 'act small', onclick: (event) => { event.stopPropagation(); void openSymbol(source, entry.key) } }, '查看'),
      $('button', {
        className: 'act small',
        onclick: async (event) => {
          event.stopPropagation()
          const result = await api.exportSymbol(source, entry.key)
          reportSymbol(result, result.ok ? `已导出到 ${result.target}` : '')
        },
      }, '导出'),
    ]
    if (source === 'curated') {
      actions.push($('button', {
        className: 'act small',
        onclick: async (event) => {
          event.stopPropagation()
          const result = await api.saveSymbolToUser(entry.key)
          reportSymbol(result, result.ok ? `已另存到用户库：${result.target}` : '')
          if (result.ok) await refreshSymbols()
        },
      }, '另存到用户库'))
    } else {
      actions.push($('button', {
        className: 'act small',
        onclick: async (event) => {
          event.stopPropagation()
          const result = await api.trashSymbol(entry.key)
          if (result.ok) symbolDetail = null
          reportSymbol(result, result.ok ? `已移入回收站：${result.movedTo}` : '')
          if (result.ok) await refreshSymbols()
        },
      }, '删除'))
    }
    body.append($('tr', { className: 'clickable' },
      $('td', {}, $('b', {}, entry.key)),
      $('td', {}, String(entry.pins ?? '—')),
      $('td', {}, entry.registered ? $('span', { className: 'tag ok' }, '引擎已载入') : $('span', { className: 'tag warn' }, '引擎未列出')),
      $('td', {}, stamp(entry.modifiedAt)),
      $('td', {}, bytes(entry.bytes)),
      $('td', {}, ...actions),
    ))
  }
  return table0(['键', '引脚', '状态', '修改时间', '体积', ''], body)
}

/** Show the transient message line of the symbols page. */
function reportSymbol(result, okText) {
  const message = document.querySelector('#main .msg')
  if (message === null) return
  message.className = `msg ${result.ok ? 'good' : result.canceled === true ? 'note' : 'bad'}`
  message.textContent = result.ok ? okText : (result.canceled === true ? '已取消' : (result.problem ?? ''))
}

async function refreshSymbols() {
  symbolsLoading = true
  const listed = await api.symbols()
  symbolsLoading = false
  symbolsLoaded = true
  symbols = { curated: listed.curated ?? [], user: listed.user ?? [], problems: listed.problems ?? [], engine: listed.engine === true }
  if (page === 'symbols') renderPage()
}

async function openSymbol(source, key) {
  const result = await api.symbol(source, key)
  if (!result.ok) {
    reportSymbol(result, '')
    return
  }
  symbolDetail = result.detail
  renderPage()
}

/** Detail panel: pin geometry straight from the engine. */
function renderSymbolDetail(host) {
  host.replaceChildren()
  if (symbolDetail === null) return
  const detail = symbolDetail
  host.append($('h3', { className: 'detail-title' },
    detail.symbol.key, ' ',
    detail.symbol.registered ? $('span', { className: 'tag ok' }, '引擎已载入') : $('span', { className: 'tag warn' }, '引擎未列出'),
    ' ', $('span', { className: 'tag' }, `${detail.symbol.pins} 引脚`),
    ' ', $('span', { className: 'tag' }, bytes(detail.symbol.bytes))))
  host.append($('div', { className: 'sub mono' }, detail.symbol.file))
  if (detail.pins.length === 0) {
    host.append($('div', { className: 'empty' },
      detail.engineProblem === '' ? '引擎没有返回该符号的引脚。' : `引擎读取失败：${detail.engineProblem}`))
    return
  }
  const rows = $('tbody')
  for (const pin of detail.pins) {
    rows.append($('tr', {},
      $('td', {}, pin.number),
      $('td', {}, pin.name || '—'),
      $('td', {}, `${(pin.x / 100).toFixed(2)}, ${(pin.y / 100).toFixed(2)} mm`),
      $('td', {}, `${pin.angle}°`),
    ))
  }
  host.append(table0(['物理号', '名称', '位置（局部）', '朝向'], rows))
}

/** Switch page and re-render. */
function go(next) {
  page = next
  runRefs = null
  for (const button of document.querySelectorAll('nav button[data-page]')) {
    button.setAttribute('aria-current', String(button.dataset.page === page))
  }
  renderPage()
  if (page === 'settings' || page === 'home') void refreshSettings()
  if (page === 'datasheets' && !datasheetsLoaded) void refreshDatasheets()
  if (page === 'symbols' && !symbolsLoaded) void refreshSymbols()
}

/** Refresh project list + settings + state from the main process. */
async function refresh() {
  const listed = await api.listProjects()
  projects = listed.projects
  problems = listed.problems
  settings = { ...settings, ...listed.config }
  if (selected === '') selected = projects[0]?.path ?? ''
  renderPage()
  if (problems.length > 0) {
    const message = $('div', { className: 'msg bad' }, problems.join('；'))
    document.getElementById('main').prepend(message)
  }
}

async function refreshSettings() {
  const view = await api.settings()
  settings = { ...settings, ...view }
  if (page === 'settings' || page === 'home') renderPage()
}

/** Start the stack for one project (or report why not). */
async function start(path) {
  if (path === '') return
  selected = path
  const result = await api.startStack(path)
  if (!result.ok) {
    const message = $('div', { className: 'msg bad' }, result.problem)
    document.getElementById('main').prepend(message)
    return
  }
  go('run')
}

document.getElementById('mark').src = api.markSrc
for (const button of document.querySelectorAll('nav button[data-page]')) {
  button.addEventListener('click', () => { if (!button.disabled) go(button.dataset.page) })
}
api.onState((next) => {
  state = next
  document.getElementById('navHint').textContent = PHASES[next.phase] ?? next.phase
  // The console repaints in place: rebuilding the page would fight the user's
  // scroll position on every streamed line.
  if (page === 'run') paintRun()
})
void refreshSettings().then(refresh)
