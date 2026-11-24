# S100 项目交接（总览）

更新日期：2026-11-24（Asia/Shanghai）。本文供新的 agent **仅凭本仓库**继续工作，说明项目目的、能力边界、文档地图与已核实基线。

本文是**相对稳定的项目总览与计划索引**，不新增 Active，也不把候选工作写成已批准实现目标。构建命令、代码入口、Windows 测试拆分与平台注意见 [`engineering-handoff.md`](engineering-handoff.md)。路线图与字号候选的调查顺序见 [`PROJECT_PLAN.md`](PROJECT_PLAN.md)。负责人、当前进度、安排、阻碍与移交日志见可变快照 [`PROJECT_STATUS.md`](PROJECT_STATUS.md)（与本文分开维护；恢复或回到原团队前先对账并更新该快照）。

## 1. 起始提交与已核实证据

以下为 2026-11-24（Asia/Shanghai）对远端 `origin/main` 执行 `git fetch` 后的核对结果，**不是**假定 2026-11-07 状态仍自动成立。新 agent 接手时必须重新读取远端 `main`，不能假定本文基线永远不变。

| 项 | 值 |
|----|----|
| 仓库 | [MasterLivens/S100](https://github.com/MasterLivens/S100) |
| 已核对的远端 `main` | `33509eff2bf29b0d55f474a5e67abfa0aa24ed9f`（2026-11-07 20:49:10 Asia/Shanghai） |
| 最近正式 Closed | **`S102-GRID-SHIM-RETIRE`**（`CoverageDataset::grid()` 临时兼容层已删除；调用方迁至 `grids[i]` / 数据集级 `sample_wgs84`） |
| Closed 文档 tip | `33509ef`（同上完整 SHA） |
| Close CI | [42374245157](https://github.com/MasterLivens/S100/actions/runs/42374245157) — workflow `ci`；`build-test (ubuntu-latest)` 与 `build-test (windows-latest)` 均为 **success**（作业完成于 2026-11-07 20:55:24 Asia/Shanghai）。这是基线历史结果，**不是**本交接文档的新 CI / 新 CTest。 |
| Eng tip-truth | `14e27bcea431d5b96e057c95957ea2e7753c4459` |
| Eng CI | [42372491173](https://github.com/MasterLivens/S100/actions/runs/42372491173) — 同上双平台 **success**（完成于 2026-11-07 20:30:47 Asia/Shanghai） |
| Open tip / Open CI | `ec97b03` / [42371854738](https://github.com/MasterLivens/S100/actions/runs/42371854738)（亦为历史双绿） |
| 本文档栈 | 叠在工程交接提交 `96695716f00cee34461ca796f6ff759caf99d03f`（`docs/engineering-handoff.md`）之上；该提交的父提交为 `33509ef`，截至核对时**尚未推送**。 |

截至上述核对：远端 `main` 仍停在 `33509ef`，其后没有更新的已推送提交。下一项在登记上仍为 **TBD**；没有已批准的 Active。

**测试声明：** 本交接文档提交本身未重新执行构建或 CTest。不得把上述基线 CI 说成“本次文档的测试已通过”。将来若推送，须由 SoftwareTester（或项目所有者指定的同等验收角色）批准**该次具体提交**，并提供针对该提交范围的 CTest 证明。

## 2. 项目目的与边界

这是跨平台（Linux / Windows）的 **S-100 海图系统**研发仓库，范围包括无 Qt 依赖的核心库、无界面 CLI、Qt6 桌面 viewer 与 CTest，**不是**仅浏览器前端。

目标（研发轨道，Track A）：在同一会话中加载并显示多种 S-100 产品（已落地切片见第 5 节），产品编解码位于共同门面之后，使 UI 与测试保持稳定。认证轨道（Track B：船级社 / MED / IEC 61174 型式认可）与 Track A 分开；**在 Track B 有意启动并完成之前，不得宣称本软件可用于 SOLAS 航行或已获认证。** 本仓库不是已通过型式认可的 ECDIS。

产品决策（已写入仓库文档，2026-10-10）：当前走 **S-100 only** 路径，暂不实现 S-57 / S-63 / S-52 双燃料，也不在当前轨道自行扩展传感器、航线或 BAM 模块。

## 3. 技术栈与依赖（已从仓库核实）

以下来自根目录 `CMakeLists.txt`、`third_party/CMakeLists.txt`、`third_party/hdf5-SYSTEM.md` 与 `.github/workflows/ci.yml`，不是口头约定。

| 依赖 | 仓库中的实际要求 |
|------|------------------|
| 语言 / 构建 | C++17（`CMAKE_CXX_STANDARD 17` 且 REQUIRED）；CMake ≥ 3.16 |
| Qt6 | `s100_viewer` 需要 `Qt6::Widgets` 与 `Qt6::Svg`；选项 `S100_BUILD_VIEWER` 默认 ON。核心库 **不**链接 Qt（合同规则）。 |
| HDF5 | **系统库**，不在 `third_party/` 内嵌源码。CMake 优先 HDF Group CONFIG 包（Windows zip），否则 `find_package(HDF5 REQUIRED COMPONENTS C)`。Linux CI：`libhdf5-dev`。Windows CI：HDF5 1.14.5，且须将运行时 DLL 放入 PATH。 |
| OpenSSL | `find_package(OpenSSL 3.0 REQUIRED COMPONENTS Crypto)`；`s100_security` 链接 `OpenSSL::Crypto`。Windows CI 须能找到 `libcrypto-3-x64.dll`。 |
| Python3 | CMake `find_package(Python3 REQUIRED COMPONENTS Interpreter)`——缺少解释器应导致配置失败，而不是跳过可复现性检查。Linux CI 另装 `python3-pil`（golden 比较）。 |
| 内嵌第三方 | Lua 5.1.5（S-100 Part 13 条款要求 5.1 引擎，编为 C++ 静态库 `s100_lua`）；pugixml 1.14（`s100_pugixml`）。出处与哈希由 CTest `third_party_provenance` 核对。 |
| Linux CI 包 | `build-essential cmake qt6-base-dev qt6-svg-dev libgl1-mesa-dev python3-pil fonts-dejavu-core libhdf5-dev libssl-dev` |
| Windows CI Qt | `jurplel/install-qt-action`，Qt **6.7.0**，`win64_msvc2019_64`（不要把 `qtsvg` 当作 aqt module 传入） |

选项 `S100_ENCDOTNET_ORACLE` 默认 **OFF**，CI 未开启；EncDotNet 对照为 warn-only，不能代替本仓库 CTest。

## 4. 模块边界

高层目标与职责（实现文件级入口见 [`engineering-handoff.md`](engineering-handoff.md) 第 3 节）：

| 目标 / 模块 | 职责 | 说明 |
|-------------|------|------|
| `s100_core` | ISO 8211 / S-101 加载，S-102/104/111 HDF5，S-124 GML，FC/PC 与 Lua 描绘，交换集与更新，会话、拾取、图层、比例尺与显示优先级，时间轴，开机自检 | **不依赖 Qt**；链接 `s100_security`、pugixml、Lua、HDF5 |
| `s100_security` | S-100 Part 15 **签名验证**、信任库、SSE 映射 | 已有 Closed 切片；**不等于** Part 15 加密 / 许可整条链路，也不等于认证完成 |
| `s100_cli` | 无界面加载与会话拾取等 | 安装布局与目录根见 powerup 合同 |
| `s100_viewer` | Qt6 海图窗口、绘制指令绘制、快照 / 调色板 | 可选构建；部分 `viewer_*` 在 Windows CI 中有排除策略 |
| `s100_tests` / `s100_portrayal_tests` | 核心与描绘 CTest | 具体用例以 `CMakeLists.txt` 注册名为准 |

公共头文件在 `include/s100/`。产品目录在 `catalogues/`（S-101 FC/PC 2.0.0，S-102 FC/PC 3.0.0，S-104 FC 2.0.0，S-111 / S-124 FC+PC 2.0.0）。S-104 按既有合同走 FC-only，不要伪造 PC。

## 5. 已具备能力与不完整处

已落地的是**按 tip 合同验收的研发切片**，不是“该标准已完整实现”或“已获认证”。细节与 MUST 断言以各合同及 [`testing/iho-ctest-case-ids.md`](testing/iho-ctest-case-ids.md) 为准；**不要重开**已 Closed 的 case。

**已有 Closed 切片（摘要，非完整标准符合性证明）：**

- **S-101：** ISO 8211 加载（含厂商 Sec.4–10 样本 `IHO-S101-LOAD-0004..0010`）、图层 / 拾取 / 会话统一拾取、交换集更新链（序号、损坏、cancel/reissue、edition harden）、FC/PC 目录完整性、Lua 描绘、桥/组合要素、文字与点标注碰撞、显示优先级与 SCAMIN、显示对照（S-164 子集与 disp-ref）、时间轴 UI。
- **S-102：** 打开与采样、多格网加深、垂向不确定度双通道、QoBC 元数据与多格网绑定；`grid()` 临时兼容层已退役（`S102-GRID-SHIM-RETIRE`）。
- **S-104 / S-111 / S-124：** 首片打开框架（水位/海流多帧采样与描绘；航警列表与几何/文本）；S-111/S-124 的 PC 接线已 Closed（`IHO-PC-WIRE-111-124`）。
- **Part 15 签名：** 验证、producer、CN 形态、aggregator 等 Closed 切片。加密、许可与整条数据保护产品链路仍不在这些切片范围内。
- **安装与开机：** power-up 自检与安装前缀目录根（`(prefix)/share/s100`）。

**明确未完成 / 不得夸大：**

- 未宣称、也未达到 ECDIS 型式认可、IEC 61174 实验室证据或 SOLAS 用途。
- 官方描绘完整度、S-98 互操作、传感器 / 航线 / BAM / 备份、S-57 双燃料：见认证差距与架构“暂不在当前范围”，不是已排期的 Active。
- 正式 S-164 / 实验室脚本型认可数据集：当前以公开样本与合成夹具做研发验收，不是认可证据。
- `docs/testing/hooks-ctest.md` 仍有过时表述（例如曾写 S-102 sample “未开始”）；**以合同与 case-id 登记为准**。
- `docs/architecture.md` 模块状态表中部分早期措辞（如 iso8211 “next”、security “Active”、文末 “signature verify is Active”）可能落后于后续 Closed 记录；**以各合同的 Closed 段与 case-id 表为准**。
- [`certification/ecdis-certified-system.md`](certification/ecdis-certified-system.md) 差距表里 “S-63 / S-100 Part 15 Not started” 应理解为：**签名验证切片已落地**，加密/许可及认证级数据保护仍未开始；不要把旧差距表读成“签名也未做”，也不要读成“Part 15 已完成”。

## 6. 文档地图

| 路径 | 用途 |
|------|------|
| [`PROJECT_HANDOFF.md`](PROJECT_HANDOFF.md) | 本文件：相对稳定的项目总览、基线证据、能力边界 |
| [`PROJECT_PLAN.md`](PROJECT_PLAN.md) | 已批准 vs 提议的计划、字号调查顺序 |
| [`PROJECT_STATUS.md`](PROJECT_STATUS.md) | 可变运作快照：接手人、进度、安排、阻碍、移交/回归日志 |
| [`engineering-handoff.md`](engineering-handoff.md) | 工程接手：阅读顺序、代码入口、构建与平台注意 |
| [`architecture.md`](architecture.md) | 分层架构与历史 Closed 摘记（注意过时行） |
| [`api-contracts.md`](api-contracts.md) | 对外 API 冻结与产品约束 |
| 各 `*-contract.md` | 单次 tip 的 MUST / 非目标 / 证据（最近一次为 [`s102-grid-shim-retire-contract.md`](s102-grid-shim-retire-contract.md)） |
| [`exchange-set.md`](exchange-set.md)、[`portrayal-engine.md`](portrayal-engine.md)、[`portrayal-contract.md`](portrayal-contract.md) | 交换集与描绘 |
| [`test-data-roots-contract.md`](test-data-roots-contract.md) | 测试数据根 |
| [`testing/iho-ctest-case-ids.md`](testing/iho-ctest-case-ids.md) | case-id 与 CTest 映射（登记源） |
| [`testing/visual-acceptance.md`](testing/visual-acceptance.md) | 视觉验收与外部 oracle 边界 |
| [`testing/hooks-ctest.md`](testing/hooks-ctest.md) | 早期 hook 表（可能过时） |
| [`testing/host-functions.txt`](testing/host-functions.txt) | 描绘 host 函数清单 |
| [`certification/README.md`](certification/README.md) | 认证研究包入口（**不是证书**） |
| [`certification/ecdis-certified-system.md`](certification/ecdis-certified-system.md) | 可认证系统形态与差距 |
| [`certification/standards-index.md`](certification/standards-index.md) | 标准与门户索引 |
| [`certification/iec61174-requirements-matrix.md`](certification/iec61174-requirements-matrix.md) | IEC 61174 主题矩阵（暂定） |
| `testdata/**/SOURCE.*`、`tools/encdotnet-s100/SOURCE.md`、`catalogues/*/SOURCE.md` | 样本与工具出处 |

不要依赖本仓库以外的私有工作区路径或未入库的 BRIEF 文件。合同与 case-id 才是可移植的依据。

## 7. 构建与测试（摘要）

根目录 [`README.md`](../README.md) 给出通用命令：

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

CI 在 Linux 上等价地使用 `-C Release`、`--timeout 120`，并设置 `S100_TESTDATA=testdata/s101`、`QT_QPA_PLATFORM=offscreen`、`QT_OPENGL=software`。完整依赖安装、无 GUI 构建的限度、以及 **Windows 并非直接跑全部 `viewer_*`**（先 `-E "viewer_"`，再仅跑 `viewer_text_collision` 与 `viewer_point_label_collision`）见 [`engineering-handoff.md`](engineering-handoff.md) 第 4 节与 `.github/workflows/ci.yml`。不要在未核对合同的情况下扩大 Windows viewer 范围或改 CI 排除项。

**夹具与数据注意：**

- IHO S-101 样本在 `testdata/s101/`（见该目录 `SOURCE.txt`）；拾取夹具 `testdata/s101/fixtures.toml`。
- 真实 S-102/104/111 `.h5`、CHS/SHOM 样本等多被 `.gitignore` 忽略；CI 依赖已跟踪的合成文件（如 `testdata/s102/synthetic_*`、`testdata/s104/synthetic_waterlevel_4326.h5` 等）。缺少被忽略的本地数据时，相关手工对照可跳过，但合同写明 **MUST NOT SKIP** 的合成夹具不得跳过。
- EncDotNet CLI 与其输出被 gitignore；视觉软对照默认不进 CI。
- 不要提交 `build/`、`Testing/` 或 EncDotNet 解包树。不要用 `git add .` 把未跟踪产物带上。

运行 viewer / CLI 的示例见 README。最近一次 shim 退役重点回归见工程交接第 4 节；新改动应按影响面选测。

## 8. 字号候选（提议，不是 Active）

群内曾报告：viewer 将 PC `FontSize:10` 按毫米经 `mm_to_px`（`logicalDpiX`，缺省 96 DPI 时约 10×96/25.4≈38 px，且像素大小有下限）换算，观感上比 EncDotNet 将同类值当作约 10 px 时大约 3–4 倍。该线索已作为 **软 Backlog** 写入 `architecture.md`，**不等于** `S102-GRID-SHIM-RETIRE`，也**不得重开**该 Closed tip。

架构曾建议若要立项，case-id 用 **`IHO-S101-FONT-SIZE`**（钉 PC `FontSize` 单位 mm-vs-px / TextScale）。这只是**提议编号，不是已登记的 Active，也不是已确认的实现目标**。在项目所有者点名、并由规划/架构完成 BRIEF 与 Open freeze 之前，不得当作当前开发任务，不得发明 CTest 名或抢跑改代码。

该视觉差异是**调查起点**，不是“标准单位已被证明写错”的结论。历史对照笔记不能代替对现行 PC / 描绘标准条款、源数据单位与 DPI / TextScale 职责的核对。区分两件事：

1. **已报告的视觉现象**（本 viewer 与某一外部软对照在相同 `FontSize:10` 下看起来差一截）；
2. **已确认的规范语义**（标准要求该值以毫米、像素还是其它单位解释，以及 TextScale / 显示比例尺如何参与）。

未完成第 2 步之前，**不要**把毫米直接改成像素，也不要全局除以 3.8。调查顺序见 [`PROJECT_PLAN.md`](PROJECT_PLAN.md)。

## 9. 决策、验收与推送（离开原机器人团队也可执行）

历史上的角色分工（新 agent 若需调整，须由项目所有者明确）：

- **规划（Research&Planner）+ 架构（SoftwareArchitecture）：** 范围、BRIEF、合同与是否冻结 / 登记 Active。不要自行发明 Active 或下一项 tip 名称。
- **实现（SoftwareEngineer 或接手的实现者）：** 按冻结合同改代码；通常由其推送。
- **验收（SoftwareTester 或所有者指定的同等角色）：** **独占**推送批准。批准必须针对**具体提交**，并附该提交范围的 CTest 证明；没有批准不得推送。

可脱离原讨论群执行的最低纪律：先读最新 `main` 与合同 → 不重开 Closed → 不把提议当 Active → 不提交构建/测试产物与密钥 → 推送前取得验收批准与提交级测试证明。不要依赖 `/workspace` 下未入库的 BRIEF 或其它机器上的本地工作区。

## 10. 接手后的第一步

1. `git fetch` 并核对远端 `main` 是否仍为本文第 1 节的 SHA（或已有更新）。
2. 阅读 [`PROJECT_STATUS.md`](PROJECT_STATUS.md)（先对账再动手）、本文、[`PROJECT_PLAN.md`](PROJECT_PLAN.md)、[`engineering-handoff.md`](engineering-handoff.md)，再按工程接手第 3 节的顺序读架构 / API 合同 / case-id / 最近 Closed 合同。
3. 在干净构建目录按 README / CI / 工程交接复现构建与测试（本文未代替你做这次复现）。
4. 向项目所有者确认下一项；在明确批准（若是字号，须先完成单位调查而不是直接改绘制）之前，不启动功能改动。
