# IHO 对照 → 第一批 CTest 用例 ID

Derived from `iho-s101-test-datasets-matrix.md`.  
**不改变当前路线**：进行中仍是 CI 绿 → Viewer CoverageFill → S-102 叠显 → 桥/组合要素 → 文字碰撞。本清单只给 Tester 挂 ID，能复用现有测试的标 Reuse，新开的标 Backlog。

Legend: **Reuse** = 已有或稍加断言；**Backlog** = 排入现计划空档，不插队。

| Case ID | IHO 来源 | 断言要点 | 落点 | 状态 |
|---------|----------|----------|------|------|
| `IHO-S101-LOAD-0008` | cells/0008 + Sec.8 | 5s 内加载，features>0 | `test_load` | Reuse |
| `IHO-S101-LOAD-0003` | cells/0003 + Sec.3 | 加载成功，面几何合法 | `test_load` / geom | Reuse |
| `IHO-S101-LOAD-0006` | cells/0006 + Sec.6 | Bridge 等无 SPAS 聚合可空几何 | `test_load` Bridge | Reuse |
| `IHO-S101-GEOM-FC` | Sec.3–4 GEOMOD | 几何类型∈FC 允许集 | FC geometry check | Reuse |
| `IHO-S101-ATTR-BASIC` | Sec.7 | ATTR 解析，code 可查 | ATTR tests | Reuse |
| `IHO-S101-LAYER-FC` | Sec.1/8 | classify_layers，无 unknown | `test_layers` | Reuse |
| `IHO-S101-PICK-WGS84` | 拾取 + fixtures | ≤1e-5°；bounds↔CATALOG/DataCoverage | pick / fixtures | Reuse |
| `IHO-S101-PORTRAY-ZERO-ERR` | Sec.5/8 + PC 2.0.0 | 样例 portray lua_errors 可控 | portrayal tests | Reuse |
| `IHO-S101-SAFETY-CONTOUR` | S-164 7.0 | 改 SafetyContour → DepthArea 指令变 | colour/safety tests | Reuse |
| `IHO-S101-UPD-CHS` | S-164 2.2.2 类 | `.000`+`.001` 要素数变化、ED/UPD | exchange-set | Reuse |
| `IHO-S101-UPD-REJECT-BARE` | 契约 | 拒单独 `open_dataset(.001)` | exchange-set | Reuse |
| `IHO-S101-CATALOGUE-PIN` | FC/PC 配对 | sha256 + 版本一致 | `catalogue_provenance` | Reuse |
| `IHO-S102-OPEN-SAMPLE` | S-102 产品 | open/bounds/sample WGS84 | s102 tests | Reuse/in flight |
| `IHO-S102-OVERLAY-SESSION` | 叠显契约 | ChartSession 键叠 S-101 | session | Reuse |
| `IHO-S102-DEEPEN-MULTIGRID` | S-102 deepen | **Closed/Done** 2026-10-19 — CI [39141930422](https://github.com/MasterLivens/S100/actions/runs/39141930422) @ `5e05675` dual-green; `s102_deepen_multigrid` Passed Ubuntu+Win (native `s100_tests`+HDF5 — **no Win O1 a/b**); Eng `0dd39f2` already on tip — docs close only; oracles MUST #1–6 unchanged; **O1=`grid()` shim → Backlog** (shim stays; still used in single-grid / UNCERT / QBC); next `IHO-S102-UNCERTAINTY` | `s102_deepen_multigrid` | Closed |
| `IHO-S102-UNCERTAINTY` | S-102 uncertainty layer rev1 | **Closed/Done** 2026-10-25 — CI [40142462597](https://github.com/MasterLivens/S100/actions/runs/40142462597) @ `13b7c3c` dual-green; `s102_uncertainty` Passed Ubuntu+Win (native `s100_tests`+HDF5 — **no Win O1 a/b**); Eng `b2d17de` + QBC-META `7b20a56` already on tip — docs close only; oracles MUST #1/#1b/#2–7 tip-truth unchanged; **O1=#1b tip-truth broadcast SUCCESS**; soft-nit T1/U2b body aligned (`30e3e7f`); `grid()` shim stays Backlog; next `IHO-S102-QBC-META` | `s102_uncertainty` | Closed |
| `IHO-S102-QBC-META` | S-102 QoBC meta + §10 broadcast | **Closed/Done** 2026-10-25 — CI [40154401620](https://github.com/MasterLivens/S100/actions/runs/40154401620) @ `670708f` dual-green; `s102_qbc_meta` Passed Ubuntu+Win (native `s100_tests`+HDF5 — **no Win O1 a/b**); Eng `7b20a56` already on tip — docs close only; oracles MUST QBC-1..4 + BCAST-1..2 tip-truth unchanged; **O1=tip-truth / docs-close freeze SUCCESS**; failure faces **`QBC-*` ≠ `BCAST-*`**; NullInstruction / no id colour band; `grid()` shim stays Backlog; next `IHO-S102-QBC-MULTIGRID` | `s102_qbc_meta` | Closed |
| `IHO-S102-QBC-MULTIGRID` | S-102 QoBC multi-grid bind | **Closed/Done** 2026-10-25 — CI [40156013552](https://github.com/MasterLivens/S100/actions/runs/40156013552) @ `30f3546` dual-green; `s102_qbc_multigrid` Passed Ubuntu+Win (native `s100_tests`+HDF5 — **no Win O1 a/b**); Eng `aabbb54` already on tip — docs close only; oracles MUST QBC-MG-1..5 tip-truth unchanged; **O1=tip-truth / docs-close freeze SUCCESS**; failure face **`QBC-MG-*`** (do **not** reopen META **`QBC-*`/`BCAST-*`**); forbid `grids[0]` hard-bind; O1 hybrid shape+origin; `grid()` shim stays Backlog; next **TBD** | `s102_qbc_multigrid` | Closed |
| `IHO-S104-OPEN-FRAME` | S-104 first slice | open≥2 frames；sample 随帧变；单帧 portray；ChartSession 叠 S-101 | `s104_open_frame` | Active |
| `IHO-S111-OPEN-FRAME` | S-111 first slice | open≥2 frames；sample speed/dir；Arrow stride；ChartSession 叠 S-101 | `s111_open_frame` | Active |
| `IHO-S124-OPEN-LIST` | S-124 first slice | GML open；`warnings.size()==2`；列表；叠图一帧；ChartSession | `s124_open_list` | Active |
| `IHO-SESSION-PICK-UNIFIED` | multi-product pick | Session 统一拾取；分点 oracle；`true`+可空 hits | `session_pick_unified` | Active |
| `IHO-CATALOGUE-COMPLETE` | FC/PC 补齐 | S-104 **仅 FC**（有 `PC_2.0.0/` → FATAL；跳过 PC pair）；S-111/124 FC+PC sha+pair FATAL | `catalogue_provenance` | Active |
| `IHO-PC-WIRE-111-124` | PC 接线 | S-124 `S124HostAdapter`+`portray_nav_warnings_pc`；S-111 `SCAROW*`+ColorProfiles；XSLT WARN；禁造 S-111 Lua；no-SKIP | `pc_wire_111_124` | Active |
| `IHO-SCAMIN-FILTER` | SCAMIN 显隐 | `visible_at_scale` / filter；oracle A–H；开/关 IgnoreScaleMinimum；no-SKIP | `scamin_filter` | Active |
| `IHO-DISPLAY-PRIORITY` | 显示优先级 | plane 秩 + `drawing_priority` 升序；oracle 钉序；viewer 用 core sort；no-SKIP | `display_priority_order` | Active |
| `IHO-TIMELINE-UI` | 时间轴 UI | S-104/111 `timeline_entries` + CLI `--list-frames` + viewer scrubber；no-SKIP | `timeline_ui` | Active |
| `IHO-S101-BRIDGE-RENDER` | 桥/组合要素渲染 | **Closed/Done** 2026-10-18 — CI [39117072306](https://github.com/MasterLivens/S100/actions/runs/39117072306) @ `065887b` dual-green `fasc_bridge_render` Passed; oracles 1–11 + **#12 BRIDGE01≥1 MUST** (was WARN); #12a print-only; #13 pixels deferred viewer/DISPLAY-COMPARE; VG12210 UI / other structure–equipment Backlog; next `IHO-S101-TEXT-COLLISION` | `fasc_bridge_render` | Closed |
| `IHO-S101-TEXT-COLLISION` | 3.1 ENC Display | **Closed/Done** 2026-10-18 — CI [39123185463](https://github.com/MasterLivens/S100/actions/runs/39123185463) @ `845c51c` dual-green `viewer_text_collision` Passed (Ubuntu+Windows); **O1=(b)** Win narrow-open Passed (no fallback to a); oracles unchanged; unpaired `Kind::Text` only; no algorithm rewrite this tip; next `IHO-S101-POINT-LABEL-COLLISION` (shared `--no-text-collision` already; separate case/CTest) | `viewer_text_collision` | Closed |
| `IHO-S101-POINT-LABEL-COLLISION` | 3.1 ENC Display | **Closed/Done** 2026-10-18 — CI [39129316141](https://github.com/MasterLivens/S100/actions/runs/39129316141) @ `9a1531c` dual-green `viewer_point_label_collision` Passed (Ubuntu+Windows); **O1=(b)** Win narrow-open Passed (no fallback to a); oracles unchanged (PL∩PL pairs + glyph survival); shared `--no-text-collision`; no algorithm rewrite this tip; next `IHO-S164-DISPLAY-COMPARE` then `IHO-S102-DEEPEN-MULTIGRID` | `viewer_point_label_collision` | Closed |
| `IHO-S164-UPD-SEQ` | S-164 Ed1.0.0 2.3.3（2.0.1 草案 2.3.2） | 乱序/重复/跳号/错版次 → 拒收，保留最后好状态；正向对照 001–003 先过；各用例断言具体错误码；回滚摘要一致；合成夹具 `testdata/s101/updates/`；no-SKIP；Linux+Win | `exchange_update_robustness` | Active |
| `IHO-S164-UPD-CORRUPT` | 2.2.1 / 2.1.5 | 截断/坏 leader/未知 RUIN/缺目标 → 拒收，数据集逐位不变（强保证）；合成夹具 `testdata/s101/updates/`；no-SKIP；Linux+Win | `exchange_update_robustness` | Active |
| `IHO-S164-CANCEL-REISSUE` | S-164 Ed1.0.0 取消 2.9.1（2.8.1）/ 取消替换 2.9.2 / 再版 2.3.6、2.9.3（2.8.2）/ 新版迟到 2.3.4（2.3.3）/ 旧版更新 2.3.5；S-101 PS §11.3.1–11.3.3、B-7.1.1 | DSED 无点修复；ED=0 取消→清空+后续 `DatasetCancelled`；再版跳过 ≤n；新版 superseded；ED> → `NewerEditionPending`；会话整体替换；no-SKIP；Linux+Win | `exchange_cancel_reissue` | Active |
| `IHO-S164-EDITION-HARDEN` | S-164 Ed1.0.0 旧版更新 2.3.5 / 新版迟到 2.3.4；S-101 PS §11.3.2–11.3.3、B-7.1.1 | 旧版更新（0<ED<基版 ED）在所有序列路径（`open_dataset_with_updates`/`open_cell_series`/会话/viewer/CLI）→ `superseded`，不报错；单独 `apply_s101_update` 仍 `EditionMismatch`；ED> 仍 `NewerEditionPending`；共用 `classify_update_edition`；基版 DSED 不可解析/缺 DSID/ED=0 → 拒收（`Invalid`/`BadStructure`），绝不按 ED0 加载；`[[edition_fixture]]`；rev1.4 与 cancel-reissue 断言不变；no-SKIP；Linux+Win | `exchange_edition_hardening` | Active |
| `IHO-S164-SIG` | S-100 Ed 5.2.1 Part 15 / Part 17; S-98 2.0.0 §9.1.1 / §20.4.3 c / App. B; S-164 1.0.0 §2.5–2.7 (2.0.1 draft names) | CATALOG.SIGN + per-file ECDSA-384-SHA2; id/issuer chain; injectable verify_time; SSE map (116/124→109); VerifyPolicy Enforce/Off (product Enforce, lib default Off); trust IHO 5.2 + test SA in test_mode; official/non-official; standalone UNVERIFIED; no --allow-unsigned; OpenSSL 3.0 + OPENSSL_API_COMPAT=30000 + OPENSSL_NO_DEPRECATED; no-SKIP; Linux+Win | `exchange_signature` | Active |
| `IHO-VIEW-GOLDEN-DAY` | 显示冒烟 | 锚点色 + 无洋红 + 漂移帽 | `viewer_golden` | Reuse（非 ECDIS 观感） |
| `IHO-S164-DISPLAY-COMPARE` | S-164 AA5 Base/Standard figs | **Closed/Done** 2026-10-18 — CI [39133583148](https://github.com/MasterLivens/S100/actions/runs/39133583148) @ `ba63833` dual-green; **O1=(a)** Ubuntu-bind (`viewer_s164_display_compare` MUST NOT SKIP Passed + Win full matrix green; Notes Win `viewer_*` exclude, same family as golden/snapshot); no `ci.yml` change; Eng `12ae582` already on tip — docs close only; oracles rev2 unchanged; BRIDGE #13 soft WARN only; next `IHO-S102-DEEPEN-MULTIGRID` | `viewer_s164_display_compare` | Closed |
| `IHO-S164-POWERUP` | S-164 Ed1.0.0 §2.1.1 / §2.2.1–2.2.2 (+ 2.0.1 draft); S-98 §20.1 / §20.4.3 c / App. B; IEC 61174/4.4.1 (via S-164); cert §9 / IEC matrix (PU-1) | rev1: PU-1…5 MUST+Block (ops/exchange only); PU-6 WARN stub; PU-2 = catalogue-complete pins (S-101/102/111/124 FC+PC, S-104 FC only); PU-3/4 **project decisions** (SIG); PU-4 mini gold `verify_exchange_set`; CLI Pass/Warn=0 Block=2; D7 UNVERIFIED allowed; CI synthetic only; no producer-code; no-SKIP; Linux+Win | `powerup_self_check` | Active |
| `IHO-S164-SIG-PRODUCER` | S-98 Ed 2.0.0 §9.1.1 CN MRN producer segment ↔ CATALOG `producerCode`; Part 17; SIG D8 follow-on | rev1: Enforce after crypto Ok; CN **seg3 only** (upper exact, no zero-strip; **seg4 never compared**) vs CATALOG `producerCode`; O out; ST→OfficialClass + **ST=`DATA_AGGREGATOR` exempt**; missing producerCode → N/A-pass; mismatch → skip + reason `producer_code_mismatch` (primary) + project-map SSE 109 (secondary; not App.B; not 126); fileName fallback OFF; fixtures `producer_match`/`producer_mismatch` DATA_PRODUCER 1:1 (99ZZ+/00AA−); old `positive/` exempt; AGGREGATOR exempt elevated permanent by **`IHO-S164-SIG-AGGREGATOR`** (rev1); hook `verify_exchange_set` only; MUST NOT break `exchange_signature` Ok; no-SKIP; Linux+Win | `exchange_signature_producer` | Active |
| `IHO-S164-SIG-CN-SHAPE` | S-98 Ed 2.0.0 §9.1.1 CN MRN shape + ST → OfficialClass; SIG D8/A9 follow-on | rev1: conforming CN = `urn:mrn:iho:<prod>:<id>` or `urn:mrn:iho:org:<prod>:<id>` (case-insensitive; `<prod>`=`[A-Za-z0-9]{2,8}`, `<id>`=`[0-9]{1,8}`); accept `org:` yes; order ST then CN shape; non-conforming CN → NonOfficial even if ST official; SCHEME_ADMINISTRATOR same; **no** new Enforce skip; reuse PRODUCER CN parser; no producerCode gate; fixtures cn_shape_ok/bad; no-SKIP; Linux+Win | `exchange_signature_cn_shape` | Active |
| `IHO-S164-SIG-AGGREGATOR` | S-98 Ed 2.0.0 §9.1.1 ST=`DATA_AGGREGATOR` on behalf of members; SIG-PRODUCER Backlog closed as policy | rev1: ST=`DATA_AGGREGATOR` → **permanent exempt** CN seg3↔`producerCode` Block (not member/aggregator/either-as-Block); DATA_PRODUCER unchanged; missing producerCode → N/A-pass; optional WARN `aggregator_producer_code_divergent` when producerCode present ≠ CN (**no** skip / **no** 109 / no new SSE); leaf certificateRef only; fixture `aggregator_exempt/` (AGGREGATOR ST + member producerCode ≠ CN → Enforce still installs); reuse PRODUCER/CN-SHAPE (do not reopen); CTest `exchange_signature_aggregator` MUST NOT SKIP; Linux+Win; queue next install-prefix → **`IHO-S164-POWERUP-INSTALL-PREFIX`** | `exchange_signature_aggregator` | Active |
| `IHO-S164-POWERUP-INSTALL-PREFIX` | S-98 §20.1 persistent FC/PC; POWERUP PU-2 follow-on; CMake install layout | rev1: catalogue root `(prefix)/share/s100` + env `S100_CATALOGUE_ROOT` override; Dev keeps `CMAKE_SOURCE_DIR` pin; Install resolve separate; `install()` MUST `catalogues/` exclude `testdata/`; product PU-4/5 skip/N/A; `S100_DEFAULT_*` same root; CTest `powerup_install_prefix` DESTDIR/`--prefix` + PU-2 Pass existence-only (sha=`catalogue_provenance`); keep `powerup_self_check`; no-SKIP; Linux+Win; no FASC/deb·rpm/TDS/reopen PU | `powerup_install_prefix` | Active |

## Backlog（不插队，挂在现计划之后）

| Case ID | IHO 来源 | 依赖 | 说明 |
|---------|----------|------|------|
| `IHO-S101-LOAD-0004..0010` | Sec.4–10 cells | 扩 `testdata/s101` | 扩样例，不改引擎接口 |
| `IHO-S101-SCAMIN` | 3.7.7 / Sec.9–10 | 显示比例尺 | → `IHO-SCAMIN-FILTER` |
| `IHO-S101-DRAW-PRIORITY` | 3.6 Display Priorities | → `IHO-DISPLAY-PRIORITY` |
| `IHO-DISP-REF-COMPARE` | Display Test Template | 参考实现截图 | 替代「只靠像素漂移」谈观感 |
| `S102-GRID-SHIM-RETIRE` | api-contracts / deepen | after UNCERT / QBC migrate off `grid()` | Retire `CoverageDataset::grid()` → `grids[0]` shim (DEEPEN O1=Backlog). Callers: `test_s102` single-grid / UNCERT / QBC; then remove shim. Do **not** insert ahead of Active S-102 tips. |

## Tester 操作建议

1. 现有 Reuse 行：在测试名或 `docs/testing/hooks-ctest.md` 里挂上 Case ID，零改计划。  
2. Backlog：等当前 S-102 刀次签收后再开，一次只捞与当前模块相关的 1–2 个 ID。  
3. 准推规则不变：新 Case 进 `main` 仍由 SoftwareTester 放行。

