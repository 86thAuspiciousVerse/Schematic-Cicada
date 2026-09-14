// m1e-shape-demo.mjs — 用 subagent 产出的形状块走 /lib/synthesize 全链
// usage: node m1e-shape-demo.mjs <port> <token> <workdir> <shape.json>
import { readFileSync, writeFileSync } from 'node:fs'
import { join } from 'node:path'

const [port, token, work, shapePath] = process.argv.slice(2)
const block = JSON.parse(readFileSync(shapePath, 'utf8'))

const synth = await fetch(`http://127.0.0.1:${port}/lib/synthesize`, {
  method: 'POST',
  headers: { 'X-Cicada-Token': token, 'Content-Type': 'application/json' },
  body: JSON.stringify(block),
}).then((r) => r.json().catch(() => ({ error: { message: `HTTP ${r.status}` } })))

console.log('synthesize(shape.json):', synth.ok === true ? `OK libId=${synth.libId}` : JSON.stringify(synth))

const get = await fetch(`http://127.0.0.1:${port}/lib/get`, {
  method: 'POST',
  headers: { 'X-Cicada-Token': token, 'Content-Type': 'application/json' },
  body: JSON.stringify({ libId: 'IC:AMS1117' }),
}).then((r) => r.json())

console.log('lib/get IC:AMS1117: name=' + get.name, 'category=' + get.category, 'pins=' + (get.pins ?? []).length)
for (const p of get.pins ?? []) console.log(`  pin ${p.number} ${p.name} type=${get.fields?.Reference} at=(${p.x},${p.y}) angle=${p.angle}`)

const scene = await fetch(`http://127.0.0.1:${port}/scene`, { headers: { 'X-Cicada-Token': token } })
  .then((r) => r.json()).catch(() => undefined)
console.log('scene ok:', scene !== undefined)

writeFileSync(join(work, 'result.txt'), JSON.stringify({ synth, get }, null, 2))
