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
| `IHO-S102-DEEPEN-MULTIGRID` | S-102 deepen | grids≥2 合成夹具；sample 路由；N×CoverageFill；FourShades/SafetyContour | `s102_deepen_multigrid` | Active |
| `IHO-S102-UNCERTAINTY` | S-102 uncertainty layer rev1 | dual-channel sample；depth-only compound §10（fixture B rev1 → `has_uncertainty==false`；broadcast flip→QBC-META）；VG `13031` vendor ext（toggle 不藏 13030）；symbolic bands；真样 NOAA unc metres only；Linux+Win | `s102_uncertainty` | Active |
| `IHO-S102-QBC-META` | S-102 QoBC meta + §10 broadcast | QBC U32 id grid + featureAttributeTable load；pick/sample lookup（surveyAuthority/ZOC/…）；PC NullInstruction（不画 id 色带）；fixture C no-SKIP；NOAA QoBC via TEST-DATA-ROOTS（缺→not exercised；set-but-bad→FAIL；CHS 可无 QoBC）；**BCAST-*** 与 **QBC-*** 失败面分离（fixture B broadcast elect）；Linux+Win | `s102_qbc_meta` | Active |
| `IHO-S104-OPEN-FRAME` | S-104 first slice | open≥2 frames；sample 随帧变；单帧 portray；ChartSession 叠 S-101 | `s104_open_frame` | Active |
| `IHO-S111-OPEN-FRAME` | S-111 first slice | open≥2 frames；sample speed/dir；Arrow stride；ChartSession 叠 S-101 | `s111_open_frame` | Active |
| `IHO-S124-OPEN-LIST` | S-124 first slice | GML open；`warnings.size()==2`；列表；叠图一帧；ChartSession | `s124_open_list` | Active |
| `IHO-SESSION-PICK-UNIFIED` | multi-product pick | Session 统一拾取；分点 oracle；`true`+可空 hits | `session_pick_unified` | Active |
| `IHO-CATALOGUE-COMPLETE` | FC/PC 补齐 | S-104 **仅 FC**（有 `PC_2.0.0/` → FATAL；跳过 PC pair）；S-111/124 FC+PC sha+pair FATAL | `catalogue_provenance` | Active |
| `IHO-PC-WIRE-111-124` | PC 接线 | S-124 `S124HostAdapter`+`portray_nav_warnings_pc`；S-111 `SCAROW*`+ColorProfiles；XSLT WARN；禁造 S-111 Lua；no-SKIP | `pc_wire_111_124` | Active |
| `IHO-SCAMIN-FILTER` | SCAMIN 显隐 | `visible_at_scale` / filter；oracle A–H；开/关 IgnoreScaleMinimum；no-SKIP | `scamin_filter` | Active |
| `IHO-DISPLAY-PRIORITY` | 显示优先级 | plane 秩 + `drawing_priority` 升序；oracle 钉序；viewer 用 core sort；no-SKIP | `display_priority_order` | Active |
| `IHO-TIMELINE-UI` | 时间轴 UI | S-104/111 `timeline_entries` + CLI `--list-frames` + viewer scrubber；no-SKIP | `timeline_ui` | Active |
| `IHO-S101-BRIDGE-RENDER` | 桥/组合要素渲染 | 0006 夹具 Bridge10/SpanFixed6/SpanOpening4/Pylon6；VG12210/prio24；聚合 Null；CHGRD 线；BRIDGE01 存在；FASC 图 B∪C 在 VG12210 开/关下同出同滤；RadarOverlay 翻 plane；Linux+Win；no-SKIP | `fasc_bridge_render` | Active |
| `IHO-S164-UPD-SEQ` | S-164 Ed1.0.0 2.3.3（2.0.1 草案 2.3.2） | 乱序/重复/跳号/错版次 → 拒收，保留最后好状态；正向对照 001–003 先过；各用例断言具体错误码；回滚摘要一致；合成夹具 `testdata/s101/updates/`；no-SKIP；Linux+Win | `exchange_update_robustness` | Active |
| `IHO-S164-UPD-CORRUPT` | 2.2.1 / 2.1.5 | 截断/坏 leader/未知 RUIN/缺目标 → 拒收，数据集逐位不变（强保证）；合成夹具 `testdata/s101/updates/`；no-SKIP；Linux+Win | `exchange_update_robustness` | Active |
| `IHO-S164-CANCEL-REISSUE` | S-164 Ed1.0.0 取消 2.9.1（2.8.1）/ 取消替换 2.9.2 / 再版 2.3.6、2.9.3（2.8.2）/ 新版迟到 2.3.4（2.3.3）/ 旧版更新 2.3.5；S-101 PS §11.3.1–11.3.3、B-7.1.1 | DSED 无点修复；ED=0 取消→清空+后续 `DatasetCancelled`；再版跳过 ≤n；新版 superseded；ED> → `NewerEditionPending`；会话整体替换；no-SKIP；Linux+Win | `exchange_cancel_reissue` | Active |
| `IHO-S164-EDITION-HARDEN` | S-164 Ed1.0.0 旧版更新 2.3.5 / 新版迟到 2.3.4；S-101 PS §11.3.2–11.3.3、B-7.1.1 | 旧版更新（0<ED<基版 ED）在所有序列路径（`open_dataset_with_updates`/`open_cell_series`/会话/viewer/CLI）→ `superseded`，不报错；单独 `apply_s101_update` 仍 `EditionMismatch`；ED> 仍 `NewerEditionPending`；共用 `classify_update_edition`；基版 DSED 不可解析/缺 DSID/ED=0 → 拒收（`Invalid`/`BadStructure`），绝不按 ED0 加载；`[[edition_fixture]]`；rev1.4 与 cancel-reissue 断言不变；no-SKIP；Linux+Win | `exchange_edition_hardening` | Active |
| `IHO-S164-SIG` | S-100 Ed 5.2.1 Part 15 / Part 17; S-98 2.0.0 §9.1.1 / §20.4.3 c / App. B; S-164 1.0.0 §2.5–2.7 (2.0.1 draft names) | CATALOG.SIGN + per-file ECDSA-384-SHA2; id/issuer chain; injectable verify_time; SSE map (116/124→109); VerifyPolicy Enforce/Off (product Enforce, lib default Off); trust IHO 5.2 + test SA in test_mode; official/non-official; standalone UNVERIFIED; no --allow-unsigned; OpenSSL 3.0 + OPENSSL_API_COMPAT=30000 + OPENSSL_NO_DEPRECATED; no-SKIP; Linux+Win | `exchange_signature` | Active |
| `IHO-VIEW-GOLDEN-DAY` | 显示冒烟 | 锚点色 + 无洋红 + 漂移帽 | `viewer_golden` | Reuse（非 ECDIS 观感） |
| `IHO-S164-DISPLAY-COMPARE` | S-164 AA5 Base/Standard figs | MUST: SHA+SOURCE, snapshot, Day RGB anchors, display-presets on/off via dump; soft pixel WARN | `viewer_s164_display_compare` | Active |

## Backlog（不插队，挂在现计划之后）

| Case ID | IHO 来源 | 依赖 | 说明 |
|---------|----------|------|------|
| `IHO-S101-LOAD-0004..0010` | Sec.4–10 cells | 扩 `testdata/s101` | 扩样例，不改引擎接口 |
| `IHO-S101-SCAMIN` | 3.7.7 / Sec.9–10 | 显示比例尺 | → `IHO-SCAMIN-FILTER` |
| `IHO-S101-DRAW-PRIORITY` | 3.6 Display Priorities | → `IHO-DISPLAY-PRIORITY` |
| `IHO-S101-TEXT-COLLISION` | 3.1 ENC Display | 文字碰撞开/关重叠对数 | `viewer_text_collision` | Active |
| `IHO-S101-POINT-LABEL-COLLISION` | 3.1 ENC Display | PointLabel 配对 Text；开/关 PointLabel 重叠对数；符号存活计数 | `viewer_point_label_collision` | Active |
| `IHO-DISP-REF-COMPARE` | Display Test Template | 参考实现截图 | 替代「只靠像素漂移」谈观感 |
| `IHO-S164-POWERUP` | 2.1.1 | Track B | 认证启动包 |

## Tester 操作建议

1. 现有 Reuse 行：在测试名或 `docs/testing/hooks-ctest.md` 里挂上 Case ID，零改计划。  
2. Backlog：等当前 S-102 刀次签收后再开，一次只捞与当前模块相关的 1–2 个 ID。  
3. 准推规则不变：新 Case 进 `main` 仍由 SoftwareTester 放行。

