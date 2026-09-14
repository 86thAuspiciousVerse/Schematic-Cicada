#!/usr/bin/env node
/**
 * 切片 1 裁剪：删除 KiCad API 序列化（protobuf）相关代码。
 * 依据：白名单排除区（API server 序列化）不属于数据模型闭合；
 * SCH_ITEM 基类未声明 Serialize/Deserialize（非虚、仅 API 层使用），删除无副作用。
 * 处理的本地改动（登记在 vendor/kicad/README.md）：
 *   - eeschema/sch_line.cpp ：api include ×3 + Serialize/Deserialize 函数块
 *   - eeschema/sch_label.cpp: api include ×2 + magic_enum.hpp（未使用） + Serialize/Deserialize 函数块
 */
import fs from 'node:fs';
import path from 'node:path';

const cases = [
  {
    file: 'eeschema/sch_line.cpp',
    dropIncludes: [
      '#include <api/api_enums.h>',
      '#include <api/api_utils.h>',
      '#include <api/schematic/schematic_types.pb.h>',
    ],
    dropFunctions: ['void SCH_LINE::Serialize', 'bool SCH_LINE::Deserialize'],
  },
  {
    file: 'eeschema/sch_label.cpp',
    dropIncludes: [
      '#include <api/api_utils.h>',
      '#include <api/schematic/schematic_types.pb.h>',
      '#include <magic_enum.hpp>',
    ],
    dropFunctions: [
      'void SCH_LABEL::Serialize', 'bool SCH_LABEL::Deserialize',
      'void SCH_DIRECTIVE_LABEL::Serialize', 'bool SCH_DIRECTIVE_LABEL::Deserialize',
      'void SCH_GLOBALLABEL::Serialize', 'bool SCH_GLOBALLABEL::Deserialize',
      'void SCH_HIERLABEL::Serialize', 'bool SCH_HIERLABEL::Deserialize',
    ],
  },
];

function stripBlock(src, startIdx) {
  // 从 startIdx（'void SCH_LINE::...' 行位置）起，括号匹配到函数结束的 '}' 行
  // 函数签名行可能含返回值在上一行？KiCad 风格为同一行 'void X::F( ... )' 起。
  let depth = 0;
  let started = false;
  let i = startIdx;
  for (; i < src.length; i++) {
    const line = src[i];
    for (const ch of line) {
      if (ch === '{') { depth++; started = true; }
      else if (ch === '}') {
        depth--;
        if (started && depth === 0) return i; // 函数结束（含该行）
      }
    }
    // 遇到下一行函数定义（深度未开）说明签名跨行失败——保守返回 null
    if (!started && i > startIdx && /^\s*(void|bool|int|EDA_ITEM|wxString|static)\s+\w+::/.test(line)) {
      return null;
    }
  }
  return null;
}

for (const c of cases) {
  const abs = path.resolve(c.file);
  let lines = fs.readFileSync(abs, 'utf8').split('\n');

  // 1) 删 include 行
  const dropSet = new Set(c.dropIncludes);
  lines = lines.filter((l) => {
    for (const inc of dropSet) if (l.includes(inc)) return false;
    return true;
  });

  // 2) 删函数块（按起始锚点查找；只删一个版本）
  for (const anchor of c.dropFunctions) {
    let ai = lines.findIndex((l) => l.includes(anchor));
    if (ai < 0) {
      console.warn(`anchor not found: ${c.file}: ${anchor}`);
      continue;
    }
    const end = stripBlock(lines, ai);
    if (end === null) {
      console.error(`could not match block: ${c.file}: ${anchor} — aborting file`);
      process.exit(1);
    }
    lines.splice(ai, end - ai + 1);
  }

  fs.writeFileSync(abs, lines.join('\n'));
  console.log(`pruned: ${c.file} (${lines.length} lines left)`);
}
