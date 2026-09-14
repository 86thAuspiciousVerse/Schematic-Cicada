# assets — 产品资产与符号库数据

## 符号库

### `cicada-libs/`（入库跟踪：精选库成品）

M1b 精选符号库：官方 `.kicad_sym` 文件**原样复制**，按键模型 `category/name` 两级目录
（`R/R.kicad_sym`、`POWER/+5V.kicad_sym`…），键 = `category:name`（docs/09）。
`index.json` = 清单（生成时间 / 来源 / 每符号 pins/arcs/multiUnit 标记）。

- 再生成：`node cicada-editor/scripts/m1b-lib-prep.mjs`（从 `kicad-symbols-src/` 抽取清单符号）
- 引擎装载：`cicada-engine.exe --lib-dir assets/cicada-libs`（两级目录枚举）

### `kicad-symbols-src/`（**gitignore，不跟踪**：官方数据源 clone）

官方符号库**稀疏 clone**，仅精选所需符号，供 `m1b-lib-prep.mjs` 抽取。

- 源：`https://gitlab.com/kicad/libraries/kicad-symbols`（master，`kicad_symbol_lib` `version 20251024`；**GitLab 是现行权威**——GitHub KiCad/kicad-symbols 是 KiCad5 旧档，勿用）
- 布局：`<Lib>.kicad_symdir/<Name>.kicad_sym`（每符号一文件，与键模型一一对应）
- 重建（选择性 clone 更新）：
  ```
  git clone --filter=blob:none --sparse https://gitlab.com/kicad/libraries/kicad-symbols.git assets/kicad-symbols-src
  cd assets/kicad-symbols-src
  git sparse-checkout set --cone Device power Connector Switch Transistor_BJT Transistor_FET Connector_Generic
  ```
- 更新时机：官方库版本升级或扩充精选清单时重新 clone / `git pull` + 重跑 prep 脚本。

## 产品 logo

`黑底白线logo.png`（主用，深色底 + 白线条）/ `白底黑线logo.png`（浅色主题）/ 金色与透明底变体。
二期 Electron 打包图标 = `黑底白线logo.png`（04 §2）。
