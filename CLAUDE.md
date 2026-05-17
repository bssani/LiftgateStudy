# Liftgate Study — CLAUDE.md

> GMTCK PQDQ Liftgate evaluation tool in VR (Quest 3 + Hand Tracking).
> 본 문서는 프로젝트의 governance rule이다. 모든 작업은 이 문서를 우선한다.

---

## 1. Project Overview

Liftgate Study는 다양한 차량의 liftgate 개폐 동작을 4가지 모드로 VR에서 체험하며 **주관적 느낌**을 평가하는 도구다.

- **사용자**: PQDQ 평가자
- **목적**: 실차 없이 liftgate 개폐 인터랙션의 직관적 평가
- **평가 데이터 로깅: 없음** — 단순히 평가자의 직접 체험이 목적
- **평가 단위**: 한 번에 한 차량, 한 평가자

---

## 2. Tech Stack

| 항목 | 버전 / 선택 |
|---|---|
| Engine | Unreal Engine 5.5.4 |
| VR Device | Meta Quest 3 |
| Connection | PCVR (Quest Link / Air Link) |
| Hand Tracking | Quest hand tracking (controller 미사용) |
| SDK | Meta XR Plugin v78 + Meta XR Interaction SDK v78 |
| Fallback | OpenXR + OpenXRHandTracking (Meta XR 미설치 환경용) |
| Code | **C++ 로직 + BP 레이아웃 hybrid** (ADR-005). Toolchain: VS 2022 + Game dev C++ workload |
| IDE | Visual Studio 2022 (필수) — UE 5.5.4 동반 toolchain |

---

## 3. Critical Rules

### R1. Tracking Origin = Floor Level
앱 시작 시 `Set Tracking Origin → Floor Level` 명시 호출. **Local로 빠지면 calibration이 전부 깨진다.** 모든 검증과 표시 좌표는 floor 기준이다.

### R2. VR Pawn은 Persistent Level에서 유지
Level swap 시에도 VR Pawn을 재생성하지 않는다. **Level streaming 또는 Level Instance**만 사용. `OpenLevel`로 차량 전환 금지.

### R3. Recenter 보호
Quest 시스템 메뉴의 recenter를 평가 중 누르면 calibration이 깨진다. 앱 시작 시 1회 강제 검증 후, 평가 중 recenter 감지 시 calibration 재검증으로 진입.

### R4. 차량 origin = World Z=0
모든 차량 sublevel은 **차량 mesh의 바닥(타이어 접지면)이 World Z=0**에 정렬되어야 한다. Datasmith import 후 alignment 보정 필수. 차량 origin이 어긋난 채로 sublevel을 등록하지 않는다.

### R5. Hand grab은 ISDK 기반
모든 grab 인터랙션은 ISDK의 `IsdkGrabbableComponent` 기반으로 작성. 자체 pinch detection 작성 금지. SDK 업그레이드 시 호환성 검증 후 진행.

### R6. HUD (head-locked) UI 금지
모든 UI는 World-space로. Head-locked는 VR 멀미 유발 (ADR-008 — wrist UI 폐기).

### R7. 평가 데이터 로깅 금지
CSV, JSON, 파일 출력 일체 없음. 평가 결과는 평가자 머리 속에만 남는다. 로깅 필요 시 별도 phase로 분리.

### R8. Confirmation = Button Poke (No Gesture)
모든 confirmation / discrete 입력 (calibration pass, mode toggle, recalibrate 등) 은 **button poke 기반** (ADR-007). 자체 pose / gesture / pinch detection 작성 금지. ISDK pinch detection 은 **grab interaction** (R5 / ADR-001) 에만 사용. 버튼은 평가자가 닿을 수 있는 위치에 배치 (§8).

---

## 4. Naming Conventions

### Asset (BP / Material / Mesh / Level)

| 항목 | 규칙 | 예시 |
|---|---|---|
| Actor Blueprint | `BP_<Name>` | `BP_Liftgate` |
| Widget Blueprint | `WBP_<Name>` | `WBP_CalibrationCheck` |
| Material | `M_<Name>` | `M_ZoneTranslucent` |
| Static Mesh | `SM_<Name>` | `SM_LiftgateHandle` |
| Level | `L_<Name>` | `L_Vehicle_C1YC` |
| Function (BP / C++) | `PascalCase` | `BeginCalibration()` |
| Variable (BP / C++) | `PascalCase` + unit suffix | `HMDHeight_mm`, `ThresholdAngle_deg` |

### C++ (ADR-005)

| 항목 | 규칙 | 예시 |
|---|---|---|
| Actor | `A<Name>` | `ACalibrationGateActor` |
| Pawn | `A<Name>` | `AVRPawn` |
| Component | `U<Name>Component` | `UCalibrationComponent` |
| UserWidget | `U<Name>Widget` | `UCalibrationCheckWidget` |
| Enum (BlueprintType) | `E<Name>` | `ECalibrationStatus` |
| Struct (BlueprintType) | `F<Name>` | `FHeightZone` |
| BP child of C++ | `BP_<Name>` / `WBP_<Name>` | `BP_CalibrationGate` from `ACalibrationGateActor` |

- 차량명은 단축 코드 (`C1YC`, `Equinox`). 공백/하이픈 금지, 언더스코어 사용.
- 단위는 변수명에 명시: `_mm`, `_cm`, `_deg`, `_rad`, `_sec`.
- UPROPERTY / UFUNCTION 의 `Category` 는 §6 의 BP variable Category 와 동일 (`Calibration`, `Mode`, `Zone`, `UI`, `Debug`).

---

## 5. Phase Plan

| Phase | 목표 | 산출물 |
|---|---|---|
| **Phase 1 — Foundation** | VR 셋업 + Calibration 검증 + 더미 차량 골격 | `L_Main`, `BP_VRPawn`, `WBP_CalibrationCheck`, `BP_CalibrationGate` |
| **Phase 2 — Liftgate Core** | `BP_Liftgate` + ISDK grab + Manual-Assist 동작 | Liftgate 잡고 열림, 40° release 시 auto-complete |
| **Phase 3 — Modes + Zones** | 4가지 mode 전체 + Zone + Height ruler | World-space mode toggle widget, zone/ruler 표시 |
| **Phase 4 — Multi-vehicle + Polish** | CAD alignment + 차량 swap + 시각 피드백 | 차량 N대 swap 가능, 평가자 onboarding |

각 Phase 시작 시 `docs/phase_kickoff/PhaseN_Kickoff.md` 작성. Phase 진행 중 결정 변경은 `docs/decisions/ADR_NNN_<topic>.md`로 박제.

---

## 6. Coding Style

ADR-005 적용: 새 로직은 C++ base + BP layout hybrid. BP-only 작성은 `BP_VRPawn` (ADR-004) 같은 명시적 예외만 허용.

### C++ (Primary)
- 로직 (Tick, math, state machine, ISDK callback, ray cast, 시간 계산) 는 모두 C++ 에 둔다.
- 모든 tunable parameter 는 `UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="…")` 로 노출. BP child 에서 default 조정.
- Magic Number 금지. 위와 같이 UPROPERTY 로 노출하거나 `static constexpr` 로 정의.
- Tick 의존 최소화. `FTimerHandle` + `GetWorld()->GetTimerManager()` 우선.
- `UFUNCTION(BlueprintCallable)` 은 BP 에서 호출이 실제로 필요한 함수에만. 남발 금지.
- Event dispatcher 는 `DECLARE_DYNAMIC_MULTICAST_DELEGATE_*` + `UPROPERTY(BlueprintAssignable)` 로.
- 헤더는 `Public/`, 구현은 `Private/`. `#pragma once` 필수. 불필요한 include 금지 (forward declare 우선).
- ISDK / Meta XR API 직접 호출 시 plugin module 을 `.Build.cs` PrivateDependencyModuleNames 에 추가.

### Blueprint (Layout / Default 만)
- C++ base 가 있는 클래스는 반드시 그 base 의 child BP 로 생성 (parent class 변경 금지)
- BP graph 는 단순 connection / Designer-tunable default 만. 복잡한 로직 추가 시 C++ 로 이관
- 함수는 단일 책임. 노드 30개 넘어가면 C++ 분리 검토
- 변수는 Category 지정 (`Calibration`, `Mode`, `Zone`, `UI`, `Debug`) — C++ UPROPERTY 도 동일 Category
- Tick 에서 무거운 연산 금지

### Comments
- 모든 주석은 **한국어**.
- 변수명/함수명/Asset명/Module명은 **영어**.

### Magic Number 예시
```
ThresholdAngle_deg = 40.0      // Manual-Assist auto-open 분기 각도
MaxAngle_deg = 85.0            // Liftgate 완전 개방 각도
AutoOpenDuration_sec = 1.2     // Power-Auto / auto-complete 시간
PinchGrabThreshold = 0.8       // ISDK pinch detection 임계값
HMDHeightMin_mm = 1400         // Calibration 최소 허용
HMDHeightMax_mm = 2000         // Calibration 최대 허용
FloorZTolerance_mm = 5.0       // Floor 검증 허용 오차
```

---

## 7. Calibration Requirements

앱 진입 시 **강제 검증** (skip 불가):

1. Tracking Origin = Floor Level 설정
2. "Stand straight, look forward" 안내 (3초)
3. HMD Height 측정
   - `HMDHeightMin_mm` ~ `HMDHeightMax_mm` 범위 → OK
   - 범위 밖 → 안내 후 재시도
4. Floor ray cast (HMD 아래 방향, world down)
   - 거리 ±`FloorZTolerance_mm` → OK
   - 초과 → "Re-do Quest boundary" 안내
5. 모두 OK → 평가 진입

평가 진입 후 HMD Height / Floor Status 의 실시간 표시는 별도 phase 에서 (필요 시 World-space widget 으로) 재설계 — ADR-008 (Wrist UI 폐기) 참조.

---

## 8. UI Conventions

- **Primary UI**: World-space widget — 평가자 proximity (정면 30~60cm 또는 적절한 환경 anchor) 내에 배치 (ADR-008 — wrist UI 폐기)
- **Auxiliary UI**: World-space (차량 옆 height ruler 등)
- **HUD (head-locked) 금지**
- 텍스트는 영어 (PQDQ 표준)
- 모든 UI 는 단위 명시 (`mm`, `deg`)
- **Interaction**: button poke 기반 (R8 / ADR-007). 버튼은 평가자가 닿을 수 있는 위치에 배치:
  - World-space 버튼 = 평가자 정면 30~60cm 거리, 어깨~허리 높이 권장
  - 멀리 배치된 widget 의 confirm 버튼 → widget 을 평가자 가까이에 배치하거나 ray pointer (Phase 2 이후 검토)

---

## 9. Liftgate Modes (정의)

| Mode | 조작 방식 |
|---|---|
| **Manual-Full** | 0°~85° 전 구간 hand grab 필요. 손 놓으면 멈춤(또는 중력 닫힘) |
| **Manual-Assist** | Hand grab으로 올림. 40° 이상에서 손 놓으면 auto-complete |
| **Power-Auto** | 버튼 1회 누름 → 끝까지 자동 개방 |
| **Power-Hybrid** | 버튼 누름 → 일정 각도까지 자동 → 이후 hand grab 필요 |

Mode는 **자유 토글**. 차량 속성이 아니라 평가 비교용 옵션.

---

## 10. Out of Scope

Phase 1~4 범위 외 (Phase 5+ 또는 별도 프로젝트):
- 평가 데이터 로깅 / CSV 출력
- Door, Hood, Sunroof 등 다른 부품
- 햅틱 피드백 (Manus, bHaptics 등)
- Multi-user / 협업 모드
- Standalone Quest 빌드 (현재는 PCVR only)
- 차량 변형 평가 (색상, trim 등)
- VR 외 desktop fallback

---

## 11. Decision Pattern

중요 결정은 `docs/decisions/` 폴더에 ADR (Architecture Decision Record)로 박제.

```
docs/decisions/
├── ADR_001_isdk_grab_layer.md     # Hand grab은 ISDK 기반
├── ADR_002_modes_free_toggle.md   # Mode는 차량 속성이 아닌 자유 토글
├── ADR_003_no_logging.md          # 평가 데이터 로깅 없음
└── ...
```

ADR 형식: Context / Decision / Consequences / Date.

---

## 12. Versioning

- 현재 버전: **v0.5**
- 변경 시 minor bump (v0.1 → v0.2)
- 큰 구조 변경 (Phase 정의 변경 등)은 major bump
- 변경 이력은 본 문서 하단 `Change Log` 섹션에 기록

---

## Change Log

- **v0.1** (2026-05-13) — Initial draft. Phase 1~4 정의, Critical Rules R1~R7, Liftgate Modes 4종 확정.
- **v0.2** (2026-05-13) — Tech stack 버전 확정 (UE 5.5.4, ISDK v78). Path B 셋업 채택.
- **v0.3** (2026-05-14) — ADR-005 채택: C++ 로직 + BP 레이아웃 hybrid 전환. §2 Tech Stack, §4 Naming Conventions (C++ 추가), §6 Coding Style 갱신.
- **v0.4** (2026-05-15) — ADR-007 채택: Interaction modality = button poke (no gesture). R8 신규, §8 UI Conventions 보강 (proximity 배치 규칙).
- **v0.5** (2026-05-15) — ADR-008 채택: WristPanel 폐기. Primary UI = World-space widget. §5 Phase 1 산출물 / §6 Coding Style 일부 / §8 UI Conventions 갱신.
