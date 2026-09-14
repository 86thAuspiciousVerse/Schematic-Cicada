#!/usr/bin/env node
/**
 * m1b-lib-prep.mjs — M1b 精选库数据准备：从官方 kicad-symbols clone
 * (assets/kicad-symbols-src/<Lib>.kicad_symdir/<Name>.kicad_sym, 官方文件原样)
 * 按「最通用优先」清单抽取 → 产出 assets/cicada-libs/<category>/<name>.kicad_sym
 * （原样复制，无改写 = 官方格式直用） + 一份清单 JSON（后续引擎 (category,name)
 * 键模型与 /lib/* 契约共用）。
 *
 * 键模型（docs 09）：键 = category:name；category 即目标目录名。
 * 运行：node scripts/m1b-lib-prep.mjs [--check]
 */
import { cpSync, existsSync, mkdirSync, readFileSync, writeFileSync } from 'node:fs'
import { dirname, join } from 'node:path'
import { fileURLToPath } from 'node:url'

const ROOT = join(dirname(fileURLToPath(import.meta.url)), '..', '..')
const SRC = join(ROOT, 'assets', 'kicad-symbols-src')
const OUT = join(ROOT, 'assets', 'cicada-libs')

/** (category, 官方库目录, 符号名列表) — 最通用优先，覆盖常用类。 */
const MANIFEST = {
  R: ['Device', ['R', 'R_Small', 'R_Shunt', 'R_Potentiometer', 'R_Trim',
    'R_Network04', 'R_Network09', 'R_Pack04']],
  C: ['Device', ['C', 'C_Polarized', 'C_Trim', 'C_Feedthrough', 'C_Small']],
  L: ['Device', ['L', 'L_Iron', 'L_Coupled']],
  D: ['Device', ['D', 'D_Schottky', 'D_TVS', 'D_Zener', 'D_Bridge_+-AA']],
  LED: ['Device', ['LED', 'LED_Filled', 'LED_ABGR']],
  Q: ['Transistor_BJT', ['Q_NPN_BEC', 'Q_PNP_BEC', 'Q_NPN_BCE']],
  QFET: ['Transistor_FET', ['Q_NMOS_GSD', 'Q_PMOS_GSD', 'Q_NMOS_GDS']],
  SW: ['Switch', ['SW_Push', 'SW_SPST', 'SW_DPST', 'SW_DIP_x01', 'SW_SPDT']],
  CRYSTAL: ['Device', ['Crystal', 'Crystal_GND24', 'Crystal_GND2']],
  MISC: ['Device', ['Fuse', 'Fuse_Small', 'Battery']],
  POWER: ['power', ['+3V3', '+5V', '+12V', '+9V', 'GND', 'VCC', 'GNDA', 'VDD', '+5VA', '+24V']],
  CONN: ['Connector_Generic', ['Conn_01x02', 'Conn_01x03', 'Conn_01x04', 'Conn_01x05', 'Conn_01x06',
    'Conn_01x08', 'Conn_02x02_Odd_Even']],
  CONN_LEGACY: ['Connector', ['Barrel_Jack', 'RJ45']],
}

let ok = 0
let missing = []
const catalog = {}
for (const [category, [lib, names]] of Object.entries(MANIFEST)) {
  catalog[category] = []
  mkdirSync(join(OUT, category), { recursive: true })
  for (const name of names) {
    // 多单元规避：任何 01x 之外的单元（_D_1_1 且 _D_2_1？）——只挑单单元件
    const src = join(SRC, `${lib}.kicad_symdir`, `${name}.kicad_sym`)
    if (!existsSync(src)) {
      missing.push(`${lib}:${name}`)
      continue
    }
    const text = readFileSync(src, 'utf8')
    const unitBodies = [...text.matchAll(/\(symbol "([A-Za-z0-9_]+)_(\d+)_(\d+)"\)/g)]
      .map((m) => Number(m[2]))
    const multiUnit = [...new Set(unitBodies)].some((u) => u > 1)
    const pins = (text.match(/\(pin(?![A-Za-z])/g) ?? []).length
    const arcs = (text.match(/\(arc(?![A-Za-z])/g) ?? []).length
    cpSync(src, join(OUT, category, `${name}.kicad_sym`), { recursive: false })
    ok += 1
    catalog[category].push({ name, pins, arcs, multiUnit })
  }
}
const report = {
  generatedAt: new Date().toISOString(),
  source: 'https://gitlab.com/kicad/libraries/kicad-symbols (master, kicad_symbol_lib v20251024)',
  total: ok,
  missing,
  catalog,
}
writeFileSync(join(OUT, 'index.json'), JSON.stringify(report, null, 2))
console.log(`prepared ${ok} symbols; missing: ${missing.length ? missing.join(', ') : 'none'}`)
for (const [cat, items] of Object.entries(catalog)) {
  const arcN = items.filter((i) => i.arcs > 0).length
  const multiN = items.filter((i) => i.multiUnit).length
  console.log(`  ${cat}: ${items.length} (ARC ${arcN}, 多单元 ${multiN})`)
}
