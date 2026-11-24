# S100 项目计划

更新日期：2026-11-24（Asia/Shanghai）。本文区分**已批准 / 已完成**与**仅提议、尚未批准**的工作。不新增 Active，不把候选 case-id 写成当前实现目标。

项目总览与基线证据见 [`PROJECT_HANDOFF.md`](PROJECT_HANDOFF.md)。构建与代码入口见 [`engineering-handoff.md`](engineering-handoff.md)。当前负责人、进度与移交日志见可变快照 [`PROJECT_STATUS.md`](PROJECT_STATUS.md)。

## 1. 当前状态（已核对）

| 项 | 状态 |
|----|------|
| 已核对远端 `main` | `33509ef`（完整 SHA 见总览）；2026-11-24 fetch 时无更新的已推送提交 |
| 最近 Closed | **`S102-GRID-SHIM-RETIRE`**（2026-11-07）。证据 URL 见总览，属历史 CI，不是本文档的新测试。 |
| 登记队列 | **空**。`testing/iho-ctest-case-ids.md` Backlog 中仅余已转向 Closed 的 `IHO-S101-SCAMIN` → `IHO-SCAMIN-FILTER`、`IHO-S101-DRAW-PRIORITY` → `IHO-DISPLAY-PRIORITY`，**不要重开**。 |
| 下一项 | **TBD**。须由项目所有者点名，再由规划 / 架构 BRIEF 并 Open freeze。 |
| Active | **无**。 |

## 2. 已批准且已完成的轨道（摘要）

以下为仓库合同已标 Closed / Done 的研发切片，属于 Track A（产品研发）。它们批准的是当时那一刀的 MUST，而不是“该国际标准已全部实现”或 Track B 认证。

大致时间线（均为已关闭，细节以各合同为准）：

- S-101 加载、图层、拾取、描绘与视觉回归（含文字/点标注碰撞、显示对照）。
- S-102 覆盖、多格网、不确定度、QoBC，以及 `grid()` 兼容层退役。
- S-104 / S-111 多帧打开与采样，S-124 航警列表，111/124 PC 接线，时间轴 UI。
- 交换集更新链、cancel/reissue、edition harden。
- Part 15 **签名验证**相关切片（不是加密/许可全链路）。
- 目录完整性、会话拾取、SCAMIN、显示优先级、开机自检与安装前缀。

完整登记见 [`testing/iho-ctest-case-ids.md`](testing/iho-ctest-case-ids.md)。不要把“已 Closed”理解成可以顺手改其 tip-truth 预言。

## 3. 提议但未批准：`IHO-S101-FONT-SIZE`

**状态：PROPOSED（提议） / 不是 Active / 不是已确认的实现目标。**

架构建议：若项目所有者确认下一项是字号问题，case-id 使用 **`IHO-S101-FONT-SIZE`**，用于钉住 PC `FontSize` 的单位（毫米 vs 像素）以及 TextScale / DPI 的职责。规划侧已记录该建议，但**在所有者点头并完成 BRIEF + Open freeze 之前不定 Active、不抢跑**。

### 3.1 已报告现象 ≠ 已确认的规范语义

已记录的视觉线索（见 `architecture.md` 软 Backlog 与 `src/viewer/main.cpp` 中 `mm_to_px`）：本 viewer 把 PC `FontSize:10` 当作毫米，按设备 `logicalDpiX`（缺省 96）换算为约 38 px；历史对照称 EncDotNet 对同类值的观感约为 10 px，大约差 3–4 倍。`docs/portrayal-engine.md` 亦写“font size from mm”，这只说明**当前实现假设**，不能单独当作标准已裁定的证据。

上述笔记**不能证明**标准单位写错，也不能证明应把毫米全局改成像素。必须区分：

- 报告的视觉差异（本实现 vs 某一软对照）；
- 现行 S-100 / S-101 描绘目录与相关标准中，`FontSize`、偏移（同样走 `mm_to_px`）、符号缩放与 TextScale 的规范语义。

### 3.2 若获批准，建议的调查顺序（仍不是开工许可）

不要一上来把毫米替换为像素，或用 3.8 这类经验系数做全局缩放。建议顺序：

1. **先查标准与源数据单位**：针对仓库实际使用的 PC 版本（S-101 PC 2.0.0 等）核对 `FontSize` / TextScale / 局部偏移的单位与适用范围；对照 `src/core/drawing_instructions.cpp` 的解析与 viewer 的 DPI 换算、像素下限，写出明确的单位合同草案。第三方 viewer（EncDotNet、NIWC 等）只作辅助对照，**不能替代**标准条款。
2. **在固定条件下复现现状**：同一单元格、比例尺、DPI、字体、调色板与截图尺寸，记录本 viewer 与对照的像素结果；标明哪些是实现差异、哪些仍未在标准中确认。
3. **再选择修复策略**（须写进获批合同）：单位纠正、显式 TextScale、或其它经合同允许的缩放；限定在文字绘制与合同写明的范围，不顺手改符号、碰撞、栅格采样或已 Closed 的 tip-truth。
4. **验收**：由规划 / 架构定义 MUST 与测试映射后再实现；经 Tester（或同等角色）审核，并在 Linux + Windows CI 上按当时合同执行。不要发明与现有流程冲突的失败前缀或第二个 CTest 体系。

未完成第 1–2 步就改绘制，视为偏离本计划。

## 4. 分阶段路线图（请看清“提议”与“已批准”）

| 阶段 | 性质 | 内容 |
|------|------|------|
| Track A 已落地切片 | **已批准且已 Closed** | 第 2 节所述 S-101/102/104/111/124、更新链、签名验证、描绘与安装等研发刀次。 |
| 下一项 tip | **未批准（TBD）** | 须所有者点名。字号仅在被点名并 freeze 后才能成为 Active。 |
| `IHO-S101-FONT-SIZE` | **提议，未批准** | 第 3 节。批准前不改功能代码。 |
| 描绘与产品补全（S-98 互操作、更完整的官方描绘、更多产品深度等） | **长期提议，未排期** | 可见于架构“未来模块”与认证差距，**不是**登记中的 Active。接手者不得自行挑选开工。 |
| Track B 认证计划 | **未启动** | MSC.530 + IEC 61174 等路径见 `docs/certification/`。无所有者明确立项前，不宣称认证工作已经开始。 |
| S-57 双燃料 / 传感器 / 航线 / BAM | **已决定暂缓** | 不是当前计划项，除非所有者撤销该决策。 |

README 中仍保留早期 P0/P1/P2 范围表述，部分能力已超出当初 MVP 措辞；规划以合同与本文件为准，不要只按 README 的旧阶段表推断“下一步必须是 P2”。

## 5. 立即下一步

1. 复现干净构建与测试（见工程交接）；本计划文档未代替执行。
2. 等待项目所有者指定下一项。
3. 若指定字号：先按第 3.2 节做单位与 DPI / TextScale 调查并形成合同草案，再 Open freeze；不要直接改 `mm_to_px`。
4. 任何推送仍须针对该次提交的验收批准与 CTest 证明。
