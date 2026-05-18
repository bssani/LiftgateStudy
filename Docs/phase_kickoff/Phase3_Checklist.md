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
- [ ] Power button 추가 — Construction Script 에서 `BP_PowerButton` actor spawn + link, 또는 별도로 Level 에 배치 (Editor 작업, W3.7)

### W3.2.D — BP_Liftgate_PowerHybrid

- [ ] Parent = `Liftgate`, 이름 `BP_Liftgate_PowerHybrid`
- [ ] CurrentMode = PowerHybrid
- [ ] PowerHybridHandoffAngle_deg = 30 (또는 평가자 피드백 따라)
- [ ] IsdkGrabbable / IsdkGrabTransformer 셋업 (PowerHybrid 도 grab 사용)
- [ ] Power button 추가 (위와 동일)

---

## W3.3 — BP_PowerButton

- [ ] Content/Blueprints/ → Blueprint Class → Parent: `PowerButtonActor`
- [ ] 이름: `BP_PowerButton`
- [ ] Components:
  - [ ] ButtonWidgetComp 의 Widget Class 지정 (간단한 "Power" 버튼 widget, 별도 WBP)
  - [ ] World-space (R8 / ADR-008)
  - [ ] Draw Size 적절 (예: 100×100)
- [ ] Event Graph:
  - [ ] Button 의 OnClicked → Self.OnButtonClicked() 호출 (C++ UFUNCTION)
- [ ] (간단한 WBP_PowerButton 작성 또는 ISDK Sample 의 button widget 활용)

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

## W3.5 — Widget BP Children

### W3.5.A — WBP_TestProgress

- [ ] Content/UI/ → Widget Blueprint
- [ ] Parent: `TestProgressWidget` (C++ class)
- [ ] 이름: `WBP_TestProgress`
- [ ] Designer: Vertical Box
  - [ ] TextBlock_TestNumber → Bind to `CurrentTestNumber` (1~4)
  - [ ] HorizontalBox: Button_Previous, Button_Next
  - [ ] Button_Previous.Visibility = Bind to function `ShouldShowPrevious()`
  - [ ] Button_Next OnClicked → Self.OnNextClicked() (UFUNCTION)
  - [ ] Button_Previous OnClicked → Self.OnPreviousClicked()

### W3.5.B — WBP_Comparison

- [ ] Parent: `ComparisonWidget`, 이름 `WBP_Comparison`
- [ ] Designer: Vertical Box
  - [ ] TextBlock_Title → "둘 중 어느 것이 더 좋습니까?"
  - [ ] HorizontalBox: Button_1, Button_2
  - [ ] Button_1 OnClicked → Self.OnChoice1Clicked()
  - [ ] Button_2 OnClicked → Self.OnChoice2Clicked()

### W3.5.C — WBP_FinalRanking

- [ ] Parent: `FinalRankingWidget`, 이름 `WBP_FinalRanking`
- [ ] Designer: Vertical Box
  - [ ] TextBlock_Instruction → "좋은 순서로 클릭하세요"
  - [ ] Grid 또는 HorizontalBox: Button_1, Button_2, Button_3, Button_4
  - [ ] 각 버튼 자식 TextBlock → Bind to `GetButtonLabel(index)` (returns "[1] Pick #1" 등)
  - [ ] 각 버튼 IsEnabled → Bind to `IsButtonEnabled(index)`
  - [ ] 각 버튼 OnClicked → Self.OnRankButtonClicked(index)
  - [ ] Button_Reset OnClicked → Self.OnResetClicked()

---

## W3.6 — L_Main 통합 (W3.7 of Kickoff)

### W3.6.A — Spawn anchor

- [ ] L_Main 열기
- [ ] 빈 Actor (SceneComponent only) 배치, 이름 `BP_SpawnAnchor` 또는 World-space tag
- [ ] 위치: PlayerStart 정면 약 3m, Z=0 (차량이 평가자 정면 멀리 보이도록)
- [ ] EvaluationSessionSubsystem 에서 이 anchor 의 World Transform 을 spawn 기준점으로 사용

### W3.6.B — Widget anchors

- [ ] 3 widget 의 World-space 표시 위치: 평가자 정면 1.5m, Z=1.5m
- [ ] L_Main 에 widget host actor (1 개) 배치 → Subsystem 이 phase 따라 widget swap

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
| Phase 전환 안 됨 | OnPhaseChanged 가 broadcast 되는지 (print log), widget 의 binding 확인 |
| Comparison click 안 잡힘 | World-space Button 의 Poke Interactor 거리 (R8), button 의 OnClicked binding |
| Final Ranking 버튼 안 잠김 | C++ `IsButtonEnabled(i)` 가 `IsRanked(i)` 의 반대값 반환하는지 |
| JSON 저장 실패 | Saved/ 디렉토리 권한, FFileHelper return 값, `MakeDirectory(..., true)` 호출 |
| 세션 reset 안 됨 | ConfirmSession 의 끝에서 phase=Idle 후 StartSession 재호출 |
| Power button 클릭해도 동작 없음 | BP_PowerButton 의 TargetLiftgate reference 가 spawn 시 set 되는지, button OnClicked → OnButtonClicked → TargetLiftgate.RequestPowerOpen() chain |

해결 안 되면 `Docs/lessons_learned/<topic>.md` 박제.

---

## Commit 규칙

- Work item 완료 시 commit
- C++ / BP 변경은 가능하면 별도 commit
- Format: `phase3: <work_item> <action>`
- 영어 commit message
