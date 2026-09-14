@echo off
set TK=0a9e9639d1e86f5631f9aa184e8516ee
curl -s -m 8 -X POST -H X-Cicada-Token:%TK% -d "{\"fileHash\":\"39a0c2c0ac166915dd76b8e20f59d41a28d54e352b5acd7b2ca52218b93ec026\",\"op\":\"clear\"}" http://127.0.0.1:16126/ops -o NUL
