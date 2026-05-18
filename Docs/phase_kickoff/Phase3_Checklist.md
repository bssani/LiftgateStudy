# Phase 3 — Work Item Checklist

> Phase3_Kickoff.md 의 W3.0~W3.8 을 UE Editor / VS 2022 에서 따라갈 수 있는 체크리스트.
> 모든 항목 CLAUDE.md v0.6 (R1~R8, ADR-001~011) 준수.

---

## Pre-flight

- [ ] PR 1 (`phase3-pr1-aliftgate-root-refactor`) 머지됨
- [ ] PR 2 (본 docs + ADR) 머지됨
- [ ] PR 3 (C++ scaffolding) 머지됨
- [ ] VS 2022 컴파일 OK

---

## W3.0 — Phase 2 Leftover

### W3.0.A — BP_Liftgate Event Graph: Unselect → EndGrab wiring

- [ ] BP_Liftgate 열기 → Event Graph
- [ ] IsdkGrabbable Component → "On Interaction Pointer Event" 노드 추가
- [ ] Break `FIsdkInteractionPointerEvent` → Type 가져옴
- [ ] Switch on `EIsdkPointerEventType`:
  - [ ] **Unselect** case → Self.EndGrab() 호출
  - [ ] 다른 case (Hover, Unhover, Select, Move, Cancel) → 무시 (또는 Cancel 도 EndGrab 처리)
- [ ] 컴파일 OK

### W3.0.B — BP_Liftgate Components: IsdkGrabbable reparent (ADR-009)

- [ ] BP_Liftgate 의 Components 패널에서 IsdkGrabbable 을 LiftgateMesh 의 child → **HingePivot 의 child** 로 drag
- [ ] IsdkGrabbable 의 Details → `Grab Transformer Component` 필드가 IsdkGrabTransformer 를 가리키는지 확인

### W3.0.C — HingePivot RelativeLocation 셋업

- [ ] HingePivot 클릭 → Transform → Location (이제 편집 가능) 조정
  - mesh 가 hinge 의 한 쪽 끝에 매달리도록 (예: liftgate 상단 hinge → mesh 가 hinge 아래)
- [ ] LiftgateMesh 의 RelativeLocation 도 조정 (mesh 가 자연스럽게 swing)

---

## W3.1 — ALiftgate 4 Mode 지원

(PR 3 에 포함, 사용자 작업 없음. 단 검증)

- [ ] C++ 컴파일 OK
- [ ] BP_Liftgate Class Defaults 에서 `CurrentMode` 가 4 종 enum 으로 보임 (ManualFull/ManualAssist/PowerAuto/PowerHybrid)
- [ ] `PowerHybridHandoffAngle_deg` UPROPERTY 노출
- [ ] `RequestPowerOpen()` UFUNCTION 이 BP 에서 호출 가능

---

## W3.2 — BP_Liftgate Variant 4 종

각 variant 의 Class Defaults 차이만 다르고 parent class 동일 (`ALiftgate`).

### W3.2.A — BP_Liftgate_Manual

- [ ] Content/Blueprints/ → Blueprint Class → Parent: `Liftgate`
- [ ] 이름: `BP_Liftgate_Manual`
- [ ] Class Defaults:
  - [ ] CurrentMode = **ManualAssist** (또는 ManualFull, vehicle 1, 2 중 어느 하나의 mode 결정)
- [ ] Mesh, IsdkGrabbable, IsdkGrabTransformer 셋업 (W3.0 의 BP_Liftgate 와 동일)

### W3.2.B — BP_Liftgate_ManualFull

- [ ] Parent = `Liftgate`, 이름 `BP_Liftgate_ManualFull`
- [ ] CurrentMode = ManualFull

### W3.2.C — BP_Liftgate_PowerAuto

- [ ] Parent = `Liftgate`, 이름 `BP_Liftgate_PowerAuto`
- [ ] CurrentMode = PowerAuto
- [ ] (선택) IsdkGrabbable / IsdkGrabTransformer 제거 (Power-Auto 는 grab 무효) — 또는 그대로 두고 mode 가 grab 무시
- [ ] Power button 추가 (ADR-012):
  - [ ] Construction Script 에서 `BP_PowerButton` (BigRedButton ⊂) SpawnActor — `TargetLiftgate` = self
  - [ ] Spawn transform: 차량 옆, 평가자 정면 30~60cm, 어깨~허리 높이 (CLAUDE.md §8)
  - [ ] 또는 BP_Liftgate_PowerAuto 의 Components 에 BP_PowerButton 을 child actor 로 배치, BeginPlay 에서 `TargetLiftgate` set
  - [ ] 또는 Level 에 직접 배치 (W3.7) — `TargetLiftgate` 를 Editor 에서 수동 wiring

### W3.2.D — BP_Liftgate_PowerHybrid

- [ ] Parent = `Liftgate`, 이름 `BP_Liftgate_PowerHybrid`
- [ ] CurrentMode = PowerHybrid
- [ ] PowerHybridHandoffAngle_deg = 30 (또는 평가자 피드백 따라)
- [ ] IsdkGrabbable / IsdkGrabTransformer 셋업 (PowerHybrid 도 grab 사용)
- [ ] Power button 추가 — ADR-012, W3.2.C 와 동일 방식 (TargetLiftgate = self)

---

## W3.3 — BP_PowerButton (ADR-012 — BigRedButton 의 child)

> ADR-012 채택: `APowerButtonActor` C++ 제거. `BP_PowerButton` 은 ISDK plugin BP `BigRedButton` (`Plugins/OculusInteractionSamples/Content/Objects/Props/BigRedButton/BigRedButton.uasset`) 의 child Blueprint.

- [ ] Content/Blueprints/ → Blueprint Class → All Classes → Parent 선택 창에서 `BigRedButton` 검색 → 채택
- [ ] 이름: `BP_PowerButton`
- [ ] **Variables 추가**:
  - [ ] `TargetLiftgate` — Type: `Liftgate` (Object Reference), Category: `PowerButton`
  - [ ] Instance Editable: ✓
  - [ ] Expose On Spawn: ✓
- [ ] **Event Graph** (parent 의 셋업은 그대로 두고 본 wiring 만 추가):
  - [ ] Components 패널 → `PokeInteractable` 우클릭 → `Bind Event to Interactor Pointer Event` 또는 parent 의 `InteractorPointerEvent` 재활용
  - [ ] Event 의 PointerEvent 출력 → `Break Isdk Interaction Pointer Event` 노드
  - [ ] `Type` 핀 → `Equal (EIsdkPointerEventType)` 노드 → `Select` 비교
  - [ ] Branch True → `TargetLiftgate` 변수 → `Request Power Open` (`ALiftgate::RequestPowerOpen()`) 호출
  - [ ] (선택) `Type == Cancel` 의 경우 무시 또는 reset visual
- [ ] **Parent 의 Debug 무음**:
  - [ ] BP_PowerButton 의 Class Defaults / Event Graph 에서 parent 의 `Enable Debug Output` flag (BigRedButton 의 Print IsdkPointerEvents 분기 제어) → false 로 set (parent 가 expose 했을 경우)
- [ ] **시각 / Audio 는 parent 가 자동** — Hover / Select 색상 전이, PokePressCue / PokeReleaseCue 사용 (BigRedButton 의 StateChanged + Setup Pointable Audio 그래프)
- [ ] (선택) 차량 디자인에 맞춘 visual tweak — BigRedButton 의 `Pointable Plane Size` / mesh material 등 Class Defaults override

---

## W3.4 — DA_TestSet_Default Data Asset

- [ ] Content/DataAssets/ 폴더 생성 (없으면)
- [ ] 우클릭 → Miscellaneous → Data Asset → Class: `VehicleTestSet`
- [ ] 이름: `DA_TestSet_Default`
- [ ] 열기 → Tests 배열에 4 entry 추가:
  - [ ] **Index 0**: 
    - Id = "Test1"
    - DebugName = (선택)
    - LiftgateClass = `BP_Liftgate_Manual` (또는 ManualFull)
    - SpawnRelativeTransform = (0, 0, 0) (anchor 기준)
    - ModelCode = "VehicleA"  
  - [ ] **Index 1**: `BP_Liftgate_ManualFull` (또는 ManualAssist 와 다른 variant), ModelCode = "VehicleB"
  - [ ] **Index 2**: `BP_Liftgate_PowerAuto`, ModelCode = "VehicleC"
  - [ ] **Index 3**: `BP_Liftgate_PowerHybrid`, ModelCode = "VehicleD"
- [ ] 저장

---

## W3.5 — Session UI Widget (ADR-013)

> ADR-013 채택: top-level widget 은 `WBP_SessionUI` 1 종 (umbrella). 내부 WidgetSwitcher 가 3 inner panel 을 phase 따라 전환.

### W3.5.A — WBP_SessionUI (umbrella, ADR-013) — 신규

- [ ] Content/UI/ → Widget Blueprint
- [ ] Parent: `SessionUIWidget` (C++ class, 신규 — 후속 PR 에서 scaffolding)
- [ ] 이름: `WBP_SessionUI`
- [ ] Designer: WidgetSwitcher (root child)
  - [ ] Slot 0: `WBP_TestProgress` instance (Child Widget)
  - [ ] Slot 1: `WBP_Comparison` instance
  - [ ] Slot 2: `WBP_FinalRanking` instance
- [ ] Event Graph (또는 C++ NativeOnInitialized):
  - [ ] BeginPlay / Construct → `UEvaluationSessionSubsystem` 의 `OnPhaseChanged` subscribe
  - [ ] Phase 매핑: Test1~Test4 → SetActiveWidgetIndex(0), Compare1v2 → 1, FinalRanking → 2, Idle/WritingLog → Visibility=Collapsed
- [ ] 활성 panel 의 reference 를 panel C++ base 로 cast → `RefreshFromSubsystem()` 호출 (필요 시 추후 추가)

### W3.5.B — WBP_TestProgress (inner panel)

- [ ] Content/UI/ → Widget Blueprint
- [ ] Parent: `TestProgressWidget` (C++ class)
- [ ] 이름: `WBP_TestProgress`
- [ ] Designer: Vertical Box
  - [ ] TextBlock_TestNumber → Bind to `CurrentTestNumber` (1~4)
  - [ ] HorizontalBox: Button_Previous, Button_Next
  - [ ] Button_Previous.Visibility = Bind to function `ShouldShowPrevious()`
  - [ ] Button_Next OnClicked → Self.OnNextClicked() (UFUNCTION)
  - [ ] Button_Previous OnClicked → Self.OnPreviousClicked()
- [ ] **Top-level placement 없음** — WBP_SessionUI 의 WidgetSwitcher slot 0 에 instance 로만 사용

### W3.5.C — WBP_Comparison (inner panel)

- [ ] Parent: `ComparisonWidget`, 이름 `WBP_Comparison`
- [ ] Designer: Vertical Box
  - [ ] TextBlock_Title → "둘 중 어느 것이 더 좋습니까?"
  - [ ] HorizontalBox: Button_1, Button_2
  - [ ] Button_1 OnClicked → Self.OnChoice1Clicked()
  - [ ] Button_2 OnClicked → Self.OnChoice2Clicked()
- [ ] Top-level placement 없음 — WBP_SessionUI 의 slot 1

### W3.5.D — WBP_FinalRanking (inner panel)

- [ ] Parent: `FinalRankingWidget`, 이름 `WBP_FinalRanking`
- [ ] Designer: Vertical Box
  - [ ] TextBlock_Instruction → "좋은 순서로 클릭하세요"
  - [ ] Grid 또는 HorizontalBox: Button_1, Button_2, Button_3, Button_4
  - [ ] 각 버튼 자식 TextBlock → Bind to `GetRankLabel(index)` (returns "1순위" 등)
  - [ ] 각 버튼 IsEnabled → Bind to `IsButtonEnabled(index)`
  - [ ] 각 버튼 OnClicked → Self.OnRankButtonClicked(index)
  - [ ] Button_Reset OnClicked → Self.OnResetClicked()
- [ ] Top-level placement 없음 — WBP_SessionUI 의 slot 2

---

## W3.6 — L_Main 통합 (W3.7 of Kickoff)

### W3.6.A — Spawn anchor

- [ ] L_Main 열기
- [ ] 빈 Actor (SceneComponent only) 배치, 이름 `BP_SpawnAnchor` 또는 World-space tag
- [ ] 위치: PlayerStart 정면 약 3m, Z=0 (차량이 평가자 정면 멀리 보이도록)
- [ ] EvaluationSessionSubsystem 에서 이 anchor 의 World Transform 을 spawn 기준점으로 사용

### W3.6.B — Session UI anchor (ADR-013)

- [ ] `BP_SessionUIAnchor` 1 개 L_Main 에 배치 (단일 host — phase swap 은 widget 내부 switcher 가 담당)
- [ ] 위치: PlayerStart 우측 +50cm, 정면 +20cm, Z=120cm (chest height)
- [ ] Rotation: widget normal 이 pawn 쪽 (slight inward yaw, 예: PlayerStart 정면이 +X 면 widget 의 yaw 는 -100° ~ -110° 정도로 pawn 쪽으로 살짝 기울임)
- [ ] 검증: pawn 이 PlayerStart 에 서서 정면 (liftgate 방향) 을 바라볼 때
  - [ ] Widget 이 시각 정면을 가리지 않음 (peripheral 가능)
  - [ ] 손가락으로 widget 의 모든 버튼 poke 가능 (팔 뻗기로 닿음)
  - [ ] Liftgate grab 시 widget 이 시야 / 손 동선 방해 안 함
- [ ] Anchor Transform 은 `BP_SessionUIAnchor` 의 UPROPERTY 로 노출 (Class Defaults 에서 평가자 피드백 따라 조정)

### W3.6.C — Session 시작

- [ ] BP_LiftgateStudyGameMode 의 BeginPlay 에서:
  - [ ] Calibration 통과 후 (또는 calibration confirm event 받은 후)
  - [ ] Subsystem.StartSession(DA_TestSet_Default) 호출
- [ ] 또는 Calibration Gate 의 OnCalibrationComplete event 에 연결

---

## W3.7 — VR Preview 전체 검증

### 흐름 검증

- [ ] VR Preview 시작 → Calibration widget 표시
- [ ] Spacebar (또는 confirm) → 평가 모드 진입
- [ ] WBP_TestProgress 표시 ("테스트 1"), BP_Liftgate_Manual 차량 spawn
- [ ] 차량의 liftgate grab → ISDK GrabTransformer 회전
- [ ] (ManualAssist) 40° 이상 release → 자동 완전 개방
- [ ] Next 버튼 poke → 차량 destroy, BP_Liftgate_ManualFull spawn, "테스트 2" 표시
- [ ] Previous 버튼 poke → 다시 BP_Liftgate_Manual spawn, "테스트 1"
- [ ] 다시 Next → "테스트 2" → Next → WBP_Comparison 표시 (차량 destroy)
- [ ] "1" 또는 "2" 클릭 → WBP_TestProgress "테스트 3", BP_Liftgate_PowerAuto spawn
- [ ] Power button 보이는지 → poke → 자동 개방
- [ ] Next → "테스트 4" → BP_Liftgate_PowerHybrid spawn
- [ ] Power button poke → 일정 각도 자동 → 멈춤 → grab 으로 계속 열기 → 40° 이상 release → 완전 개방
- [ ] Next → WBP_FinalRanking 표시
- [ ] 4 버튼 순서대로 click → 버튼 pressed/disabled, rank 숫자 표시
- [ ] Reset 클릭 → 4 버튼 enabled, ranking 비움
- [ ] 다시 4 개 click → 자동 confirm → JSON 저장 → 세션 reset (Test 1 로)

### JSON 검증

- [ ] `<ProjectDir>/Saved/EvaluationLogs/session_*.json` 파일 생성
- [ ] 내용:
  - schema_version=1
  - session_id (UUID v4 format)
  - timestamp_utc
  - tests array (4 entry)
  - comparison_1v2_winner_slot (1 또는 2)
  - final_ranking_by_slot (4 element array)

### 모든 UPROPERTY 노출 확인

- [ ] BP_Liftgate_Manual / _ManualFull / _PowerAuto / _PowerHybrid 의 Class Defaults 에서 ThresholdAngle / MaxAngle / GrabSensitivity / PowerHybridHandoffAngle 모두 조정 가능
- [ ] DA_TestSet_Default 의 4 entry 자유 편집

---

## 막힐 시 디버그

| 증상 | 1차 확인 |
|---|---|
| C++ 컴파일 실패 | `.Build.cs` 에 `Json`, `JsonUtilities`, `DeveloperSettings` 추가했는지 |
| Vehicle spawn 안 됨 | DA_TestSet_Default 의 entry 확인, Subsystem 의 SpawnActor 호출 시 World 가 valid 한지 |
| Phase 전환 안 됨 | OnPhaseChanged 가 broadcast 되는지 (print log), `WBP_SessionUI` 의 WidgetSwitcher SetActiveWidgetIndex 호출 확인 (ADR-013) |
| Liftgate 조작 시 widget 이 가림 | `BP_SessionUIAnchor` 의 Transform 조정 — 더 옆으로 / 더 뒤로 / 더 낮게 (ADR-013 D2) |
| Widget 의 버튼이 손에 안 닿음 | Anchor 가 너무 멀거나 높음 — pawn standing position 에서 팔 뻗어 30~60cm 거리로 조정 |
| Comparison click 안 잡힘 | World-space Button 의 Poke Interactor 거리 (R8), button 의 OnClicked binding |
| Final Ranking 버튼 안 잠김 | C++ `IsButtonEnabled(i)` 가 `IsRanked(i)` 의 반대값 반환하는지 |
| JSON 저장 실패 | Saved/ 디렉토리 권한, FFileHelper return 값, `MakeDirectory(..., true)` 호출 |
| 세션 reset 안 됨 | ConfirmSession 의 끝에서 phase=Idle 후 StartSession 재호출 |
| Power button 클릭해도 동작 없음 | (ADR-012) BP_PowerButton 의 parent 가 `BigRedButton` 인지, Event Graph 의 `Type == Select` 분기가 wired 됐는지, `TargetLiftgate` ref 가 spawn 시 set 됐는지 |
| BigRedButton 의 색상 / 사운드 피드백 없음 | parent BP 의 `Bind Event to Interactable State Changed` 그래프가 disable 되지 않았는지, dynamic material instance 가 생성됐는지 |

해결 안 되면 `Docs/lessons_learned/<topic>.md` 박제.

---

## Commit 규칙

- Work item 완료 시 commit
- C++ / BP 변경은 가능하면 별도 commit
- Format: `phase3: <work_item> <action>`
- 영어 commit message
