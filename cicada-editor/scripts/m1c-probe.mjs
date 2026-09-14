// probe9: wire selectable + add-to-context with a wire item.
const OUT_DIR = '/mnt/c/dsh/Schematic-Cicada/cicada-editor/build-msvc-kicad'
import fs from 'node:fs'
const PORT = 9333
const WEB = fs.readFileSync(`${OUT_DIR}/web-url.txt`, 'utf8').trim()
const sleep = (ms) => new Promise((r) => setTimeout(r, ms))
const targets = await (await fetch(`http://127.0.0.1:${PORT}/json`)).json()
const page = targets.find((t) => (t.type ?? '') === 'page')
let msgId = 0
const pending = new Map()
const ws = new WebSocket(page.webSocketDebuggerUrl)
await new Promise((res, rej) => { ws.onopen = res; ws.onerror = () => rej(new Error('ws')) })
ws.onmessage = (e) => { const m = JSON.parse(String(e.data)); if (m.id !== undefined && pending.has(m.id)) { const p = pending.get(m.id); pending.delete(m.id); m.error ? p.reject(new Error(m.error.message)) : p.resolve(m.result) } }
const send = (method, params = {}) => new Promise((resolve, reject) => { const id = ++msgId; pending.set(id, { resolve, reject }); ws.send(JSON.stringify({ id, method, params })) })
const evalJs = async (expression, awaitPromise = false) => { const r = await send('Runtime.evaluate', { expression, returnByValue: true, awaitPromise }); if (r.exceptionDetails !== undefined) throw new Error(JSON.stringify(r.exceptionDetails)); return r.result?.value }
const click = async (x, y, button = 'left') => { await send('Input.dispatchMouseEvent', { type: 'mousePressed', x, y, button, clickCount: 1 }); await send('Input.dispatchMouseEvent', { type: 'mouseReleased', x, y, button, clickCount: 1 }); await sleep(350) }
await send('Emulation.setDeviceMetricsOverride', { width: 1500, height: 950, deviceScaleFactor: 1, mobile: false })
await send('Page.navigate', { url: `${WEB}&layout=workspace` })
await sleep(9000)
const st = await evalJs(`fetch('/cicada/editor/state').then(r => r.json()).then(async s => { const sc = await fetch('http://127.0.0.1:' + s.enginePort + '/scene', { headers: { 'X-Cicada-Token': s.engineToken } }).then(r => r.json()); const g = document.querySelector('[data-cicada-canvas-pane] svg > g'); const m = /translate\\(([-\\d.]+) ([-\\d.]+)\\) scale\\(([-\\d.]+)\\)/.exec(g.getAttribute('transform')); const q = document.querySelector('[data-cicada-canvas-pane] svg').getBoundingClientRect(); return { sc, vp: { ox: +m[1], oy: +m[2], s: +m[3], left: q.left, top: q.top } } })`, true)
const w = st.sc.wires[0]
const mid = [(w.points[0][0] + w.points[1][0]) / 2, (w.points[0][1] + w.points[1][1]) / 2]
const px = { x: Math.round(st.vp.left + st.vp.ox + mid[0] * st.vp.s), y: Math.round(st.vp.top + st.vp.oy + mid[1] * st.vp.s) }
console.log('wire', w.uuid.slice(0, 8), '@px', px)
await click(px.x, px.y)
await sleep(400)
const sel = await evalJs(`[...document.querySelectorAll('[data-cicada-canvas-pane] [data-selected]')].map(e => e.tagName + ':' + (e.getAttribute('points') ?? '').slice(0, 24))`)
console.log('selected after wire click:', JSON.stringify(sel))
await click(px.x, px.y, 'right')
const menuClick = await evalJs(`(() => { const b = [...document.querySelectorAll('[data-cicada-canvas-pane] button')].find(x => x.textContent.includes('加入到上下文')); if (!b) return 'no-menu'; b.click(); return 'clicked' })()`)
console.log('menu:', menuClick)
await sleep(700)
const status = await evalJs(`[...document.querySelectorAll('[data-cicada-canvas-pane] span')].map(s => s.textContent).join(' | ')`)
console.log('status:', status.slice(0, 80))
ws.close()
process.exit(0)
