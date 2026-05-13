# Phase 1 Kickoff — Foundation

> **For Claude Code**: 이 문서는 Phase 1의 작업 명세다. 모든 작업은 `CLAUDE.md`를 우선한다. 충돌 시 사용자에게 확인 후 진행한다.

---

## 1. Phase 1 Goal

VR로 진입한 평가자가:
1. Calibration 강제 검증을 통과하고
2. Wrist UI를 손목에서 확인할 수 있으며
3. 더미 차량 앞에 정상 비례로 서있을 수 있는

**최소 골격**을 구축한다.

Liftgate 인터랙션, Zone, Mode 선택 등은 **Phase 2 이후**에서 다룬다.

---

## 2. Pre-conditions

작업 시작 전 다음이 충족되어야 한다. 누락 시 사용자에게 보고 후 대기.

- [ ] UE 5.5.2 프로젝트 `LiftgateStudy` 생성 완료 (Blank, Blueprint 기반)
- [ ] Plugins 활성화:
  - Meta XR Plugin (v85.0+)
  - Meta XR Interaction SDK (v85.0+)
  - OpenXR
  - OpenXRHandTracking
- [ ] Project Settings → Meta XR → Hand Tracking Support: **Controllers and Hands**
- [ ] Project Settings → Engine → Rendering → Default RHI: **DirectX 12** (또는 Vulkan)
- [ ] ISDK Sample 프로젝트 별도 다운로드 (hand rig prefab 참조용)
- [ ] Quest 3 + Quest Link 연결 동작 확인

---

## 3. Folder Structure (작업 후 상태)

```
LiftgateStudy/
├── CLAUDE.md
├── README.md
├── docs/
│   ├── phase_kickoff/
│   │   └── Phase1_Kickoff.md          ← 이 문서
│   ├── decisions/
│   │   ├── ADR_001_isdk_grab_layer.md
│   │   ├── ADR_002_modes_free_toggle.md
│   │   └── ADR_003_no_logging.md
│   └── lessons_learned/
│       └── (Phase 1 진행 중 발견 사항 기록)
└── Content/
    ├── Maps/
    │   ├── L_Main.umap                ← Persistent Level
    │   └── L_Vehicle_Dummy.umap       ← 더미 차량 sublevel
    ├── Blueprints/
    │   ├── BP_VRPawn.uasset
    │   └── BP_VehicleInfo.uasset      ← 골격만, 상세는 Phase 2
    ├── UI/
    │   ├── WBP_CalibrationCheck.uasset
    │   └── WBP_WristPanel.uasset
    ├── Materials/
    │   ├── M_FloorGrid.uasset
    │   └── M_UI_Base.uasset
    └── (ISDK 관련 자산은 plugin에서 자동 제공)
```

---

## 4. Work Items

### W1.1 — Project 초기 셋업

**작업**:
1. Default GameMode 생성: `BP_LiftgateStudyGameMode`
   - Default Pawn: `BP_VRPawn` (W1.2에서 생성)
2. Project Settings → Maps & Modes:
   - Default GameMode: `BP_LiftgateStudyGameMode`
   - Editor Startup Map: `L_Main`
   - Game Default Map: `L_Main`
3. Input Mapping Context 등록:
   - `IMC_IsdkHand` (ISDK 제공) 을 `Add Mapping Context`로 BeginPlay 시 적용

**검증**:
- 프로젝트 실행 → 에러 없이 빈 Level 진입
- VR Preview 진입 가능

---

### W1.2 — BP_VRPawn

**작업**:
1. `Pawn` 기반 BP 생성, 이름 `BP_VRPawn`
2. Component 구조:
   ```
   BP_VRPawn (Pawn)
   ├── DefaultSceneRoot
   ├── VROrigin (SceneComponent)        ← Floor 기준 origin
   │   ├── Camera (CameraComponent)     ← HMD
   │   ├── HandRig_Left (ISDK)          ← Sample에서 가져옴
   │   └── HandRig_Right (ISDK)         ← Sample에서 가져옴
   └── (필요 시 추가 컴포넌트)
   ```
3. BeginPlay에서:
   - `Set Tracking Origin → Floor Level` 호출 (필수, R1)
   - `Add Mapping Context (IMC_IsdkHand)` 호출
4. ISDK Hand Rig prefab은 **Sample에서 복사**해서 사용 (자체 작성 금지, R5)

**검증**:
- VR Preview → HMD 위치 = Camera 위치 일치
- 손이 트래킹되어 화면에 보임
- 손목 회전이 실제 손 회전 따라감

---

### W1.3 — L_Main (Persistent Level)

**작업**:
1. `L_Main` Level 생성
2. 배치:
   - `BP_VRPawn` (World Z=0 위치)
   - Directional Light + Sky Sphere (기본 조명)
   - Floor reference grid:
     - Plane mesh, 10m × 10m, Z=0
     - `M_FloorGrid` 머티리얼 (10cm 간격 grid 텍스처 또는 procedural)
     - 평가자가 발 밑 위치를 시각 확인할 수 있어야 함
3. `L_Vehicle_Dummy`를 streaming sublevel로 등록 (Level → Levels 패널)
   - 기본 상태: Loaded + Visible

**검증**:
- L_Main 시작 시 floor grid가 발 밑에 보임
- 더미 차량 박스가 옆에 위치

---

### W1.4 — WBP_CalibrationCheck

**작업**:
1. **3D Widget** (UMG Widget, World-space로 사용)
2. UI 구성:
   ```
   ┌────────────────────────────────────┐
   │      CALIBRATION CHECK             │
   │                                    │
   │   Stand straight, look forward     │
   │                                    │
   │   HMD Height:    [---] mm          │
   │   Floor Z:       [---] mm          │
   │                                    │
   │   Status: [CHECKING / PASS / FAIL] │
   │                                    │
   │   [Pinch to confirm]               │
   └────────────────────────────────────┘
   ```
3. 검증 로직:
   - 매 frame Camera world location의 Z 측정 → `HMDHeight_mm`
   - HMD 아래 방향 ray cast → floor와의 차이 `FloorZ_mm`
   - Pass 조건:
     - `HMDHeight_mm` ∈ [`HMDHeightMin_mm`, `HMDHeightMax_mm`] (기본 1400~2000)
     - `|FloorZ_mm|` ≤ `FloorZTolerance_mm` (기본 5mm)
   - Fail 시 안내 메시지:
     - HMD < 1400: "You may be sitting. Please stand."
     - HMD > 2000: "Calibration may be off. Re-do Quest boundary setup."
     - Floor 초과: "Floor not detected at Z=0. Re-do boundary."
4. 평가자가 Pinch (ISDK pinch action)로 확인 → 평가 모드 진입

**Magic Number는 모두 BP variable로 EditAnywhere 노출**.

**검증**:
- VR 진입 시 widget이 평가자 정면 1.5m 거리에 표시
- HMD Height가 실시간 갱신
- 잘못된 자세 (앉기, 점프) 시 Fail 표시
- 정상 자세에서 Pinch → 평가 진입

---

### W1.5 — WBP_WristPanel

**작업**:
1. **UMG Widget**, 좌측 손 wrist socket에 부착
2. Visibility 제어:
   - 왼손 손바닥이 위를 향하면 (Up vector와 World Up의 dot product > 0.5) → Visible
   - 아니면 Hidden
3. Phase 1 표시 항목 (최소):
   ```
   ┌──────────────────────┐
   │  CALIBRATION         │
   │  HMD: ---- mm        │
   │  Floor: ✓ / ✗        │
   │                      │
   │  [Recalibrate]       │  ← 버튼, 누르면 W1.4 widget 다시 표시
   └──────────────────────┘
   ```
4. Recalibrate 버튼은 ISDK poke 또는 pinch interaction으로 동작

**검증**:
- 손바닥 위로 향할 때만 Wrist UI 보임
- HMD Height 실시간 갱신
- Recalibrate 버튼 누르면 calibration widget 재진입

---

### W1.6 — L_Vehicle_Dummy (더미 차량 Sublevel)

**작업**:
1. `L_Vehicle_Dummy` Level 생성
2. 단순 박스 mesh 배치:
   - 크기: 4500mm × 1800mm × 1600mm (대략 mid-size SUV)
   - 위치: 박스 바닥이 Z=0 (R4 준수)
   - 평가자로부터 약 1.5m 거리 후방에 배치
   - 머티리얼: 단색 (회색 또는 차량 색)
3. 박스 후면에 작은 marker mesh 1개 (Liftgate handle 위치 예상점)
   - Phase 2에서 BP_Liftgate로 대체될 위치 reference

**검증**:
- 평가자 위치에서 차량 박스가 정상 비례로 보임 (어깨 높이 정도)
- 차량 바닥과 floor grid가 정확히 일치 (틈 없음)

---

## 5. ADR 작성 (병행)

Phase 1 진행 중 다음 3개 ADR을 박제한다:

### ADR_001 — ISDK 기반 Hand Grab
- **Context**: Hand grab 인터랙션 구현 방식 선택
- **Decision**: Meta XR Interaction SDK의 `IsdkGrabbableComponent` 사용. 자체 pinch detection 작성 금지
- **Consequences**:
  - + 안정적 grab 판정, tracking confidence 자동 처리
  - + 평가자 간 일관성 보장
  - − SDK 버전 의존성 발생
  - − SDK 업그레이드 시 호환성 검증 필요

### ADR_002 — Mode는 자유 토글
- **Context**: Liftgate mode 4종을 차량 속성으로 고정할지, 평가자가 자유롭게 토글할지
- **Decision**: 자유 토글. 차량 metadata에 SupportedModes 없음
- **Consequences**:
  - + 같은 차량을 4가지 mode로 비교 평가 가능
  - + 차량 메타데이터 단순화
  - − 실차에 없는 mode도 체험 가능 (실차 충실도 ↓)

### ADR_003 — 평가 데이터 로깅 없음
- **Context**: 평가 시 (각도, 시간, zone hit 등) 로깅 필요 여부
- **Decision**: 일체 로깅하지 않음. 평가는 평가자의 주관적 체험에만 의존
- **Consequences**:
  - + 구현 단순화
  - + 평가자 부담 감소
  - − 사후 분석 불가
  - − 평가자 간 정량 비교 불가
  - → 향후 로깅 필요 시 별도 Phase로 분리

---

## 6. Deliverables

Phase 1 완료 시점 산출물:

1. **UE 프로젝트 `LiftgateStudy/`** — 위 폴더 구조대로
2. **Working build**: PIE (Play In Editor) 또는 VR Preview에서:
   - VR 진입 → Calibration Check → Pinch 확인 → 더미 차량 앞에 위치
   - Wrist UI 손바닥 방향 따라 visible/hidden
   - Recalibrate 동작
3. **README.md** — Phase 1 완료 상태, 셋업 방법, 알려진 이슈 명시
4. **3개 ADR** — `docs/decisions/` 폴더에 박제

---

## 7. Verification Criteria (사용자 검증 항목)

Phase 1 완료 보고 시 사용자가 직접 확인할 항목:

- [ ] Quest 3로 PCVR 진입 시 floor가 발 밑에 정확히 위치
- [ ] HMD Height 값이 평가자 실제 키와 일치 (±5cm)
- [ ] Floor Z difference가 0±5mm
- [ ] Calibration Fail 시 안내 메시지 적절히 표시
- [ ] Pinch로 calibration 통과 동작
- [ ] Wrist UI가 손바닥 방향 따라 visible/hidden 전환
- [ ] Recalibrate 버튼 동작
- [ ] 더미 차량 박스가 정상 비례 (어깨 높이쯤)
- [ ] 차량 박스 바닥과 floor grid가 일치 (틈 없음)
- [ ] 모든 Magic Number가 BP variable로 노출되어 튜닝 가능

---

## 8. Out of Scope (Phase 2로 명시 미룸)

다음은 Phase 1에서 **다루지 않는다**:

- 실제 Liftgate mesh / 인터랙션 / 회전
- Zone visualizer (Red/Green/Yellow)
- Height ruler (차량 옆 키 표시 막대)
- Mode 선택 UI / 4가지 mode 구현
- Power button (BP_Liftgate child component)
- 실제 차량 mesh import (Datasmith)
- CAD origin alignment 스크립트
- 차량 swap UX (여러 차량 간 전환)
- Standalone Quest 빌드

위 항목 중 Phase 1에서 임의 진행 금지. 필요 시 사용자에게 확인.

---

## 9. 확정된 Decision (이 문서에서 변경 금지)

| 항목 | 확정 |
|---|---|
| VR 디바이스 | Quest 3 + Hand Tracking, PCVR |
| SDK | Meta XR Interaction SDK |
| Hand grab | ISDK 기반 (자체 구현 금지) |
| Modes | 자유 토글 (4가지) |
| Zone | 차량별 커스텀 (Phase 3) |
| Calibration | 강제 검증 (skip 불가) |
| 평가 로깅 | 없음 |
| Power button 위치 | BP_Liftgate child component, Level에서 평가자가 수동 배치 |
| Level 구조 | Persistent + streaming sublevel |
| UI | Wrist Panel (좌측 손목) 통합 |

---

## 10. 작업 순서 (권장)

블로커 위험을 줄이는 순서:

1. **W1.1** (셋업) → 빈 VR 진입 가능한지 확인
2. **W1.2** (Pawn) → 손이 트래킹되는지 확인
3. **W1.3** (Level) → 발 밑 floor 확인, 비례감 확인
4. **W1.6** (더미 차량) → 차량 비례감 확인
5. **W1.4** (Calibration) → 검증 로직 동작
6. **W1.5** (Wrist UI) → 손목 UI 동작 + Recalibrate

---

## 11. 막힐 시 디버그 우선순위

| 증상 | 1차 확인 |
|---|---|
| Hand tracking 안 뜸 | Project Settings → Meta XR → Hand Tracking Support 확인 |
| Floor가 어긋남 | `Set Tracking Origin` 호출 여부, Quest Guardian 재설정 |
| ISDK Hand rig가 손에 안 붙음 | Sample에서 prefab 다시 복사 |
| Wrist UI가 안 보임 | Hand socket 이름 확인 (Quest hand mesh 표준명) |
| Calibration이 항상 Fail | HMD Height 값 디버그 출력, ray cast trace 시각화 |
| PCVR 연결 안 됨 | Quest Link 앱 재시작, USB-C 케이블 점검 |

해결 안 되는 이슈는 `docs/lessons_learned/` 에 박제.

---

## 12. Phase 1 완료 후 회고

Phase 2 진입 전 다음 항목 회고:

1. ISDK 셋업에서 시간 가장 많이 든 부분
2. Quest 3 specific 함정 (있다면 `lessons_learned/`에 박제)
3. Phase 2 진입 전 보완 필요 사항
4. 본 문서 (CLAUDE.md 포함) 수정 필요 항목

회고 결과는 `docs/phase_kickoff/Phase2_Kickoff.md` 작성 시 반영.

---

## 13. Claude Code 작업 원칙

- 사용자에게 결정을 요구할 때 **선택지를 명확히** 제시 (A / B / C)
- 본 문서에 명시된 확정 사항은 **재확인 없이** 진행
- 본 문서에 없는 결정 사항은 **반드시 사용자 확인** 후 진행
- 작업 중 발견된 함정 / 학습 / 우회는 `lessons_learned/`에 즉시 박제
- 모든 commit 메시지는 영어, format: `phase1: <work_item> <action>` (예: `phase1: W1.4 add calibration ray cast logic`)
- 한 work item 완료 시 사용자에게 검증 요청
