// M1c interactive verification v2: drives the headless Edge (CDP port 9333)
// with real input events and validates each step against engine truth fetched
// from inside the page (/cicada/editor/state → engine /scene).
// Run: node ../cicada-editor/scripts/m1c-drive.mjs   (WSL node)
import fs from 'node:fs'

const PORT = 9333
const OUT_DIR = '/mnt/c/dsh/Schematic-Cicada/cicada-editor/build-msvc-kicad'
const WEB = fs.readFileSync(`${OUT_DIR}/web-url.txt`, 'utf8').trim()
const sleep = (ms) => new Promise((r) => setTimeout(r, ms))

let targets = []
for (let i = 0; i < 40; i += 1) {
  try {
    targets = await (await fetch(`http://127.0.0.1:${PORT}/json`)).json()
    if (targets.some((t) => (t.type ?? '') === 'page')) break
  } catch {}
  await sleep(500)
}
const page = targets.find((t) => (t.type ?? '') === 'page')
if (page === undefined) throw new Error('no page target')

let msgId = 0
const pending = new Map()
const ws = new WebSocket(page.webSocketDebuggerUrl)
await new Promise((resolve, reject) => { ws.onopen = resolve; ws.onerror = () => reject(new Error('ws error')) })
ws.onmessage = (event) => {
  const m = JSON.parse(String(event.data))
  if (m.id !== undefined && pending.has(m.id)) {
    const { resolve, reject } = pending.get(m.id)
    pending.delete(m.id)
    if (m.error !== undefined) reject(new Error(m.error.message))
    else resolve(m.result)
  }
}
const send = (method, params = {}) =>
  new Promise((resolve, reject) => {
    const id = ++msgId
    pending.set(id, { resolve, reject })
    ws.send(JSON.stringify({ id, method, params }))
  })
const evalJs = async (expression, awaitPromise = false) => {
  const r = await send('Runtime.evaluate', { expression, returnByValue: true, awaitPromise })
  if (r.exceptionDetails !== undefined) throw new Error(`eval: ${r.exceptionDetails.text ?? 'err'}`)
  return r.result?.value
}
const shot = async (name) => {
  const s = await send('Page.captureScreenshot', { format: 'png' })
  fs.writeFileSync(`${OUT_DIR}/${name}.png`, Buffer.from(s.data, 'base64'))
  console.log(`SHOT ${name}`)
}
const click = async (x, y) => {
  await send('Input.dispatchMouseEvent', { type: 'mousePressed', x, y, button: 'left', clickCount: 1 })
  await send('Input.dispatchMouseEvent', { type: 'mouseReleased', x, y, button: 'left', clickCount: 1 })
  await sleep(350)
}
const press = async (x, y) => {
  await send('Input.dispatchMouseEvent', { type: 'mousePressed', x, y, button: 'left', clickCount: 1 })
  await sleep(80)
}
const move = async (x, y, buttons = 1) => {
  await send('Input.dispatchMouseEvent', { type: 'mouseMoved', x, y, buttons })
  await sleep(60)
}
const release = async (x, y) => {
  await send('Input.dispatchMouseEvent', { type: 'mouseReleased', x, y, button: 'left', clickCount: 1 })
  await sleep(400)
}
const key = async (k, code, vk) => {
  await send('Input.dispatchKeyEvent', { type: 'rawKeyDown', key: k, code, windowsVirtualKeyCode: vk })
  await send('Input.dispatchKeyEvent', { type: 'keyUp', key: k, code, windowsVirtualKeyCode: vk })
  await sleep(350)
}
const buttonRect = async (label) => {
  const r = await evalJs(`(() => {
    const b = [...document.querySelectorAll('[data-cicada-canvas-pane] button')]
      .find(x => x.textContent.trim() === ${JSON.stringify(label)})
    if (!b) return null
    const q = b.getBoundingClientRect()
    return { x: Math.round(q.left + q.width / 2), y: Math.round(q.top + q.height / 2) }
  })()`)
  if (r === null) throw new Error(`button not found: ${label}`)
  return r
}
const logUi = async (tag) => {
  const active = await evalJs(`document.querySelector('[data-cicada-canvas-pane] button[data-active]')?.textContent.trim() ?? '(none)'`)
  const status = await evalJs(`[...document.querySelectorAll('[data-cicada-canvas-pane] span')].map(s => s.textContent).join(' | ')`)
  console.log(`[${tag}] activeTool=${active}  spans=[${status}]`)
}
/** Engine truth from inside the page (session-authenticated /state → engine /scene). */
const engineScene = async () => {
  const st = await evalJs(`fetch('/cicada/editor/state').then(r => r.json())`, true)
  if (st?.enginePort === undefined) return { state: st, scene: null }
  const sc = await evalJs(
    `fetch('http://127.0.0.1:${st.enginePort}/scene', { headers: { 'X-Cicada-Token': ${JSON.stringify(st.engineToken)} } }).then(r => r.json())`, true)
  return { state: st, scene: sc }
}
const svgRect = () => evalJs(`(() => { const s = document.querySelector('[data-cicada-canvas-pane] svg'); if (!s) return null; const q = s.getBoundingClientRect(); return { left: q.left, top: q.top, width: q.width, height: q.height } })()`)
/** Rendered px center of the last component (viewport transform + scene truth). */
const lastPlacedPx = async () => {
  const t = await engineScene()
  const comps = t.scene?.components ?? []
  const last = comps[comps.length - 1]
  const vp = await evalJs(`(() => {
    const g = document.querySelector('[data-cicada-canvas-pane] svg > g')
    const m = /translate\\(([-\\d.]+) ([-\\d.]+)\\) scale\\(([-\\d.]+)\\)/.exec(g.getAttribute('transform'))
    const q = document.querySelector('[data-cicada-canvas-pane] svg').getBoundingClientRect()
    return { ox: +m[1], oy: +m[2], s: +m[3], left: q.left, top: q.top }
  })()`)
  return { x: Math.round(vp.left + vp.ox + last.x * vp.s), y: Math.round(vp.top + vp.oy + last.y * vp.s), refdes: last.refdes }
}

await send('Page.enable')
await send('Runtime.enable')
await send('Emulation.setDeviceMetricsOverride', { width: 1500, height: 950, deviceScaleFactor: 1, mobile: false })
await send('Page.navigate', { url: `${WEB}&layout=workspace` })
await sleep(9000)
console.log('canvas-pane:', await evalJs(`!!document.querySelector('[data-cicada-canvas-pane]')`))
const svg = await svgRect()
if (svg === null) throw new Error('svg not found')
const cx = svg.left + svg.width * 0.45
const cy = svg.top + svg.height * 0.45
const p2 = { x: svg.left + svg.width * 0.68, y: svg.top + svg.height * 0.55 }
const p3 = { x: svg.left + svg.width * 0.72, y: svg.top + svg.height * 0.58 }
const pPlace = { x: svg.left + svg.width * 0.5, y: svg.top + svg.height * 0.72 }

// ── 0) baseline ────────────────────────────────────────────────────────────
let t = await engineScene()
console.log('baseline comps:', t.scene?.components.map((c) => c.refdes), 'wires:', t.scene?.wires.length)

// ── 1) wire draw (v3, wx/KiCad semantics): free cursor + two-segment 45°
//      preview; click freezes a pair; Space finishes ─────────────────────────
await click(...Object.values(await buttonRect('画线')))
await logUi('after-wire-btn')
await click(cx, cy)                       // chain start A
await move(p2.x + 40, p2.y + 30, 0)       // free diagonal move → preview
await shot('v3-wire-preview')             // grid + A→mid→end preview
await click(p2.x, p2.y)                   // freeze pair 1
await move(p3.x, p3.y, 0)
await click(p3.x, p3.y)                   // freeze pair 2
await key(' ', 'Space', 32)
await logUi('after-wire-commit')
await shot('v3-wire')
t = await engineScene()
console.log('after wire: wires=', t.scene?.wires.length)

// ── 2) place by click (place tool → canvas click) ──────────────────────────
await click(...Object.values(await buttonRect('放置')))
await logUi('after-place-btn')
await click(pPlace.x, pPlace.y)
await logUi('after-place-click')
await shot('v2-place')
t = await engineScene()
console.log('after place: comps=', t.scene?.components.map((c) => c.refdes))

// ── 3) drag move the symbol just placed (center ≈ pPlace) ───────────────────
await click(...Object.values(await buttonRect('选择')))
await logUi('after-select-btn')
const tgt = await lastPlacedPx()
console.log('move target:', JSON.stringify(tgt))
await move(tgt.x, tgt.y, 0) // ensure the pointer is over the symbol before pressing
await press(tgt.x, tgt.y)
await move(tgt.x + 60, tgt.y + 45)
await move(tgt.x + 110, tgt.y + 70)
await release(tgt.x + 110, tgt.y + 70)
await logUi('after-move')
await shot('v2-move')
t = await engineScene()
console.log('after move: comps=', t.scene?.components.map((c) => [c.refdes, c.x, c.y]))

// ── 4) splitter drag using the real handle ─────────────────────────────────
const handle = await evalJs(`(() => { const h = document.querySelector('[data-cicada-splitter]'); if (!h) return null; const q = h.getBoundingClientRect(); return { x: Math.round(q.left + q.width / 2), y: Math.round(q.top + q.height / 2), w: q.width } })()`)
console.log('handle:', handle, 'at:', await evalJs(`(() => { const h = document.querySelector('[data-cicada-splitter]'); const q = h.getBoundingClientRect(); const el = document.elementFromPoint(q.left + q.width / 2, q.top + q.height / 2); return el?.tagName + ':' + (el?.getAttribute('data-cicada-splitter') !== null ? 'splitter' : (el?.className ?? '').toString().slice(0, 30)) })()`))
const beforeW = await evalJs(`document.querySelector('[data-workspace-canvas]')?.getBoundingClientRect().width`)
if (handle !== null) {
  await move(handle.x, handle.y, 0)
  await press(handle.x, handle.y)
  await move(handle.x + 120, handle.y)
  await release(handle.x + 120, handle.y)
}
const afterW = await evalJs(`document.querySelector('[data-workspace-canvas]')?.getBoundingClientRect().width`)
const savedW = await evalJs(`localStorage.getItem('cicada.canvasWidth')`)
console.log(`splitter: ${beforeW} -> ${afterW} px; saved=${savedW}`)
await shot('v2-splitter')

ws.close()
process.exit(0)
