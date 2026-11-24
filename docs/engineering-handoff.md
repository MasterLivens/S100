# S100 工程接手说明

更新日期：2026-11-24（Asia/Shanghai）。本文为工程交接，不新增 Active，也不代表后续功能已经获得批准。

## 1. 先从这里开始

- 仓库：[MasterLivens/S100](https://github.com/MasterLivens/S100)。
- 本次核对远端 `main`：`33509eff2bf29b0d55f474a5e67abfa0aa24ed9f`。
- 最近完成：`S102-GRID-SHIM-RETIRE`；移除 `CoverageDataset::grid()` 临时兼容层，调用方已迁移到 `grids[0]` 或数据集级 `sample_wgs84`。
- 该基线的 [GitHub Actions](https://github.com/MasterLivens/S100/actions/runs/42374245157) 已核对为 Linux、Windows 成功。这是基线历史结果，不是本次交接文档的新 CI。
- 当前没有已批准的下一项 Active；不要把下述候选计划当作已开始工作。
- 新 agent 必须先重新读取远端 `main` 和最新提交，不能假定本文记录的基线永远不变。

## 2. 项目目标与边界

这是 Linux / Windows 跨平台 S-100 海图研发系统，采用 **C++17、Qt6、CMake、GitHub Actions**，不改为 Rust。

核心库 `s100_core` 不依赖 Qt；`s100_viewer` 提供 Qt 图形界面，`s100_cli` 提供无界面入口，`s100_tests` 为核心测试入口。已有 S-101、S-102、S-104、S-111、S-124 产品相关代码，以及目录、Lua 描绘、交换集、更新链和 Part 15 签名验证；各项支持范围以合同及测试为准，不能据此宣称完整标准兼容。

当前不自行扩展 S-57 双燃料、传感器、航线或报警模块；本项目不是已通过型式认可的 ECDIS，不能宣称满足 SOLAS 航行使用要求。

## 3. 阅读顺序与代码入口

1. [架构和工作范围](architecture.md)：含历史状态和未来模块；部分早期状态文字可能陈旧，结合最后的 Closed 记录读取。
2. [API 合同](api-contracts.md)：对外接口与约束。
3. [登记的 case-id 和验收映射](testing/iho-ctest-case-ids.md)：不要自己发明已存在流程以外的编号或测试名。
4. [最近关闭的 shim 清理合同](s102-grid-shim-retire-contract.md)。
5. [测试数据根目录合同](test-data-roots-contract.md)、[可视验收](testing/visual-acceptance.md)、[认证边界](certification/README.md)。

常用代码入口：

- `include/s100/`：公共类型和 API；`coverage.hpp` 为栅格覆盖相关接口。
- `src/core/`：ISO 8211、各产品加载、目录、描绘、交换集、会话及更新逻辑。
- `src/security/`：签名验证及信任链相关实现。
- `src/viewer/main.cpp`：Qt viewer 与文字绘制等当前实现。
- `src/core/drawing_instructions.cpp`：PC 绘制指令解析，包括 `FontSize`。
- `tests/`、`testdata/`、`catalogues/`：测试、样本和产品目录；样本来源以各自 SOURCE 文件为准。
- `CMakeLists.txt` 与 `.github/workflows/ci.yml`：实际构建、CTest 注册和双平台 CI 的依据。

## 4. 构建与验证

Linux 依赖按当前 CI 安装：

```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake qt6-base-dev qt6-svg-dev \
  libgl1-mesa-dev python3-pil fonts-dejavu-core libhdf5-dev libssl-dev
cmake -S . -B build-handoff -DCMAKE_BUILD_TYPE=Release
cmake --build build-handoff --config Release
S100_TESTDATA=testdata/s101 QT_QPA_PLATFORM=offscreen QT_OPENGL=software \
  ctest --test-dir build-handoff -C Release --output-on-failure --timeout 120
```

这些命令源自当前 CI；本次只增加文档，没有在新环境重新执行完整构建。Python3 解释器为 CMake 必需项；OpenSSL 要求 3.0；Lua / pugixml 的接入见 `third_party/`。不构建 GUI 可配置 `-DS100_BUILD_VIEWER=OFF`，但不能用无 GUI 测试代替完整验收。

Windows 请照 `.github/workflows/ci.yml` 配置 Qt 6.7.0、HDF5 和 OpenSSL，并把运行时 DLL 放入 PATH。Windows 当前不是直接运行全部 viewer 测试：先 `ctest ... -E "viewer_"`，再分别运行 `viewer_text_collision` 和 `viewer_point_label_collision`；不要在不核对合同的情况下扩大范围或更改 CI 排除项。

最近一次 shim 清理重点回归为 `s100_tests`、`s102_deepen_multigrid`、`s102_uncertainty`、`s102_qbc_meta`、`s102_qbc_multigrid`。新改动还需按其影响范围选测，不限于此列表。

## 5. 后续候选计划（未批准，不是 Active）

架构在群内提出候选 case-id **`IHO-S101-FONT-SIZE`**，等待 MIN YE 确认及 Planner 的 BRIEF / Open freeze；截至本次交接，没有把它登记为 Active。

问题线索：PC 的 `FontSize:10` 在 viewer 中被按毫米换算时，96 DPI 下约为 38 px；历史对比报告称 EncDotNet 约为 10 px。该观察是调查起点，不是最终标准结论，不应直接全局除以 3.8。

建议获批后按以下顺序开展：

1. 核对相应 PC 版本、标准单位、现有 `FontSize` 解析、绘制 DPI 换算和 TextScale 的职责，形成明确单位合同。
2. 固定同一样本、比例尺、DPI、字体和截图条件，复现并记录现状；第三方 viewer 只作辅助对照，不替代标准依据。
3. 按批准合同选择单位修复或显式缩放方案；限定修改范围，不改变符号、碰撞和已有栅格采样语义。
4. 由 Planner / Arch 定义验收及测试映射，补充单位换算和可视回归，再经 Tester 审核及双平台 CI。

其他未来模块只作为架构文档中的长期候选；接手 agent 不应自行挑选实现。

## 6. 协作与提交约束

现有分工：Research&Planner 负责范围 / BRIEF，SoftwareArchitecture 负责合同，SoftwareEngineer 实现，SoftwareTester 验收。新 agent 接手后如需改变流程，由 MIN YE 明确调整。

- 推送前需 SoftwareTester 批准具体提交，且要求的测试通过；不因本说明而绕过审核。
- 避免重开已 Closed 的任务；新问题单独界定范围。
- 不提交缓存、构建目录、测试输出或本地私密配置；不要用 `git add .` 把历史未跟踪产物一并纳入。
- 历史本机工作区为 `/workspace/s100-eng-wt`、`/workspace/s100-chart`；这些只是原环境路径，新 agent 不应依赖它们存在。
- 历史环境的 `build-eng` 曾因 Qt / HDF5 配置失败，临时复用旧对象手动编译不是可复现的接手构建方法；优先使用干净目录及 CI 依赖。
- 原团队有周用量剩余 ≤5% 停工约束。不要把旧日期的用量或重置时间当作当前值；新环境继续任务时应确认适用额度及 owner 的限制。

接手第一步：读最新 `main` → 阅读本说明及上列合同 → 在干净目录复现构建测试 → 向 MIN YE 确认候选字号工作是否获批；在此之前不启动功能改动。
