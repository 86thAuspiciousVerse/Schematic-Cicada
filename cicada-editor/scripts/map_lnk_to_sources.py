#!/usr/bin/env python3
"""链接驱动（性能版）：一次扫描 KiCad 树全部 .cpp 建立 (Class, method) -> file 索引，
再将 MSVC LNK2001 日志的 demangled 符号映射到定义文件，输出待补入
KICAD_LIB_SRCS 的源清单与未映射符号。
用法：python3 scripts/map_lnk_to_sources.py <lnk日志> <kicad树根>
"""
import re, sys, os

log_path, kicad_root = sys.argv[1], sys.argv[2]
text = open(log_path, encoding='utf-8', errors='replace').read()

# 1) LNK 符号 demangled 提取
sigs = set()
for m in re.finditer(r'无法解析的外部符号 "([^"]+)"', text):
    sig = m.group(1)
    if '::' in sig: sigs.add(sig)

# 2) 一遍扫描：文件 -> { (class,method) : True }
index = {}
def scan_file(path, key):
    try:
        src = open(path, encoding='utf-8', errors='replace').read()
    except: return
    for m in re.finditer(r'([A-Za-z_][\w:]*)::([A-Za-z_~][\w_~]*)\s*\(', src):
        index.setdefault((m.group(1), m.group(2)), set()).add(key)

files = []
for dirpath, dns, fns in os.walk(kicad_root):
    dns[:] = [d for d in dns if d not in ('build-msvc-kicad', 'build', '.git')]
    for fn in fns:
        if fn.endswith('.cpp') and os.path.basename(dirpath) != 'build':
            files.append(os.path.join(dirpath, fn))
for f in files:
    scan_file(f, os.path.relpath(f, kicad_root).replace('\\', '/'))

# 3) 映射符号
found, missed = {}, []
for sig in sorted(sigs):
    m = re.search(r'([A-Za-z_][\w:]*::[\w_~]+)\(', sig)
    if not m:
        missed.append(sig); continue
    qual = m.group(1)
    cls, method = qual.rsplit('::', 1)
    locs = index.get((cls, method))
    if not locs:
        # 多级限定（A::B::C::method）退化尝试末级类
        locs = index.get((qual.split('::')[-2] if '::' in qual.rstrip(method)[:-2] else method, None))
    if locs:
        for loc in locs: found.setdefault(loc, set()).add(sig)
    else:
        missed.append(sig)

print(f'# sigs={len(sigs)}  files={len(found)}  missed={len(missed)}')
print('# add to KICAD_LIB_SRCS:')
for f in sorted(found): print(f)
print('# missed:')
for s in missed[:50]: print('  ', s)
