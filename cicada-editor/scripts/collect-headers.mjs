#!/usr/bin/env node
/**
 * KiCad include 依赖闭包收集器（只读扫描，不编译）。
 * 从种子头文件递归收集 KiCad 内部（含 thirdparty bundled）头依赖，产出:
 *   copy: [{from, to}]  —— 树内相对路径，供 git archive 精确拷贝
 *   external: []        —— wx/boost/std 等外部（不拷贝，由宿主平台提供）
 *   missing: []         —— 解析失败的头（需人工裁决）
 * 用法:
 *   node scripts/collect-headers.mjs <kicad根> <种子头...> [--json 输出路径]
 */
import fs from 'node:fs';
import path from 'node:path';

const kicadRoot = process.argv[2];
const seeds = process.argv.slice(3).filter((a) => !a.startsWith('--') && a !== '--json');
const jsonOutIdx = process.argv.indexOf('--json');
const jsonOut = jsonOutIdx >= 0 ? process.argv[jsonOutIdx + 1] : null;
if (jsonOut && seeds.includes(jsonOut)) seeds.splice(seeds.indexOf(jsonOut), 1);

// ── KiCad 内部 include 根：常规 + thirdparty（bundled，随源拷走）──
const roots = [
  ['include', 'include'],
  ['common', 'common'],
  ['eeschema', 'eeschema'],
  ['libs/kimath/include', 'libs/kimath/include'],
  ['libs/core/include', 'libs/core/include'],
  ['libs/sexpr/include', 'libs/sexpr/include'],
  ['libs/kiplatform/include', 'libs/kiplatform/include'],
  // thirdparty bundled（按上游相对路径保留）
  ['thirdparty/dynamic_bitset', 'thirdparty/dynamic_bitset'],
  ['thirdparty/rtree', 'thirdparty/rtree'],
  ['thirdparty/nlohmann_json', 'thirdparty/nlohmann_json'],
  ['thirdparty/fmt', 'thirdparty/fmt'],
  ['thirdparty/clipper2/Clipper2Lib/include', 'thirdparty/clipper2/Clipper2Lib/include'],
];
// 泛化：thirdparty 下任何 `<dir>/include` 也作为根
for (const name of fs.readdirSync(path.join(kicadRoot, 'thirdparty'))) {
  const inc = path.join(kicadRoot, 'thirdparty', name, 'include');
  if (!fs.existsSync(inc) || !fs.statSync(inc).isDirectory()) continue;
  roots.push([`thirdparty/${name}/include`, `thirdparty/${name}/include`]);
}

// 真外部：wx/boost/glm 等宿主平台依赖（KiCad 里由 vcpkg 提供）+ C 标准库
const EXTERNAL_PREFIX = /^(wx|boost|glm|cairo|gtk|gdk|glad|ngspice|BaseTsd|openssl|curl|pthread|sys\/|unistd|windows|tchar)/;
const STD_HEADERS = new Set([
  'algorithm','array','atomic','bitset','cassert','cctype','cerrno','cfloat','chrono',
  'cinttypes','climits','cmath','csetjmp','csignal','cstdarg','cstdbool','cstddef',
  'cstdint','cstdio','cstdlib','cstring','ctime','cwchar','cwctype','deque','exception',
  'filesystem','forward_list','fstream','functional','future','initializer_list','iomanip',
  'ios','iosfwd','iostream','istream','iterator','limits','list','locale','map','memory',
  'mutex','new','numeric','optional','ostream','queue','random','ratio','regex',
  'scoped_allocator','set','shared_mutex','span','sstream','stack','stdexcept',
  'stop_token','streambuf','string','string_view','system_error','thread','tuple',
  'type_traits','typeindex','typeinfo','unordered_map','unordered_set','utility',
  'valarray','variant','vector','version',
  // C 系统头（无 .h 前缀规则之外的）
  'math.h','stdlib.h','stdint.h','stddef.h','stdbool.h','stdarg.h','stdio.h','string.h',
  'ctype.h','wchar.h','locale.h','time.h','limits.h','float.h','inttypes.h','iso646.h',
  'malloc.h','assert.h','errno.h','signal.h','setjmp.h','stdatomic.h','complex.h','fenv.h','tgmath.h',
]);

function walk(dir, acc = []) {
  for (const name of fs.readdirSync(dir)) {
    const p = path.join(dir, name);
    let st;
    try { st = fs.statSync(p); } catch { continue; }
    if (st.isDirectory()) walk(p, acc);
    else if (/\.(h|hpp|hh|inc|in|tcc)$/.test(p)) acc.push(p);
  }
  return acc;
}

const rootMap = new Map();
for (const [dir, toDir] of roots) {
  const base = path.join(kicadRoot, dir);
  if (!fs.existsSync(base)) continue;
  for (const f of walk(base)) rootMap.set(f, path.posix.join(toDir, path.relative(base, f)));
}

function resolveInclude(inc, fromDir) {
  if (inc.startsWith('.')) {
    const p = path.join(fromDir, inc);
    return fs.existsSync(p) ? p : null;
  }
  const p2 = path.join(fromDir, inc);
  if (fs.existsSync(p2)) return p2;
  for (const root of roots) {
    const p = path.join(kicadRoot, root[0], inc);
    if (fs.existsSync(p)) return p;
  }
  return null;
}

function includesOf(absPath) {
  let src = fs.readFileSync(absPath, 'utf8');
  // 剥离块注释与行注释（KiCad bundled 单头版 json.hpp 等把内部 include 全注释掉）
  src = src.replace(/\/\*[\s\S]*?\*\//g, '').replace(/\/\/[^\n]*/g, '');
  const found = [];
  const re = /#\s*include\s*[<"]([^>"]+)[>"]/g;
  let m;
  while ((m = re.exec(src))) found.push(m[1]);
  return found;
}

const copy = new Set();
const external = new Set();
const missing = new Set();
const processed = new Set();

function collect(absPath) {
  if (processed.has(absPath)) return;
  processed.add(absPath);
  const fromDir = path.dirname(absPath);
  for (const inc of includesOf(absPath)) {
    const base = inc.includes('/') ? inc.split('/')[0] : inc;
    if (STD_HEADERS.has(inc) || EXTERNAL_PREFIX.test(inc)) {
      external.add(inc);
      continue;
    }
    const abs = resolveInclude(inc, fromDir);
    if (!abs) {
      if (/^config\.h$/.test(inc) || inc.endsWith('.cmake')) continue;
      missing.add(`${inc} (from ${path.relative(kicadRoot, absPath)})`);
      continue;
    }
    const to = rootMap.get(abs);
    if (to) {
      copy.add(to);
      collect(abs);
    } else {
      const rel = path.relative(kicadRoot, abs).replace(/\\/g, '/');
      copy.add(rel);
      collect(abs);
    }
  }
}

for (const seedRel of seeds) {
  const abs = path.join(kicadRoot, seedRel);
  if (!fs.existsSync(abs)) {
    console.error(`seed missing: ${seedRel}`);
    process.exit(1);
  }
  copy.add(path.relative(kicadRoot, abs).replace(/\\/g, '/'));
  collect(abs);
}

const result = {
  copy: [...copy].sort(),
  external: [...external].sort(),
  missing: [...missing].sort(),
};
if (jsonOut) {
  fs.writeFileSync(jsonOut, JSON.stringify(result, null, 1));
  console.log(`copy=${result.copy.length} external=${result.external.length} missing=${result.missing.length}`);
  console.log('written to', jsonOut);
} else {
  console.log(JSON.stringify(result, null, 1));
}
