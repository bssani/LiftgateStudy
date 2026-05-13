# Phase 1 — Work Item Checklist

> Phase1_Kickoff.md의 W1.1~W1.6를 **UE Editor에서 따라갈 수 있는 체크리스트**로 풀어둔 문서.
> 모든 항목은 CLAUDE.md (Critical Rules R1~R7, Naming Conventions, Coding Style) 를 준수해야 한다.
> 작업 순서는 Phase1_Kickoff.md §10 권장 순서 (W1.1 → W1.2 → W1.3 → W1.6 → W1.4 → W1.5).

---

## Pre-flight (작업 시작 전)

- [ ] Step A 완료 확인 (`Docs/Project_Setup_Prompt.md` §A.7)
- [ ] `.uproject` 실행 시 에러 없음
- [ ] Plugins 활성화: Meta XR / Meta XR Interaction SDK / OpenXR / OpenXRHandTracking
- [ ] Project Settings → Plugins → Meta XR → Hand Tracking Support = **Controllers and Hands**
- [ ] Quest 3 + Quest Link 연결, VR Preview 옵션 노출됨
- [ ] ISDK Sample (`5.5.4-v78` branch) 별도 clone 완료 (`C:\Reference\Unreal-InteractionSDK-Sample\`)
- [ ] 작업 중 Quest 시스템 메뉴 recenter 누르지 않기 (R3)

---

## W1.1 — Project 초기 셋업

### W1.1.1 — GameMode Blueprint 생성

- [ ] Content Browser → Content/Blueprints/ 폴더 생성 (없으면)
- [ ] 우클릭 → Blueprint Class → Parent: **GameModeBase**
- [ ] 이름: `BP_LiftgateStudyGameMode`
- [ ] 열기 → Class Defaults:
  - [ ] Default Pawn Class: `BP_VRPawn` (W1.2에서 생성 후 다시 와서 지정)
  - [ ] HUD Class: None
  - [ ] Player Controller Class: PlayerController (기본)
  - [ ] Game State Class: GameStateBase (기본)
- [ ] 컴파일 + 저장

### W1.1.2 — Project Settings 적용

- [ ] Edit → Project Settings → Maps & Modes:
  - [ ] Default GameMode: `BP_LiftgateStudyGameMode`
  - [ ] Editor Startup Map: `L_Main` (W1.3에서 생성 후 다시 와서 지정)
  - [ ] Game Default Map: `L_Main`
- [ ] Engine → Rendering → Default RHI: **DirectX 12** (또는 Vulkan)
- [ ] (확인) Plugins → Meta XR → Hand Tracking Support = **Controllers and Hands**

### W1.1.3 — Input Mapping Context

- [ ] Content/Input/ 폴더 생성
- [ ] `IMC_IsdkHand` 는 ISDK Sample 또는 Plugin Content 에서 migrate (자체 작성 금지, R5)
- [ ] BeginPlay 시 적용은 W1.2의 BP_VRPawn 에서

### 검증

- [ ] 프로젝트 실행 → 에러 없이 빈 Level 진입
- [ ] VR Preview 진입 가능 (까만 화면이어도 OK, 다음 W에서 Pawn 추가)

---

## W1.2 — BP_VRPawn (Child of IsdkSamplePawn — ADR-004)

> ADR-004 적용: parent를 `IsdkSamplePawn` 으로 둠. Hand rig migrate 단계 삭제.
> Parent BP 위치: `Plugins/OculusInteractionSamples/Content/Blueprints/IsdkSamplePawn.uasset`

### W1.2.1 — Child Blueprint 생성

- [ ] Content/Blueprints/ → 우클릭 → Blueprint Class → **All Classes** 검색창
- [ ] **`IsdkSamplePawn`** 선택 → 이름: `BP_VRPawn`
- [ ] (대안: Content Browser에서 `IsdkSamplePawn.uasset` 위에 우클릭 → "Create Child Blueprint Class")
- [ ] 열기 → Class Defaults:
  - [ ] Auto Possess Player: **Player 0**

### W1.2.2 — Component 구조 확인

- [ ] Parent 상속 component 확인 (직접 추가 / 삭제 금지):
  ```
  BP_VRPawn (IsdkSamplePawn)
  └── (parent: HandRig_Left, HandRig_Right, Camera, EnhancedInputComponent 등)
  ```
- [ ] Wrist UI attach point 추가 (W1.5에서 활용):
  - [ ] Add Component → **WidgetComponent**, 이름 `WristUIAnchor`
  - [ ] Parent socket: 좌측 hand mesh 의 wrist socket (ISDK Sample 의 hand mesh 표준 socket 이름 확인, 예: `wrist_l` 또는 `WristRoot_L`)
  - [ ] Widget Class: (W1.5에서 `WBP_WristPanel` 지정)
  - [ ] Draw Size: 200×150 (W1.5에서 조정)
  - [ ] Initial Visibility: Hidden (W1.5의 dot product 조건에서 토글)

### W1.2.3 — BeginPlay override

- [ ] Event BeginPlay 노드 추가 (My Blueprint → Functions → Override → BeginPlay)
- [ ] **Add Call to Parent Function** 노드 먼저 배치 (parent 초기화 보장)
- [ ] **Set Tracking Origin** node → `Floor Level` (필수, R1 — parent가 호출해도 명시)
- [ ] (parent가 ISDK IMC 등록 안 하면) Get Player Controller → Cast to PlayerController → **Add Mapping Context** (IMC_IsdkHand, Priority 0)
- [ ] (선택) Debug print: "BP_VRPawn BeginPlay OK, Floor Origin set"

### W1.2.4 — 변수 / Category

- [ ] 변수 생성 시 Category 지정 (CLAUDE.md §6):
  - `Calibration`, `Mode`, `Zone`, `UI`, `Debug`

### 검증 (ADR-004 §Verification)

- [ ] W1.1.1로 돌아가서 BP_LiftgateStudyGameMode 의 Default Pawn Class = `BP_VRPawn` 설정
- [ ] VR Preview → HMD 위치 = Camera 위치 일치 (헤드 움직임 따라감)
- [ ] 양손이 트래킹되어 화면에 visible
- [ ] 손목 회전이 실제 손 회전 따라감
- [ ] 손가락 굴곡 (pinch, point, fist) 정상 표시
- [ ] Camera world Z (mm) 가 평가자 실제 키 ± 5cm
- [ ] Quest Guardian 재설정 후에도 floor 가 발 밑에 정확
- [ ] 위 R1 검증 실패 시 → ADR-004 Option C (Migrate / duplicate) 로 전환 검토

---

## W1.3 — L_Main (Persistent Level)

### W1.3.1 — Level 생성

- [ ] File → New Level → Empty Level
- [ ] Save As → Content/Maps/L_Main
- [ ] Project Settings → Maps & Modes 에서 Editor Startup Map / Game Default Map 을 `L_Main` 으로 지정 (W1.1.2 보완)

### W1.3.2 — 기본 배치

- [ ] **BP_VRPawn** 1개를 World 에 배치, Location (0, 0, 0)
  - 주의: Pawn이 GameMode 의 Default Pawn 으로 spawn 되므로 Level 배치는 reference 용 (실제 player pawn 은 spawn)
  - 또는 Place from Class → BP_VRPawn 으로 명시 배치 + Auto Possess Player = Player 0
- [ ] **Directional Light** 1개 (강도 3.14 lux, Source Angle 0.5°)
- [ ] **Sky Atmosphere** + **Sky Light** + **Exponential Height Fog** (기본 lighting)
- [ ] **Sky Sphere** (선택, BP_Sky_Sphere)

### W1.3.3 — Floor Reference Grid

- [ ] Plane mesh 배치 (basic shape: Plane)
  - [ ] Scale: X=100, Y=100 (= 10m × 10m)
  - [ ] Location: Z=0
  - [ ] 회전: 기본 (XY 평면)
- [ ] `M_FloorGrid` 머티리얼 생성 (Content/Materials/):
  - [ ] 10cm 간격 grid (procedural: WorldPosition → frac → step, 또는 grid texture)
  - [ ] 시각 확인 용이하게 대비 명확
- [ ] Plane 에 `M_FloorGrid` 적용

### W1.3.4 — Sublevel 등록 (L_Vehicle_Dummy)

- [ ] W1.6 완료 후 Window → Levels 패널 열기
- [ ] Add Existing → `L_Vehicle_Dummy`
- [ ] Streaming Method: **Always Loaded** (Phase 1 에서는 단순화)
- [ ] R2 준수: VR Pawn 은 Persistent Level (L_Main) 에 있고 sublevel swap 시 재생성 금지

### 검증

- [ ] L_Main 시작 시 floor grid 가 발 밑에 보임
- [ ] grid 가 평평하고 10cm 간격으로 시각 확인 가능
- [ ] 더미 차량 (W1.6) 박스가 옆에 위치
- [ ] HMD 가 Z≈평가자 키 (mm) 위치에 표시 (Outliner 에서 Camera world location 확인)

---

## W1.6 — L_Vehicle_Dummy (더미 차량 Sublevel) — 권장 순서상 먼저

### W1.6.1 — Level 생성

- [ ] File → New Level → Empty Level
- [ ] Save As → Content/Maps/L_Vehicle_Dummy

### W1.6.2 — 박스 mesh 배치

- [ ] Place Actor → Basic → Cube
- [ ] Scale 조정으로 4500mm × 1800mm × 1600mm 비례 만들기
  - UE 기본 Cube 1 unit = 100cm = 1m. 즉 Scale (4.5, 1.8, 1.6) 이 4.5m × 1.8m × 1.6m
- [ ] Location:
  - [ ] **Z = 0.8m (=Scale.Z / 2)** — Cube 의 pivot 이 중앙이라 박스 바닥이 Z=0 에 닿게 (R4)
  - [ ] X, Y: 평가자 (BP_VRPawn 위치) 로부터 약 1.5m 후방
- [ ] R4 검증: 박스 바닥과 L_Main 의 floor grid 가 같은 Z 평면, 틈 없음

### W1.6.3 — 머티리얼

- [ ] 단색 머티리얼 (회색 또는 차량 색) — `M_VehicleDummy_Base` (선택)
- [ ] Cube 에 적용

### W1.6.4 — Liftgate Handle Marker

- [ ] 작은 마커 mesh (Sphere, Scale 0.05 = 5cm) 배치
- [ ] Location: 박스 후면 중앙, 지면에서 약 80~100cm 높이 (handle 예상점)
- [ ] Phase 2 에서 BP_Liftgate 로 대체될 reference

### 검증

- [ ] 평가자 위치 (BP_VRPawn) 에서 차량 박스가 정상 비례로 보임 (어깨 ~ 머리 높이 정도)
- [ ] 차량 바닥과 floor grid 가 정확히 일치 (틈 없음, R4)
- [ ] Liftgate handle marker 가 박스 후면에서 시각 확인 가능

---

## W1.4 — WBP_CalibrationCheck

### W1.4.1 — Widget Blueprint 생성

- [ ] Content/UI/ 폴더 생성
- [ ] 우클릭 → User Interface → Widget Blueprint → Parent: UserWidget
- [ ] 이름: `WBP_CalibrationCheck`
- [ ] **3D World-space 로 사용** (HUD 금지, R6)

### W1.4.2 — UI 레이아웃

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

- [ ] Vertical Box 안에 TextBlock 들 배치
- [ ] 텍스트는 영어 (PQDQ 표준, CLAUDE.md §8)
- [ ] 단위 명시 (`mm`, `deg`) — CLAUDE.md §8

### W1.4.3 — Variables (모두 EditAnywhere, Category=Calibration)

- [ ] `HMDHeightMin_mm` (float, default **1400**)
- [ ] `HMDHeightMax_mm` (float, default **2000**)
- [ ] `FloorZTolerance_mm` (float, default **5.0**)
- [ ] `HMDHeight_mm` (float, runtime)
- [ ] `FloorZ_mm` (float, runtime)
- [ ] `Status` (enum or text: CHECKING / PASS / FAIL)

### W1.4.4 — 검증 로직

- [ ] Tick (또는 Timer 0.1s) 에서:
  - [ ] Camera world location 의 Z (cm) → `HMDHeight_mm = Z * 10`
  - [ ] LineTraceByChannel: Camera location, 방향 = -Z (world down), 거리 = 3m
    - [ ] Hit 위치 Z (cm) → `FloorZ_mm = HitZ * 10`
- [ ] Pass 조건 평가:
  - [ ] `HMDHeightMin_mm <= HMDHeight_mm <= HMDHeightMax_mm`
  - [ ] `abs(FloorZ_mm) <= FloorZTolerance_mm`
- [ ] Status 갱신
- [ ] Fail 시 안내 메시지 분기:
  - [ ] HMD < 1400: "You may be sitting. Please stand."
  - [ ] HMD > 2000: "Calibration may be off. Re-do Quest boundary setup."
  - [ ] Floor 초과: "Floor not detected at Z=0. Re-do boundary."

### W1.4.5 — Pinch 확인 진입

- [ ] ISDK pinch action 바인딩 (자체 pinch 작성 금지, R5)
- [ ] Pass 상태 + Pinch detected → 평가 모드 진입 (widget 제거 또는 hide)

### W1.4.6 — Widget 배치

- [ ] L_Main 시작 시 Camera 정면 1.5m 거리에 spawn
- [ ] World-space, 평가자 시야 정면 (HUD 금지)

### 검증

- [ ] VR 진입 시 widget 이 평가자 정면 1.5m 거리에 표시
- [ ] HMD Height 가 실시간 갱신 (±1mm 흔들림 OK)
- [ ] 잘못된 자세 (앉기) → "You may be sitting" 표시
- [ ] HMD > 2000 / Floor 어긋남 → 해당 메시지 표시
- [ ] 정상 자세 + Pinch → 평가 진입 (widget 사라짐)
- [ ] 모든 Magic Number (HMDHeightMin_mm 등) 가 BP variable 로 노출, 튜닝 가능

---

## W1.5 — WBP_WristPanel

### W1.5.1 — Widget Blueprint 생성

- [ ] Content/UI/ → Widget Blueprint → `WBP_WristPanel`
- [ ] Parent: UserWidget

### W1.5.2 — UI 레이아웃

```
┌──────────────────────┐
│  CALIBRATION         │
│  HMD: ---- mm        │
│  Floor: ✓ / ✗        │
│                      │
│  [Recalibrate]       │
└──────────────────────┘
```

- [ ] 텍스트 영어, 단위 명시

### W1.5.3 — Widget Component 부착

- [ ] BP_VRPawn 의 HandRig_Left 하위에 WidgetComponent 추가
- [ ] Widget Class: `WBP_WristPanel`
- [ ] Draw Size: 적절히 (예: 200×150)
- [ ] World-space, Wrist socket 위치에 attach (Quest hand mesh 의 wrist socket 이름은 ISDK Sample 에서 확인)
- [ ] Pivot: 손목에 자연스럽게 위치

### W1.5.4 — Visibility 제어

- [ ] Tick:
  - [ ] 왼손 mesh 의 Up vector 가져오기 (손바닥 방향)
  - [ ] Dot(handUp, WorldUp) 계산
  - [ ] `> 0.5` → Visibility = Visible
  - [ ] `<= 0.5` → Visibility = Collapsed (또는 Hidden)
- [ ] Threshold 값 (`WristVisibleDotThreshold`, default 0.5) 를 BP variable 로 노출

### W1.5.5 — 데이터 바인딩

- [ ] HMD Height: BP_VRPawn 또는 GameState 에서 가져와 실시간 갱신
- [ ] Floor Status: ✓ / ✗ (W1.4 의 검증 결과 참조)

### W1.5.6 — Recalibrate 버튼

- [ ] Button widget → ISDK poke 또는 pinch interaction 으로 동작 (R5)
- [ ] OnClick: WBP_CalibrationCheck 재표시 + 검증 재시작

### 검증

- [ ] 손바닥 위로 향할 때만 Wrist UI 보임
- [ ] 손등이 위로 향하거나 손이 옆을 향하면 hidden
- [ ] HMD Height 실시간 갱신
- [ ] Recalibrate 버튼 누르면 calibration widget 재진입
- [ ] HUD 가 아닌 wrist 에 attach 되어 머리 움직임 따라가지 않음 (R6)

---

## Phase 1 최종 검증 (Phase1_Kickoff.md §7)

다음을 모두 통과하면 Phase 1 완료. 사용자가 직접 확인:

- [ ] Quest 3 로 PCVR 진입 시 floor 가 발 밑에 정확히 위치
- [ ] HMD Height 값이 평가자 실제 키와 일치 (±5cm)
- [ ] Floor Z difference 가 0 ± 5mm
- [ ] Calibration Fail 시 안내 메시지 적절히 표시
- [ ] Pinch 로 calibration 통과 동작
- [ ] Wrist UI 가 손바닥 방향 따라 visible / hidden 전환
- [ ] Recalibrate 버튼 동작
- [ ] 더미 차량 박스가 정상 비례 (어깨 높이쯤)
- [ ] 차량 박스 바닥과 floor grid 가 일치 (틈 없음)
- [ ] 모든 Magic Number 가 BP variable 로 노출되어 튜닝 가능

---

## Phase 1 작업 중 막힐 때

| 증상 | 1차 확인 |
|---|---|
| Hand tracking 안 뜸 | Project Settings → Meta XR → Hand Tracking Support |
| Floor 어긋남 | `Set Tracking Origin → Floor Level` BeginPlay 호출 여부, Quest Guardian 재설정 |
| ISDK Hand rig 안 붙음 | Sample 에서 prefab 다시 Migrate, Sample branch 확인 (5.5.4-v78) |
| Wrist UI 안 보임 | Hand socket 이름 (Quest hand mesh 표준), Dot threshold (0.5) 디버그 출력 |
| Calibration 항상 Fail | HMD Height 값 print, ray cast 시각화 (Debug Line) |
| PCVR 연결 끊김 | Quest Link 앱 재시작, USB-C 데이터 케이블 (충전 전용 X), Air Link 시 5GHz Wi-Fi |
| Pinch 가 안 잡힘 | ISDK pinch action 바인딩 확인, Tracking Confidence Low 시 환경 조명 |

해결 안 되는 이슈는 `Docs/lessons_learned/<topic>.md` 로 박제.

---

## Commit 규칙

- 한 work item 완료 시 commit
- Commit message format: `phase1: <work_item> <action>`
  - 예: `phase1: W1.4 add calibration ray cast logic`
  - 예: `phase1: W1.2 wire BP_VRPawn floor tracking origin`
- 영어 commit message
