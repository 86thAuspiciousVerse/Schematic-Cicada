// m1b-libget-check.mjs — 校验 M1b 收尾复验产物（/lib/list 键唯一 / /lib/get fields）
// usage: node m1b-libget-check.mjs <workdir>
import { readFileSync } from 'node:fs'
import { join } from 'node:path'

const work = process.argv[2]
const list = JSON.parse(readFileSync(join(work, 'list.json'), 'utf8'))
const seen = {}
const dup = []
for (const s of list.symbols) {
  if (seen[s.libId]) dup.push(s.libId)
  seen[s.libId] = 1
}
console.log(`lib/list total=${list.symbols.length} dup=${dup.length ? dup.join(',') : 'none'}`)
console.log('first6: ' + list.symbols.slice(0, 6).map((s) => `${s.libId}:${s.pins}pin`).join(' '))

const get = JSON.parse(readFileSync(join(work, 'get.json'), 'utf8'))
console.log('lib/get R:R_Shunt:')
console.log(JSON.stringify(get, null, 1))
const fieldsOk = get.fields && Object.keys(get.fields).length >= 4
  && get.fields.Reference !== undefined && get.fields.Value === 'R_Shunt'
console.log('FIELDS-OK: ' + fieldsOk)

const led = JSON.parse(readFileSync(join(work, 'get-led.json'), 'utf8'))
console.log('lib/get LED:LED fields: ' + JSON.stringify(led.fields) + ' pins=' + (led.pins?.length ?? '?'))
const rR = list.symbols.filter((s) => s.libId === 'R:R')
console.log('R:R key count in list: ' + rR.length)
