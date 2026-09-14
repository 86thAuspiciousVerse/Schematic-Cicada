# cicada-editor

Schematic-Cicada 的**引擎**：把 KiCad 的解析 / 几何 / 连接性内核抽成**无 GUI 的本地 HTTP 服务**，
供画布前端与 AI 管道调用。产品窗口（浏览器 / Electron 启动器）与 AI 宿主不在本目录。

## 定位
- **无 GUI**：不在进程内嵌 WebView —— 窗口由外部浏览器（一期 `Edge --app`）或 Electron 启动器承担。
- **只绑 127.0.0.1 + 令牌**：启动时 `--port 0` 取动态端口，stdout 打印 `cicada-engine: 127.0.0.1:<端口> <令牌>`；
  每个请求带 `X-Cicada-Token`。
- **坐标系 = IU 整数（0.01mm）**：引擎与 JSON 一律 IU，前端渲染层才换算缩放；库几何来自 `.kicad_sym`，
  文件里写 mm 文本。

## 端点

| 端点 | 作用 |
|---|---|
| `GET /scene` | 场景快照：元件与引脚（连接点 + 引脚根部坐标）、导线、junction、标签、no_connect |
| `POST /ops` | 语义算子：放符号 / 连线 / 标签 / 电源符号 / 未连接标记 / 断开 / 删除；带 fileHash 冲突保护 |
| `POST /document` | 装载或清空文档（空串清文档；文件缺失时建父目录并写空文档） |
| `POST /export` | 导出 KiCad `.kicad_sch`（内嵌 `lib_symbols`，KiCad 可直接打开） |
| `POST /lib/synthesize` | 形状块 → 确定性符号 → 写用户库 `IC/<name>.kicad_sym` 并立即装载 |
| `GET /lib/list` / `GET /lib/get` | 库目录 / 单符号（引脚几何、字段） |
| `POST /shutdown` | 退出 |

## 目录

```
src/service/       引擎服务：HTTP、场景 JSON、算子、形状合成、库装载
src/kicad_bridge/  .cicada_sch ↔ KiCad 模型桥（装载与 lib 体合成）
src/core/          KiCad 抽取内核（解析、连接图、文档编辑）
src/vendor/kicad/  上游 KiCad 源码裁剪副本（遵循其原始许可）
scripts/           构建、重建与验收脚本
```

## 构建与验证（PowerShell）

```powershell
# 重建（MSVC x64；需要本地 KiCad / vcpkg 依赖与 third_party/）
.\scripts\m1b-rebuild.bat

# 引擎单测
ctest --test-dir build-msvc-kicad -R "cicada-engine-(libtest|shapesynth-test)"

# 库装载回归
.\scripts\m1b-libtest.bat

# 端到端：装载 → 导出 → kicad-cli 解析 → 网表 → 电气完整性 → 往返一致
.\scripts\m1-export-verify.bat <工作区目录>

# 领域单测（引擎桥 / 运行时 / 知识 / 画布）
.\scripts\m1-vitest.bat
```

## 与产品其余部分的关系

- 画布与 AI 对话界面：配套仓库的 `packages/client/*` 客户端插件。
- 窗口与进程监督：`../cicada-shell/`（Electron 启动器）。
- AI 管道（角色契约、知识图景、数据手册流水线、流程编排）：配套 harness 仓库。
