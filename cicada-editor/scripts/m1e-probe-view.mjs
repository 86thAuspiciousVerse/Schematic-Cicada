// m1e-probe-view.mjs — 查看 place R:R_Shunt 后的 /scene pin 坐标（量级检查）
import { readFileSync } from 'node:fs'
import { join } from 'node:path'

const work = process.argv[2]
const scene = JSON.parse(readFileSync(join(work, 'scene.json'), 'utf8'))
const ops = JSON.parse(readFileSync(join(work, 'ops.json'), 'utf8'))
console.log('ops.ok =', ops.ok)
for (const c of scene.components ?? []) {
  console.log(`component ${c.refdes} libId=${c.libId} at=(${c.x},${c.y})`)
  for (const p of c.pins ?? []) console.log(`  pin ${p.number} at=(${p.x},${p.y}) core=(${p.ix},${p.iy})`)
}
