#!/usr/bin/env node
/** 裁剪：删除 SCH_TEXT::ShowSyntaxHelp（语法帮助 HTML 弹窗，依赖 wxHTML + dialog 生成头；
 *  UI 帮助功能不在模型闭包白名单）。登记见 vendor/kicad/README.md。 */
import fs from 'node:fs';

const file = 'eeschema/sch_label.cpp';
const anchor = 'HTML_MESSAGE_BOX* SCH_TEXT::ShowSyntaxHelp';
let lines = fs.readFileSync(file, 'utf8').split('\n');
let ai = lines.findIndex((l) => l.includes(anchor));
if (ai < 0) { console.error('anchor missing'); process.exit(1); }
let depth = 0, started = false, end = ai;
for (let i = ai; i < lines.length; i++) {
  for (const ch of lines[i]) {
    if (ch === '{') { depth++; started = true; }
    else if (ch === '}') { depth--; if (started && depth === 0) { end = i; i = lines.length; break; } }
  }
}
lines.splice(ai, end - ai + 1);
fs.writeFileSync(file, lines.join('\n'));
console.log(`pruned ShowSyntaxHelp (${file}, ${lines.length} lines left)`);
