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

- [ ] UE 5.5.4 프로젝트 `LiftgateStudy` 생성 완료 (Path B 셋업: `Docs/Project_Setup_Prompt.md`)
- [ ] Plugins 활성화:
  - Meta XR Plugin (v78)
  - Meta XR Interaction SDK (v78)
  - OculusInteractionSamples (ADR-004 → IsdkSamplePawn 사용)
  - OpenXR
  - OpenXRHandTracking
- [ ] Project Settings → Meta XR → Hand Tracking Support: **Controllers and Hands**
- [ ] Project Settings → Engine → Rendering → Default RHI: **DirectX 12** (또는 Vulkan)
- [ ] Quest 3 + Quest Link 연결 동작 확인
- [ ] **Visual Studio 2022 + `Game development with C++` workload 설치** (ADR-005, hard requirement)
- [ ] UE 5.5.4 와 동일한 toolchain (VS 2022, Windows SDK) — plugin 컴파일러 일치

---

## 3. Folder Structure (작업 후 상태)

```
LiftgateStudy/
├── CLAUDE.md
├── README.md
├── LiftGateStudy.uproject            ← Modules 항목 자동 추가됨 (ADR-005)
├── Docs/
│   ├── Project_Setup_Prompt.md
│   ├── phase_kickoff/
│   │   ├── Phase1_Kickoff.md         ← 이 문서
│   │   └── Phase1_Checklist.md
│   ├── decisions/
│   │   ├── ADR_001_isdk_grab_layer.md
│   │   ├── ADR_002_modes_free_toggle.md
│   │   ├── ADR_003_no_logging.md
│   │   ├── ADR_004_bp_vrpawn_extends_isdk_sample_pawn.md
│   │   └── ADR_005_cpp_adoption.md
│   └── lessons_learned/
├── Source/                            ← ADR-005, C++ module
│   └── LiftGateStudy/
│       ├── LiftGateStudy.Build.cs
│       ├── LiftGateStudy.h / .cpp
│       ├── Public/
│       │   ├── LiftgateStudyGameMode.h
│       │   ├── Calibration/
│       │   │   ├── CalibrationTypes.h            (ECalibrationStatus)
│       │   │   ├── CalibrationCheckWidget.h
│       │   │   └── CalibrationGateActor.h
│       │   └── UI/
│       │       └── WristPanelWidget.h
│       └── Private/
│           └── (대응 .cpp)
└── Content/
    ├── Maps/
    │   ├── L_Main.umap                ← Persistent Level
    │   └── L_Vehicle_Dummy.umap       ← 더미 차량 sublevel
    ├── Blueprints/
    │   ├── BP_LiftgateStudyGameMode.uasset   ← C++ ALiftgateStudyGameMode 의 child
    │   ├── BP_VRPawn.uasset                  ← IsdkSamplePawn (BP) 의 child (ADR-004)
    │   ├── BP_CalibrationGate.uasset         ← C++ ACalibrationGateActor 의 child
    │   └── BP_VehicleInfo.uasset             ← 골격만, 상세는 Phase 2
    ├── UI/
    │   ├── WBP_CalibrationCheck.uasset       ← C++ UCalibrationCheckWidget 의 child
    │   └── WBP_WristPanel.uasset             ← C++ UWristPanelWidget 의 child
    ├── Input/
    │   └── IMC_IsdkHand.uasset
    ├── Materials/
    │   ├── M_FloorGrid.uasset
    │   └── M_UI_Base.uasset
    └── (ISDK 관련 자산은 plugin에서 자동 제공)
```

---

## 4. Work Items

### W1.1 — Project 초기 셋업 + C++ Module Bootstrap

> ADR-005 적용: C++ module 부트스트랩이 본 work item 의 첫 단계.

**작업**:
1. **C++ Module 부트스트랩** (최초 1회):
   - UE Editor → Tools → **New C++ Class** → `GameModeBase` parent → 이름 `LiftgateStudyGameMode`
   - 첫 C++ 클래스 생성 시 UE 가 자동으로:
     - `Source/LiftGateStudy/` 디렉토리 생성
     - `LiftGateStudy.Build.cs`, `LiftGateStudy.h/.cpp` 생성
     - `.uproject` 에 `Modules` 항목 추가
   - VS 2022 가 자동으로 sln 생성 후 컴파일 → 성공 확인
2. **`.Build.cs` 의존성 추가**:
   ```csharp
   PublicDependencyModuleNames.AddRange(new string[] {
       "Core", "CoreUObject", "Engine", "InputCore",
       "EnhancedInput", "UMG", "Slate", "SlateCore",
       "HeadMountedDisplay",
   });
   ```
3. **GameMode C++ 구현** (`ALiftgateStudyGameMode`):
   - Constructor 에서 `DefaultPawnClass = BP_VRPawn` (W1.2 의 BP child 지정은 BP 에서)
4. **BP child 생성**: `BP_LiftgateStudyGameMode` (parent = `ALiftgateStudyGameMode`)
   - Default Pawn Class: `BP_VRPawn` (W1.2 에서 생성)
5. **Project Settings → Maps & Modes**:
   - Default GameMode: `BP_LiftgateStudyGameMode`
   - Editor Startup Map: `L_Main`
   - Game Default Map: `L_Main`
6. **Input Mapping Context 등록**:
   - `IMC_IsdkHand` (ISDK 제공) 을 BP_VRPawn BeginPlay 에서 `Add Mapping Context` 호출

**검증**:
- C++ 모듈 컴파일 성공 (VS 2022 빌드 OK)
- `.uproject` 에 `"Modules": [{"Name": "LiftGateStudy", ...}]` 추가 확인
- 프로젝트 실행 → 에러 없이 빈 Level 진입
- VR Preview 진입 가능

---

### W1.2 — BP_VRPawn (Child of IsdkSamplePawn)

> ADR-004 적용: `BP_VRPawn` 은 `IsdkSamplePawn` 의 child BP. Hand rig migrate 단계는 제거됨.

**작업**:
1. Content/Blueprints/ → 우클릭 → Blueprint Class → **Pick Parent Class**:
   - "All Classes" 검색 → **`IsdkSamplePawn`** 선택
   - 이름: `BP_VRPawn`
2. Component 구조 (parent 상속분 + 추가):
   ```
   BP_VRPawn (IsdkSamplePawn)
   ├── (parent에서 상속: HandRig_Left, HandRig_Right, Camera 등 — 직접 추가 금지)
   └── WristUIAnchor (WidgetComponent, W1.5에서 활용)
       └── parent: 좌측 hand mesh wrist socket
   ```
3. BeginPlay override:
   - `Set Tracking Origin → Floor Level` 호출 (필수, R1) — parent 호출 여부와 무관하게 명시
   - parent BeginPlay 호출 (Add Call to Parent Function)
   - 필요 시 `Add Mapping Context` 호출 (parent 가 이미 ISDK IMC 등록한다면 생략)
4. ISDK Hand Rig는 parent (`IsdkSamplePawn`) 에서 이미 제공 — **자체 hand rig 작성 / migrate 금지** (R5, ADR-004)

**검증** (ADR-004 §Verification):
- VR Preview → HMD 위치 = Camera 위치 일치
- 양손 트래킹 정상, 손목 회전 / 손가락 굴곡 실시간 반영
- Camera Z (mm) 가 평가자 실제 키 ± 5cm
- Quest Guardian 재설정 후에도 floor 가 발 밑에 정확
- 위 검증 실패 시 ADR-004 Option C (Migrate) 로 전환 검토

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

### W1.4 — Calibration (C++ Logic + WBP Layout + BP_CalibrationGate)

> ADR-005 적용. 세 가지 산출물:
> - **C++**: `ECalibrationStatus` enum + `UCalibrationCheckWidget` + `ACalibrationGateActor`
> - **BP**: `WBP_CalibrationCheck` (C++ widget 의 child, UMG layout 만)
> - **BP**: `BP_CalibrationGate` (C++ actor 의 child, WidgetComponent 설정)

**작업**:

#### W1.4.A — C++: ECalibrationStatus
- `Source/LiftGateStudy/Public/Calibration/CalibrationTypes.h`
- `UENUM(BlueprintType)` 으로 `ECalibrationStatus { Checking, Pass, Fail }` 정의
- (선택) `FCalibrationFailReason` 텍스트 enum

#### W1.4.B — C++: UCalibrationCheckWidget
- `Source/LiftGateStudy/Public/Calibration/CalibrationCheckWidget.h/.cpp`
- Base: `UUserWidget`, `UCLASS(Abstract, BlueprintType, Blueprintable)`
- UPROPERTY (Category="Calibration", EditAnywhere, BlueprintReadWrite):
  - `float HMDHeightMin_mm = 1400.f;`
  - `float HMDHeightMax_mm = 2000.f;`
  - `float FloorZTolerance_mm = 5.f;`
  - `float FloorRayLength_cm = 300.f;`
- UPROPERTY (Category="Calibration", BlueprintReadOnly):
  - `float HMDHeight_mm;`
  - `float FloorZ_mm;`
  - `ECalibrationStatus Status;`
  - `FText FailReason;`
- 함수:
  - `virtual void NativeTick(...) override` — 매 frame 측정 / 평가
  - `void EvaluateOnce()` — HMD Z 측정 + ray cast + 판정 + 상태 갱신
  - `void RequestComplete()` — Status == Pass 일 때만 OnCalibrationComplete 발행
- Event Dispatcher:
  - `DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCalibrationComplete);`
  - `UPROPERTY(BlueprintAssignable) FOnCalibrationComplete OnCalibrationComplete;`
- Fail 메시지 분기 (한국어 주석 / 영어 UI 텍스트, CLAUDE.md §8):
  - HMD < `HMDHeightMin_mm`: "You may be sitting. Please stand."
  - HMD > `HMDHeightMax_mm`: "Calibration may be off. Re-do Quest boundary setup."
  - `|FloorZ_mm| > FloorZTolerance_mm` 또는 hit 없음: "Floor not detected at Z=0. Re-do boundary."

#### W1.4.C — C++: ACalibrationGateActor
- `Source/LiftGateStudy/Public/Calibration/CalibrationGateActor.h/.cpp`
- Base: `AActor`, `UCLASS(Blueprintable)`
- Components:
  - `UWidgetComponent* CalibrationWidgetComp` (root child)
- UPROPERTY (Category="Calibration", EditAnywhere):
  - `TSubclassOf<UCalibrationCheckWidget> CalibrationWidgetClass;` — BP_CalibrationGate 에서 WBP_CalibrationCheck 지정
- `BeginPlay()`:
  - WidgetComponent 의 widget instance 가져옴
  - `OnCalibrationComplete` 에 self 의 `HandleComplete()` 바인딩
- `HandleComplete()`:
  - widget 숨김 또는 self `Destroy()`
- Confirm Button OnClicked binding (ADR-007, R8) — WBP_CalibrationCheck 의 Button 이 ISDK Poke Interactor 로 눌리면 `HandlePinchInput()` (역사적 이름, 실제로는 confirm) 호출 → `RequestComplete()` (Status=Pass 일 때만)

#### W1.4.D — BP: WBP_CalibrationCheck (layout)
- Parent: `UCalibrationCheckWidget`
- UMG Designer 로 다음 layout:
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
  │   [ Confirm ]   (button)           │
  └────────────────────────────────────┘
  ```
- Text 는 영어 (PQDQ 표준, CLAUDE.md §8). Bind 함수로 C++ UPROPERTY 참조.

#### W1.4.E — BP: BP_CalibrationGate (host actor)
- Parent: `ACalibrationGateActor`
- WidgetComponent default:
  - Widget Class: `WBP_CalibrationCheck`
  - Space: World
  - Draw Size: 400×300
- L_Main 에 1개 배치 (W1.3 에서 위치 지정)

**검증**:
- C++ 모듈 컴파일 성공
- VR 진입 시 widget 이 평가자 정면 1.5m 거리에 표시
- HMD Height 가 실시간 갱신
- 잘못된 자세 (앉기, 점프) 시 Fail 표시 + 메시지 분기 정확
- 정상 자세에서 Confirm 버튼 poke → `OnCalibrationComplete` 발행 → BP_CalibrationGate Destroy
- UPROPERTY 가 BP child 에서 default 조정 가능

---

### W1.5 — Wrist Panel (C++ Logic + WBP Layout)

> ADR-005 적용. 두 가지 산출물:
> - **C++**: `UWristPanelWidget` (visibility 판정 + 데이터 바인딩)
> - **BP**: `WBP_WristPanel` (UMG layout, parent = `UWristPanelWidget`)
> - host: `BP_VRPawn` 의 `WristUIAnchor` WidgetComponent (ADR-004 W1.2 에서 추가됨)

**작업**:

#### W1.5.A — C++: UWristPanelWidget
- `Source/LiftGateStudy/Public/UI/WristPanelWidget.h/.cpp`
- Base: `UUserWidget`, `UCLASS(Abstract, BlueprintType, Blueprintable)`
- UPROPERTY (Category="UI", EditAnywhere, BlueprintReadWrite):
  - `float WristVisibleDotThreshold = 0.5f;`
- UPROPERTY (Category="UI", BlueprintReadOnly):
  - `float HMDHeight_mm;`
  - `bool bFloorOK;`
- 함수:
  - `virtual void NativeTick(...) override`:
    - 좌측 hand mesh 의 Up vector vs World Up dot product 계산
    - 임계 이상이면 Visible, 아니면 Collapsed
    - HMD / Floor 상태를 Calibration widget 또는 GameState 에서 읽어 갱신
  - `UFUNCTION(BlueprintCallable) void OnRecalibrateClicked()` — calibration 재진입 트리거
- Event Dispatcher:
  - `DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRecalibrateRequested);`
  - `UPROPERTY(BlueprintAssignable) FOnRecalibrateRequested OnRecalibrateRequested;`

#### W1.5.B — BP: WBP_WristPanel (layout)
- Parent: `UWristPanelWidget`
- UMG Designer:
  ```
  ┌──────────────────────┐
  │  CALIBRATION         │
  │  HMD: ---- mm        │
  │  Floor: ✓ / ✗        │
  │                      │
  │  [Recalibrate]       │
  └──────────────────────┘
  ```
- Recalibrate 버튼 OnClicked → `OnRecalibrateClicked()` 호출
- 버튼은 **ISDK Poke Interactor** 로 동작 (R8 / ADR-007)

#### W1.5.C — BP_VRPawn 에서 binding
- `WristUIAnchor` WidgetComponent (ADR-004 W1.2 에서 추가됨) 의 Widget Class = `WBP_WristPanel`
- BeginPlay 에서 widget instance 의 `OnRecalibrateRequested` 에 calibration gate spawn 함수 바인딩

**검증**:
- C++ 모듈 컴파일 성공
- 손바닥 위로 향할 때만 Wrist UI 보임 (dot > threshold)
- 손등 / 옆 방향이면 hidden
- HMD Height 실시간 갱신
- Recalibrate 버튼 누르면 calibration widget 재진입
- UPROPERTY threshold 가 BP child 에서 default 조정 가능

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

## 5. 관련 ADR

본 Phase 의 결정 근거가 되는 ADR (모두 `Docs/decisions/` 에 박제됨):

| ADR | 주제 | 본 Phase 영향 |
|---|---|---|
| ADR-001 | ISDK 기반 Hand Grab | Phase 2 grab interaction (Phase 1 직접 사용 없음) |
| ADR-002 | Mode 자유 토글 | Phase 1 범위 외, 차량 metadata 단순화 |
| ADR-003 | 평가 데이터 로깅 없음 | 모든 W 에서 파일 출력 금지 |
| ADR-004 | BP_VRPawn = IsdkSamplePawn child | W1.2 의 hand rig migrate 제거 |
| ADR-005 | C++ 로직 + BP 레이아웃 hybrid | W1.1 module bootstrap, W1.4 / W1.5 C++ widget base |
| ADR-007 | Button poke 기반 confirmation | W1.4 Confirm 버튼, W1.5 Recalibrate 버튼 — gesture / pinch 금지 |

---

## 6. Deliverables

Phase 1 완료 시점 산출물:

1. **UE 프로젝트 `LiftgateStudy/`** — 위 폴더 구조대로 (Source/ + Content/ + Docs/)
2. **컴파일되는 C++ 모듈** (`LiftGateStudy`):
   - `ALiftgateStudyGameMode`
   - `UCalibrationCheckWidget`, `ACalibrationGateActor`, `ECalibrationStatus`
   - `UWristPanelWidget`
3. **Working build**: PIE (Play In Editor) 또는 VR Preview 에서:
   - VR 진입 → Calibration Check → Confirm 버튼 poke → 더미 차량 앞에 위치
   - Wrist UI 손바닥 방향 따라 visible/hidden
   - Recalibrate 동작
4. **README.md** — Phase 1 완료 상태, 셋업 방법, 알려진 이슈 명시
5. **ADR 5종** — `Docs/decisions/` 폴더에 박제 (ADR-001~005)

---

## 7. Verification Criteria (사용자 검증 항목)

Phase 1 완료 보고 시 사용자가 직접 확인할 항목:

- [ ] Quest 3로 PCVR 진입 시 floor가 발 밑에 정확히 위치
- [ ] HMD Height 값이 평가자 실제 키와 일치 (±5cm)
- [ ] Floor Z difference가 0±5mm
- [ ] Calibration Fail 시 안내 메시지 적절히 표시
- [ ] Confirm 버튼 poke 로 calibration 통과 동작 (Phase 1 검증 중에는 keyboard Space 임시 사용 허용)
- [ ] Wrist UI가 손바닥 방향 따라 visible/hidden 전환
- [ ] Recalibrate 버튼 동작
- [ ] 더미 차량 박스가 정상 비례 (어깨 높이쯤)
- [ ] 차량 박스 바닥과 floor grid가 일치 (틈 없음)
- [ ] 모든 Magic Number 가 C++ UPROPERTY 로 노출되어 BP child 에서 튜닝 가능 (ADR-005)
- [ ] C++ 모듈 `LiftGateStudy` 가 VS 2022 에서 컴파일 통과
- [ ] Live Coding 또는 Editor restart 컴파일 정상

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
| SDK | Meta XR Interaction SDK v78 |
| VR Pawn | `BP_VRPawn` = `IsdkSamplePawn` 의 child (ADR-004) |
| Code 패턴 | C++ 로직 + BP layout hybrid (ADR-005) |
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

1. **W1.1** (셋업 + C++ module bootstrap) → 빈 VR 진입 가능 + 컴파일 OK 확인
2. **W1.2** (Pawn, ADR-004) → 손이 트래킹되는지 확인
3. **W1.3** (Level) → 발 밑 floor 확인, 비례감 확인
4. **W1.6** (더미 차량) → 차량 비례감 확인
5. **W1.4** (Calibration, C++ hybrid) → 검증 로직 동작
6. **W1.5** (Wrist UI, C++ hybrid) → 손목 UI 동작 + Recalibrate

각 W 완료 시 commit (`phase1: W1.x <action>`). C++ 변경은 별도 commit (`phase1: W1.x add UCalibrationCheckWidget skeleton` 등).

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
