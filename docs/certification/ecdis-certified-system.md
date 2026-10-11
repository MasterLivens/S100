# What a class-approvable ECDIS / chart system looks like

Research snapshot for the team (Research&Planner). **Not legal advice.** Official texts are paywalled or flag-state controlled; use the index in `standards-index.md` to obtain editions.

Our current `/workspace/s100-chart` is an **engineering R&D viewer**, not a type-approved ECDIS. Closing that gap means building (and proving) the full stack below.

> **Product decision (2026-10-10, MIN YE):** ignore S-57 for now. Engineering track is **S-100-only**. S-57/S-63/S-52 dual-fuel remains documented below as regulatory context / deferred scope — do not implement until revived.

## 1. Legal frame (why “船级社认证” exists)

To satisfy **SOLAS V/18 & V/19** chart carriage with electronic means, an ECDIS must:

1. Be **type-approved** (Administration or RO / notified body acting for flag).
2. Use **up-to-date official ENCs**.
3. Stay compatible with **latest applicable IHO standards**.
4. Have **adequate independent back-up** arrangements.

Classification societies (DNV, ABS, LR, CCS, ClassNK, …) typically act as **recognized organizations / MED notified bodies**: they run or witness type approval, issue Module B (design) + Module D/E/F (production), and for EU ships enable the **Wheelmark** (MED item **4.30** ECDIS).

## 2. Installation timeline (regulatory; our build targets ≥2029 S-100 path)

| Install date | Performance standard | Test method |
|--------------|----------------------|-------------|
| ≥ 2009 and < 2026 | MSC.232(82) | IEC 61174 Ed.4 |
| **2026-01-01 … 2028-12-31** | MSC.530(106)/Rev.1 **or** MSC.232(82) | IEC 61174 Ed.4 **or** Ed.5 |
| **≥ 2029-01-01** | MSC.530(106)/Rev.1 only | **IEC 61174 Ed.5** |

Sources: MSC.530(106)/Rev.1; ClassNK ECDIS guidance; IHO ENC & ECDIS page (updated May 2026).

## 3. Standards stack a certified system must implement

### 3.1 IMO (what the system must do)

- **MSC.530(106)/Rev.1** — S-100-era ECDIS performance standards (display, route planning/monitoring, alarms, sensors, back-up, updates, …).
- Legacy: **MSC.232(82)**, older A.817(19) path for historical installs.
- Related: **MSC.191(79)** presentation of navigation-related information; **MSC.302(87)** bridge alert management; **A.694(17)** general requirements for shipborne radio/nav equipment.
- Guidance: **MSC.1/Circ.1503** ECDIS – Guidance for good practice.

### 3.2 IEC (how labs test / type-approve)

| Standard | Role |
|----------|------|
| **IEC 61174** Ed.4 (2015) / **Ed.5** (DIS 2026) | ECDIS operational & performance requirements, **methods of testing**, required results |
| **IEC 60945** | Environmental / EMC (temp, vibration, humidity, compass safe distance, …) |
| **IEC 62288** | Presentation of navigation-related information (symbols, colours, day/dusk/night) |
| **IEC 61162-1/-2/-450** | NMEA / digital interfaces (GPS, gyro, log, AIS, radar, …) |
| **IEC 62923-1/-2** | Bridge Alert Management (BAM) |
| Optional interfaces often on certificates | VDR (IEC 61996), BNWAS, INS, MSI/NAVTEX, route transfer |

Typical **MED Module B** certificate cites exactly this set (see commercial ECDIS Wheelmark certificates from DNV et al.).

### 3.3 IHO — S-57 ECDIS path (**deferred** for this project)

| Spec | Role |
|------|------|
| **S-57** | ENC transfer standard |
| **S-52** + Presentation Library | Chart content & display aspects |
| **S-52 Annex A:100** Ed.5.0.0 (in force 2026-01-01) | S-100 ECDIS presentation library for **S-57 ENC** during dual fuel |
| **S-63** | Data protection (encrypted ENC) |
| **S-64** | Test data sets for type approval (S-57 era) |
| **S-61** | RNC product spec (if RCDS mode claimed) |

### 3.4 IHO — S-100 ECDIS path (**active** product target)

| Spec | Role | Force note (IHO May 2026) |
|------|------|---------------------------|
| **S-100** Ed.5.2.1 | Universal hydrographic data model | In force 2026-01-01; ECDIS must support 5.2.x |
| **S-98** | S-100 ECDIS & **interoperability** catalogues | Ed.3.0.0 TBA; Ed.2.0.0 initial operational available |
| **S-164** | Test data sets for S-100 ECDIS (+ instruction manual) | Ed.2.0.0 TBA; Ed.1.0.0 for implementation/testing |
| **S-101** Ed.2.0.0 | ENC | In force 2026-01-01 |
| **S-102** Ed.3.0.0 | Bathymetric surface | In force |
| **S-104** Ed.2.0.0 | Water level | In force |
| **S-111** Ed.2.0.0 | Surface currents | In force |
| **S-124** Ed.2.0.0 | Navigational warnings | In force |
| **S-128** Ed.2.0.0 | Catalogue of nautical products | In force |
| **S-129** Ed.2.0.0 | Under keel clearance management | In force |
| **S-100 Part 15** | Data protection (S-100 products) | Parallel to S-63 for S-57 |
| **S-158:\*** | Validation checks per product / interoperability | Implementation/testing editions |

Portal: https://iho.int/en/enc-ecdis

## 4. Functional anatomy of a certifiable ECDIS

A type-approved product is not “a chart viewer.” Labs expect these **subsystems** (mapped to MSC / IEC 61174 themes):

1. **Chart data management**
   - Load official exchange sets; apply sequential updates; detect gaps; ENC update status report.
   - S-57 (**S-63** decrypt) and S-100 (**Part 15**) permit/encryption paths.
   - Dual-fuel S-57 + S-101: **deferred** (product decision).
2. **Portrayal engine**
   - Official Presentation Library / S-100 Portrayal Catalogue + Lua (Part 9).
   - Display categories (base / standard / other); safety contour & depth; SCAMIN; day/dusk/night palettes (IEC 62288).
   - **S-98 interoperability** rules when multiple S-1xx layers overlay.
3. **Navigation sensors & own-ship**
   - Position, COG/SOG, heading, water speed; integrity / fallback; sensor failure alarms.
   - Interfaces per IEC 61162.
4. **Route planning & monitoring**
   - Create/edit/check routes; cross-track; critical points; look-ahead; anti-grounding using ENC (+ S-102/S-104/S-129 where claimed).
5. **Alerts & indications**
   - Navigational alarms + BAM integration (IEC 62923); audible/visual; acknowledge/history.
6. **Pick report / interrogation**
   - Query chart objects and overlaid products; textual report meeting IHO/IEC expectations.
7. **Radar / AIS / Navtex overlays** (as claimed on certificate)
8. **Back-up ECDIS / independent arrangement**
   - Second station or approved alternative; mode switch documented.
9. **System integrity**
   - Power, display failure, software identity / version for survey; often **cyber** (e.g. IACS UR E27 / class cyber rules) on recent certificates.
10. **Hardware & environmental**
    - Marine HMI, brightness, compass safe distance, EMC — IEC 60945 lab reports.
11. **Test artefacts**
    - Pass **S-64** (S-57) and **S-164** (S-100) scripted scenarios with required screenshots/results per IEC 61174.

## 5. Type-approval process (what we must plan for)

```
Requirements freeze (IMO + IHO editions claimed)
        ↓
Design & software configuration management
        ↓
Factory / lab tests: IEC 61174 (+ S-64/S-164) , 62288, 61162, 62923
        ↓
Environmental / EMC: IEC 60945
        ↓
Notified body / RO: MED Module B (EC type examination)  [or national TA]
        ↓
Production quality: Module D (or E/F)
        ↓
Wheelmark / flag certificate → ship install & survey
```

China path often: **CCS** (or flag) type approval referencing the same IEC/IHO set. EU ships: **MED 4.30**. US: sometimes USCG acceptance via MED mutual recognition numbers on certificates.

## 6. Gap vs our project today

| Certifiable ECDIS need | s100-chart now |
|------------------------|----------------|
| Type-approval intent / IEC 61174 matrix | Research only |
| Dual-fuel S-57 + S-101 | **Deferred** — not in roadmap |
| Official portrayal / S-98 | Not started |
| S-63 / S-100 Part 15 | Not started |
| Sensors, route, BAM, back-up | Out of scope |
| S-64 / S-164 formal TDS | Using public S-101 TDS cells only |
| IEC 60945 hardware | Software on PC |
| Class / MED engagement | None |

**Recommendation:** keep **two tracks** —

- **Track A — Product R&D** (current): multi-product chart engine + Qt viewer (S-101→102/104/111/124).
- **Track B — Certification program** (with Architect): treat target as **S-100 / MSC.530 + IEC 61174 Ed.5**; claim list + IEC 61174 matrix (S-57 rows deferred); lab & class engagement plan.

Without Track B, we must **never** claim navigational / SOLAS use.

## 7. Immediate document deliverables for Architect

1. This file — system shape & gap.
2. `standards-index.md` — bibliography & download portals.
3. Next (with Architect): `iec61174-requirements-matrix.md` (shall statements → modules → tests).

