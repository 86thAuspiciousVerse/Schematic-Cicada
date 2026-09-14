// m1e-synth-view.mjs — M1e-1 端到端判读
import { existsSync, readFileSync } from 'node:fs'
import { join } from 'node:path'

const work = process.argv[2]
const j = (f) => JSON.parse(readFileSync(join(work, f), 'utf8'))

const s1 = j('s1.json')
console.log('synthesize(1st):', s1.ok === true ? `OK libId=${s1.libId}` : JSON.stringify(s1))
const s2 = j('s2.json')
console.log('synthesize(replace):', s2.ok === true ? `OK warnings=${JSON.stringify(s2.warnings)}` : JSON.stringify(s2))
const s3 = j('s3.json')
console.log('synthesize(bad electrical):', s3.error?.code === 'bad_request' ? `OK 400 (${s3.error.message})` : JSON.stringify(s3))
try {
  // 状态码由 m1e-404verify.mjs（node，无 cmd 引号风险）断言；此处只判响应体契约。
  const miss = j('miss.json')
  console.log('/lib/get unknown:', miss.error?.code === 'not_found' ? 'OK not_found' : `UNEXPECTED ${JSON.stringify(miss)}`)
} catch { console.log('/lib/get unknown: CHECK-MISSING') }

const get = j('get.json')
console.log('lib/get IC:AMS1117: name=' + get.name, 'fields=', JSON.stringify(get.fields), 'pins=', (get.pins ?? []).length)

const place = j('place.json')
console.log('place-symbol:', place.ok === true ? 'OK' : JSON.stringify(place))
const scene = j('scene.json')
const u = (scene.components ?? []).find((c) => c.libId === 'IC:AMS1117')
console.log('scene IC:AMS1117 at=(' + (u?.x ?? '?') + ',' + (u?.y ?? '?') + ') pins=' + (u?.pins?.length ?? 0))
if (u) for (const p of u.pins) console.log(`  pin ${p.number} at=(${p.x},${p.y}) core=(${p.ix},${p.iy})`)

console.log('user-lib/IC files:', existsSync(join(work, 'dirlist.txt')) ? readFileSync(join(work, 'dirlist.txt'), 'utf8').trim().replace(/\n/g, ',') : 'none')

const get2 = j('get2.json')
console.log('persist after restart: lib/get IC:AMS1117 ->', get2.ok !== undefined || get2.name ? `OK name=${get2.name} pins=${(get2.pins ?? []).length}` : JSON.stringify(get2))
const list2 = j('list2.json')
const ic = (list2.symbols ?? []).filter((s) => s.libId.startsWith('IC:'))
console.log('lib/list IC keys after restart:', ic.map((s) => s.libId).join(',') || 'NONE')
