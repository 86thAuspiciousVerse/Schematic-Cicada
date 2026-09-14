#!/usr/bin/env node
/**
 * 从 KiCad 10.0.6 cmake/config.h.cmake 生成 Cicada 最小 config.h。
 * 策略：所有 #cmakedefine → #undef（默认 off，由编译错误逐个开启）；
 * 全部 @VAR@ 占位 → 空字符串/0（未使用宏保持可编译）。
 * 用法：node scripts/generate-config.mjs <模板路径> <输出路径>
 */
import fs from 'node:fs';
import path from 'node:path';

const [, , tplPath, outPath] = process.argv;
const lines = fs.readFileSync(tplPath, 'utf8').split('\n');

const out = [];
for (const raw of lines) {
  let line = raw;
  // #cmakedefine01 X  ->  #define X 0
  let m = line.match(/^\s*#cmakedefine01\s+([A-Za-z0-9_]+)/);
  if (m) {
    out.push(`#define ${m[1]} 0`);
    continue;
  }
  // #cmakedefine X [comment]  ->  #undef X
  m = line.match(/^\s*#cmakedefine\s+([A-Za-z0-9_]+)/);
  if (m) {
    out.push(`/* off by default: */ #define ${m[1]} 0`);
    continue;
  }
  // #define A  "@VAR@"  /  #define A  @VAR@  -> empty
  line = line
    .replace(/"@([A-Z0-9_]+)@"/g, '""')
    .replace(/@([A-Z0-9_]+)@/g, '0');
  out.push(line);
}

fs.writeFileSync(
  outPath,
  `// Cicada 最小 config.h —— 由 scripts/generate-config.mjs 从\n` +
    `// KiCad 10.0.6 cmake/config.h.cmake 生成（@->off，@VAR@->空）。\n` +
    `// 注意：按编译错误逐个开启宏；禁止整包拷贝 KiCad 构建生成物。\n\n` +
    out.join('\n') + '\n',
);
console.log(`config.h written: ${outPath} (${out.length} lines)`);
