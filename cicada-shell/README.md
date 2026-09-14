# cicada-shell — Schematic-Cicada 产品壳（Electron）

> 规格权威：`docs/04-shell.md` §2。本文件只讲"这是什么 / 怎么跑 / 怎么测 / 边界在哪"。

## 是什么

产品窗口壳的第二期形态：一个 Electron 常驻托盘应用，**监督**既有 launcher 栈
（`cicada-launcher/bin/cicada-app.ts`：单实例锁 → 引擎 → DSH host → Edge `--app` 窗口 → 清算），
并补上无窗口 launcher 做不到的事：

1. 单一实例（第二实例唤起既有启动器窗口）；
2. **启动器窗口**（docs/04 §5，Minecraft 式）：左导航 6 页 —— 开始 / 项目 / 资料库 / 符号库 / 运行 / 设置；
   每次启动都经过它（拍板①），点「启动」才起栈开产品窗口，产品窗口关闭后回到启动器；
3. 托盘 + 快捷状态窗（启动阶段、引擎端口、失败原因、`launch.txt` 尾部，均可读）；
4. 退出时 `taskkill /T` 清算整棵子进程树（Windows 无优雅子进程信号；孤儿 host 会永久占用 3123）；
5. 启动前预检（引擎 exe / launcher 入口 / node 缺失 → 直接给可读原因，不 spawn）。

**项目 = DSH workspace**（docs/04 §5.2）：列表来自 DSH 自己的注册表
（`<home>/storages/workspace.json`，**只读**，启动器绝不写它），并与 `<home>/projects/*`
里未注册的目录合并显示；新建项目 = 在 `<home>/projects/<名字>/` 建目录，注册发生在首次启动
（launcher `--workspace <dir>` → host cwd + `CICADA_INITIAL_WORKSPACE`）。

**本批不嵌页面**：产品窗口仍是 launcher 打开的 Edge `--app` 窗口；是否把页面搬进
`BrowserWindow` 留到打包批（docs/04 §2.5）。

## 怎么跑

```
cicada-editor\scripts\start-cicada-shell.bat      # 推荐（双击/命令行）
cd cicada-shell && npm start                      # 等价（需先 npm install）
```

需要 PATH 上有**原生 Node ≥ 22**（`CICADA_NODE_BIN` 可指定）：**host 不能跑在 Electron
自带 Node 上**——实测 Electron 39 会让 `cordis-plugin-hmr` 条目失败
（`--expose-internals is required for HMR service`，原生插件 ABI 不兼容），原因详见 docs/04 §2.1。
`cicada-shell/node_modules` 由 `npm install` 生成（Electron devDependency ~250MB + 二进制 ~210MB；
网络受限时用 `cicada-editor\scripts\install-electron-offline.bat <zip>` 离线装）。

## 页面进度（docs/04 §5.7）

| 页 | 状态 |
|---|---|
| 开始 / 项目 / 运行 / 设置 | **L1 已实现**（项目卡片、新建项目、打开已有目录、启动/停止、**实时控制台**（流式日志 + 自动滚动开关 + 清空）、只读设置表） |
| 资料库（datasheet DB） | **L2 已实现**（表 + 详情：组/引脚/claims 溯源/shape/full.md 预览 + 删除→回收站 + 导出 + 打开回收站） |
| 符号库（精选 / 用户库） | **L3 已实现**（两栏 + 引擎引脚表 + 查看/导出/另存到用户库/删除用户库符号→回收站；精选库只读） |
| `.kicad_sym` 导入、从 shape 重建符号 | **L4 已实现**（导入到用户库：选文件 + 类别，同名冲突显式确认；资料库详情「按 shape 生成/重建符号」走引擎 `/lib/synthesize`） |

## 怎么测

```
cicada-editor\scripts\shell-test.bat             # node --test，输出 logs\shell-test.log（82/82）
cicada-editor\scripts\shell-ui-smoke.bat         # UI 冒烟：隐藏窗口真渲染六页并读回 DOM
                                                 # 报告 logs\shell-ui-smoke.json（每页字数 + 开头 160 字）
cicada-editor\scripts\shell-smoke.bat            # 无窗口无托盘端到端：真起 launcher 全栈再清算
                                                 # 默认带项目工作区（<home>/projects/_smoke）＝启动器真实形态
                                                 # 报告 logs\shell-smoke.json（含 workspace 字段）
cd cicada-shell && npm test                      # 等价于 shell-test
```

测试只覆盖纯逻辑（配置派生、行协议解析/脱敏、进程树清算命令、日志尾部），
不需要 Electron 运行时。

## 边界

- **不重实现 launcher**：启动全序仍由 `cicada-launcher` 单点拥有（docs/04 §2.1）；
  壳只观察它的 stdout 行协议（`cicada: 引擎端口 N` / `cicada: 窗口地址 <url>` / 失败行）。
- **不写日志文件**：壳不落盘；`logs/launch.txt` 由 launcher 写，壳只读尾部且**脱敏 token**。
- **无构建步骤**：`src/*.js` 源码即产物（ESM）；`src/status-preload.cjs` 必须是 CJS
  （Electron 沙箱 preload 不支持 ESM）。
- **零机器路径**：所有路径由 `cicada-shell/` 的上级派生，或用 docs/04 §2.2 的环境变量覆盖。
- **host 的 cwd = 项目目录**（`--workspace`，docs/04 §5.2）：因此 dev 模式的 `tsx` 预载必须是**绝对 file URL**（`import.meta.resolve('tsx/esm')`）——裸写 `tsx/esm` 会按子进程 cwd 解析，在项目目录里直接 `ERR_MODULE_NOT_FOUND`（2026-09-12 实测），并需显式给 `TSX_TSCONFIG_PATH`；工作区目录不存在时退回 launcher 自己的 cwd 并在 `launch.txt` 记一行。
- **不写工作区注册表**：项目列表只读 DSH 的 `workspace.json`；注册/改名走 host 的
  `WorkspaceCommands`（首次启动时由客户端幂等注册）。壳自己只创建 `<home>/projects/<名字>/` 目录。
- **删除一律进回收站**（docs/04 §5.6）：`<home>/trash/<时间戳>/…`，界面明示可还原；库条目一律 `rename`（跨盘回退 copy+remove），从不 unlink。
- **管理服务**（`src/manager/`，docs/04 §5.5）：按需拉起（真 `node.exe` + tsx）、只监听 127.0.0.1 随机端口 + 一次性令牌、stdin 关闭即退出；数据手册读取复用 `cicada-knowledge` 的
  `GlobalDatasheetDb`（单一权威，壳不自己解析 index/detail/shape）。
- **符号几何只问引擎**：符号页的引脚来自 `cicada-engine` 的 `/lib/list` + `/lib/get`（`src/engine-client.js` 按需起一个空文档引擎，空闲 60s 自杀），壳**不解析 `.kicad_sym`**——键与几何仍是引擎单一权威；文件层只负责"哪些文件存在 / 大小 / 删除与拷贝"。
- 打包（electron-builder + 随包 `node.exe` + 生产态 DSH + app-local VC 运行库）属 Batch 4，
  不在本目录实现。
