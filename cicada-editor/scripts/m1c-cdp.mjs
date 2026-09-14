// M1c verify: minimal CDP client (Node >=22 global WebSocket + fetch) against
// the headless Edge started by start-cdp-edge.bat. Navigates to the cicada
// workspace page, waits, captures a screenshot.
// Run (WSL node, from anywhere): node ../cicada-editor/scripts/m1c-cdp.mjs
import fs from 'node:fs'

const WEB = fs.readFileSync('/mnt/c/dsh/Schematic-Cicada/cicada-editor/build-msvc-kicad/web-url.txt', 'utf8').trim()
const OUT = '/mnt/c/dsh/Schematic-Cicada/cicada-editor/build-msvc-kicad/m1c-cdp.png'
const PORT = 9333

// Blank target is auto-created by the browser; find it, then navigate.
let targets = []
for (let i = 0; i < 40; i += 1) {
  try {
    targets = await (await fetch(`http://127.0.0.1:${PORT}/json`)).json()
    if (targets.some((t) => (t.type ?? '') === 'page')) break
  } catch {
    // browser not up yet
  }
  await new Promise((r) => setTimeout(r, 500))
}
const page = targets.find((t) => (t.type ?? '') === 'page')
if (page === undefined) throw new Error('no page target')

let msgId = 0
const pending = new Map()
const ws = new WebSocket(page.webSocketDebuggerUrl)
await new Promise((resolve, reject) => {
  ws.onopen = resolve
  ws.onerror = (e) => reject(new Error(`ws error: ${String(e.message ?? e)}`))
})
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

await send('Page.enable')
await send('Emulation.setDeviceMetricsOverride', {
  width: 1500, height: 950, deviceScaleFactor: 1, mobile: false,
})
await send('Page.navigate', { url: `${WEB}&layout=workspace` })
await new Promise((r) => setTimeout(r, 8000))
const shot = await send('Page.captureScreenshot', { format: 'png' })
fs.writeFileSync(OUT, Buffer.from(shot.data, 'base64'))
console.log(`SHOT-OK ${OUT}`)
ws.close()
process.exit(0)
