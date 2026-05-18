# Phase 3 Kickoff — Test Session + 4 Modes + Vehicle Swap + Logging

> **For Claude Code**: Phase 3 의 작업 명세. 모든 작업은 `CLAUDE.md` v0.6 우선. 충돌 시 사용자 확인.

---

## 1. Phase 3 Goal

평가자가 한 세션 동안:

1. Test 1 (차량 1 의 liftgate) 체험 → Next
2. Test 2 (차량 2) 체험, Prev/Next 로 Test 1 ↔ Test 2 자유 swap → Next
3. **Comparison 1 vs 2** widget — 더 나은 것 클릭 → 즉시 Test 3 으로 진입
4. Test 3 체험 → Next
5. Test 4 체험, Prev/Next 로 Test 3 ↔ Test 4 자유 swap → Next
6. **Final Ranking** widget — 4 vehicle 을 좋은 순서로 클릭. pressed 버튼은 lock. Reset 으로 정정 가능. 4 개 모두 클릭 시 자동 confirm
7. JSON 자동 저장 (`Saved/EvaluationLogs/session_*.json`, ADR-006) + 세션 자동 리셋 → Test 1 부터 다음 평가자 진행 가능

CLAUDE.md §9 의 4 mode (Manual-Full / Manual-Assist / Power-Auto / Power-Hybrid) 가 각각 차량 1~4 의 동작으로 적용됨.

(Phase 3 와 4 사이 comparison 은 **없음** — 사용자 spec)

---

## 2. Pre-conditions

- [ ] Phase 1 §7 검증 통과 (calibration / dummy car / VR setup)
- [ ] Phase 2 §7 검증 통과 (Manual-Assist BP_Liftgate 동작 OK)
- [ ] **ADR-009 적용된 ALiftgate root component pattern** — PR `phase3-pr1` 머지됨
- [ ] BP_Liftgate 의 IsdkGrabbable 이 HingePivot 의 child 로 reparent 완료 (Editor 작업)

---

## 3. Folder Structure (Phase 3 종료 시)

```
Source/LiftGateStudy/
├── Public/
│   ├── Calibration/    (Phase 1)
│   ├── Liftgate/
│   │   ├── LiftgateTypes.h         (ELiftgateMode + ELiftgateState + Phase 3 의 FVehicleTestEntry, EEvaluationPhase 추가)
│   │   └── Liftgate.h              (4 mode 모두 지원, Power button trigger API 추가)
│   │   # PowerButtonActor.h/.cpp 는 ADR-012 로 제거됨 — BP_PowerButton 이 BigRedButton (ISDK plugin BP) 의 child 로 작성
│   ├── Session/                    (Phase 3 신규)
│   │   ├── VehicleTestSet.h        (UDataAsset)
│   │   └── EvaluationSessionSubsystem.h
│   └── UI/                         (Phase 3 신규)
│       ├── TestProgressWidget.h
│       ├── ComparisonWidget.h
│       └── FinalRankingWidget.h
└── Private/  (대응 .cpp)

Content/
├── Blueprints/
│   ├── BP_LiftgateStudyGameMode.uasset   (Phase 1)
│   ├── BP_VRPawn.uasset                  (Phase 1, ADR-004)
│   ├── BP_CalibrationGate.uasset         (Phase 1)
│   ├── BP_Liftgate_Manual.uasset         (Phase 3 신규, ManualFull 또는 ManualAssist 의 base)
│   ├── BP_Liftgate_PowerAuto.uasset      (Phase 3 신규)
│   ├── BP_Liftgate_PowerHybrid.uasset    (Phase 3 신규)
│   └── BP_PowerButton.uasset             (Phase 3 신규, BigRedButton (ISDK plugin BP) 의 child — ADR-012)
├── UI/
│   ├── WBP_CalibrationCheck.uasset       (Phase 1)
│   ├── WBP_TestProgress.uasset           (Phase 3)
│   ├── WBP_Comparison.uasset             (Phase 3)
│   └── WBP_FinalRanking.uasset           (Phase 3)
├── DataAssets/                            (Phase 3 신규 폴더)
│   └── DA_TestSet_Default.uasset
└── Maps/
    ├── L_Main.umap               (Phase 3 에서 spawn anchor 추가, session driver 배치)
    └── L_Vehicle_Dummy.umap      (Phase 1, 환경)
```

---

## 4. Work Items

### W3.0 — Phase 2 마무리 잔여 (작음)

> Phase 2 W2.4 / W2.5 / W2.6 이 Phase 3 와 통합되므로 잔여 정리.

- [ ] BP_Liftgate Event Graph: ISDK `InteractionPointerEvent` 의 `Unselect` type → `ALiftgate::EndGrab()` 호출 wiring (PR1 머지 후 사용자 BP 작업)
- [ ] `ALiftgate::EndGrab` 가 HingePivot 의 실제 회전값을 읽도록 수정 (PR3 에 포함)
- [ ] Manual-Assist 40° auto-complete 동작 검증 (VR Preview)

---

### W3.1 — ALiftgate 4 Mode 지원 + Power Button API

기존 ALiftgate 에 다음 추가:

```cpp
// Power-Auto / Power-Hybrid 의 Power 버튼이 호출
UFUNCTION(BlueprintCallable, Category="Liftgate")
void RequestPowerOpen();

// Power-Hybrid: 자동 개방 중간 각도 (ManualFull로 전환되는 각도)
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Liftgate", meta=(ClampMin="0"))
float PowerHybridHandoffAngle_deg = 30.f;
```

Mode 별 동작 (`EndGrab` + `Tick` 분기):

| Mode | Grab 동작 | Release 동작 | Power button |
|---|---|---|---|
| ManualFull | ISDK GrabTransformer 회전 | 그 자리 멈춤 (Phase 3 단순화: 중력 없음) | N/A |
| ManualAssist | 동일 | Threshold 이상 → AutoOpening lerp | N/A |
| PowerAuto | Grab 무시 (또는 grab 가능하지만 효과 없음) | N/A | 클릭 → AutoOpening (시작 = Closed 각도) |
| PowerHybrid | ISDK 회전 (단, 일정 각도 이상부터 grab 받음) | 그 자리 멈춤 | 클릭 → AutoOpening 으로 PowerHybridHandoffAngle_deg 까지, 그 다음 `State=Closed` (idle, grab 받을 준비) |

Power-Hybrid 의 흐름:
```
Idle (Closed)
  │ RequestPowerOpen()
  ▼
AutoOpening (target = PowerHybridHandoffAngle_deg)
  │ 도달 시
  ▼
Idle (Closed) ← 사용자가 grab 으로 계속 열 수 있음
  │ ISDK Grab + EndGrab (ManualAssist 와 동일)
  ▼
AutoOpening (target = MaxAngle_deg) — Threshold 이상이면
또는 그 자리 멈춤 — Threshold 미만이면
```

---

### W3.2 — Power Button (BP_PowerButton, BigRedButton 의 child) — ADR-012

**ADR-012 채택**으로 `APowerButtonActor` C++ 클래스는 제거. `BP_PowerButton` 은 ISDK plugin BP `BigRedButton` (`Plugins/OculusInteractionSamples/Content/Objects/Props/BigRedButton/BigRedButton.uasset`) 의 child Blueprint.

**BP_PowerButton (BigRedButton ⊂)**:
- **Variables** (BP 에서 추가):
  - `TargetLiftgate` (`ALiftgate*`, `Category="PowerButton"`, Instance Editable, Expose On Spawn)
- **Event Graph** (BigRedButton parent 의 셋업 + 본 추가):
  - parent 의 `InteractorPointerEvent` 구독
  - `Break Isdk Interaction Pointer Event` → `Type == Select` 분기 → `TargetLiftgate.RequestPowerOpen()` 호출
  - `Enable Debug Output` flag → false (parent 의 Print 디버그 무음)

**Spawn 책임**:
- PowerAuto / PowerHybrid vehicle 의 spawn 시 `BP_PowerButton` 도 함께 spawn
- `TargetLiftgate` 를 SpawnActor 시 해당 ALiftgate instance 로 set (Expose On Spawn 으로 wiring)
- spawn 위치: 차량 옆 평가자 정면 30~60cm, 어깨~허리 높이 (CLAUDE.md §8) — `BP_Liftgate_PowerAuto` / `_PowerHybrid` 의 Construction Script 또는 Level 직접 배치

**라우팅**:
| 책임 | 위치 |
|---|---|
| Poke detection | ISDK PokeInteractable (BigRedButton 내장) |
| 시각 / audio 피드백 | BigRedButton parent (StateChanged → color, PokePressCue / PokeReleaseCue) |
| Select event 분기 | BP_PowerButton Event Graph (Select type 만 처리) |
| Liftgate 상태 머신 | `ALiftgate::RequestPowerOpen()` (C++) |

ADR-007 (button poke) 의 구체화 — ISDK native pipeline 활용으로 UMG OnClicked 우회 제거.

---

### W3.3 — FVehicleTestEntry + UVehicleTestSet (ADR-010)

```cpp
USTRUCT(BlueprintType)
struct FVehicleTestEntry
{
    UPROPERTY(EditAnywhere) FName Id;
    UPROPERTY(EditAnywhere) FText DebugName;
    UPROPERTY(EditAnywhere) TSubclassOf<ALiftgate> LiftgateClass;
    UPROPERTY(EditAnywhere) FTransform SpawnRelativeTransform;
    UPROPERTY(EditAnywhere) FName ModelCode;  // logging
};

UCLASS(BlueprintType)
class LIFTGATESTUDY_API UVehicleTestSet : public UPrimaryDataAsset
{
    UPROPERTY(EditAnywhere) TArray<FVehicleTestEntry> Tests;  // 정확히 4
};
```

Editor 에서 `DA_TestSet_Default` 자산 생성 후 4 entry 등록.

---

### W3.4 — UEvaluationSessionSubsystem (ADR-011)

상태 머신 + vehicle spawn/destroy + JSON 저장 책임.

```cpp
UENUM(BlueprintType)
enum class EEvaluationPhase : uint8
{
    Idle, Test1, Test2, Compare1v2, Test3, Test4, FinalRanking, WritingLog
};

UCLASS()
class LIFTGATESTUDY_API UEvaluationSessionSubsystem : public UGameInstanceSubsystem
{
    UFUNCTION(BlueprintCallable) void StartSession(UVehicleTestSet* TestSet);
    UFUNCTION(BlueprintCallable) void RequestNext();
    UFUNCTION(BlueprintCallable) void RequestPrevious();
    UFUNCTION(BlueprintCallable) void RequestComparisonChoice(int32 LocalWinnerIndex);
    UFUNCTION(BlueprintCallable) void AddRanking(int32 VehicleIndex);
    UFUNCTION(BlueprintCallable) void ResetRanking();
    UFUNCTION(BlueprintCallable) bool IsRanked(int32 VehicleIndex) const;
    UFUNCTION(BlueprintCallable) int32 GetRankOrder(int32 VehicleIndex) const;

    UPROPERTY(BlueprintAssignable) FOnPhaseChangedSignature OnPhaseChanged;

    // Spawn / destroy 책임 — Phase 전환 시 자동 호출
private:
    UPROPERTY(Transient) TObjectPtr<ALiftgate> ActiveLiftgate;
    void SpawnVehicleForIndex(int32 SlotIndex);  // 0~3
    void DestroyActiveLiftgate();
    void WriteSessionLog();   // ADR-006, JSON
};
```

Spawn anchor: `L_Main` 의 정해진 위치 (`PlayerStart` 정면 약 3m, Z=0). FVehicleTestEntry 의 `SpawnRelativeTransform` 추가 적용.

---

### W3.5 — Widgets (3 종)

#### WBP_TestProgress (C++ base: UTestProgressWidget)

- 표시: "테스트 N" 큰 텍스트 (N=1~4)
- Buttons: 
  - Previous (Test 2 / Test 4 에서만 visible) → Subsystem.RequestPrevious()
  - Next → Subsystem.RequestNext()
- World-space placement (ADR-008): 평가자 정면 1.5m, 어깨 높이

#### WBP_Comparison (C++ base: UComparisonWidget)

- 표시: "둘 중 어느 것이 더 좋습니까?" + "1" "2" 두 버튼
- Click 1 → Subsystem.RequestComparisonChoice(0)
- Click 2 → Subsystem.RequestComparisonChoice(1)
- 즉시 다음 phase

#### WBP_FinalRanking (C++ base: UFinalRankingWidget)

- 표시: "좋은 순서로 클릭하세요" + 4 버튼 ([1] [2] [3] [4]) + Reset 버튼
- 버튼 클릭 시: 
  - Subsystem.AddRanking(vehicleIndex)
  - Subsystem.GetRankOrder(vehicleIndex) 로 순서 (1~4) 시각 표시 (텍스트 또는 색 변경)
  - 해당 버튼 disabled (재클릭 방지)
- 4 개 모두 ranked 시: 자동 ConfirmSession() 호출 → JSON write → 세션 reset → Test 1 로 복귀
- Reset 버튼: Subsystem.ResetRanking() → 4 버튼 모두 enabled, 순서 비움

각 widget 의 visibility 는 `OnPhaseChanged` 구독으로 자동 토글.

---

### W3.6 — JSON Writer (ADR-006)

```cpp
// EvaluationSessionSubsystem.cpp 내부
void UEvaluationSessionSubsystem::WriteSessionLog()
{
    // 1. JSON object 빌드 (FJsonObject)
    // 2. FJsonSerializer::Serialize → FString
    // 3. FFileHelper::SaveStringToFile to FPaths::ProjectSavedDir()/EvaluationLogs/session_<timestamp>_<uuid>.json
    // 4. IFileManager::MakeDirectory(.../EvaluationLogs, true)
}
```

`Json` / `JsonUtilities` 모듈 dependency 추가 (.Build.cs).

---

### W3.7 — L_Main Integration

- [ ] L_Main 에 Session driver 배치 (또는 BP_LiftgateStudyGameMode 의 BeginPlay 에서 Subsystem.StartSession() 호출)
- [ ] Spawn anchor: World-space 정해진 위치 (PlayerStart 정면 3m)
- [ ] 3 widget 의 World-space spawn anchor 도 정의 (평가자 정면 1.5m, 어깨 높이)
- [ ] DA_TestSet_Default 참조 — GameMode 또는 GameInstance 에 UPROPERTY

---

### W3.8 — VR Preview 전체 검증

§7 verification 항목 통과 확인.

---

## 5. 관련 ADR

| ADR | 주제 | Phase 3 영향 |
|---|---|---|
| ADR-001 | ISDK 기반 Hand Grab | 4 vehicle 모두 ISDK grab 사용 |
| ADR-002 | Mode 자유 토글 | **재해석**: Phase 3 는 각 vehicle 이 고정 mode (실차 충실). Phase 3 의 paired comparison 가 평가 핵심 → "자유 토글" 자유는 약화. ADR-002 의 향후 확장은 별도 phase |
| ADR-003 | 평가 데이터 로깅 없음 | ADR-006 이 partial supersede |
| ADR-004 | BP_VRPawn = IsdkSamplePawn child | 영향 없음 |
| ADR-005 | C++ + BP hybrid | 모든 Phase 3 신규 클래스가 C++ base + BP child |
| ADR-006 | Evaluation Result Logging | **핵심** — JSON 저장 |
| ADR-007 | Button poke | 모든 widget 이 button poke |
| ADR-008 | World-space UI | 모든 widget 이 World-space |
| ADR-009 | ALiftgate root pattern | **prereq** — PR1 |
| ADR-010 | Vehicle Test Set Data Asset | **핵심** — UVehicleTestSet |
| ADR-011 | Comparison-based Test Session | **핵심** — Subsystem 상태 머신 |
| ADR-012 | BP_PowerButton ⊂ BigRedButton | **W3.2 / W3.3 갱신** — APowerButtonActor 제거, ISDK plugin BP 의 child 로 |

---

## 6. Deliverables

1. **C++**:
   - `ALiftgate` 4 mode 지원 + RequestPowerOpen API
   - `FVehicleTestEntry`, `UVehicleTestSet`
   - `EEvaluationPhase`, `UEvaluationSessionSubsystem` (JSON writer 포함)
   - `UTestProgressWidget`, `UComparisonWidget`, `UFinalRankingWidget`
   - (ADR-012 — `APowerButtonActor` 는 deliverable 에서 제외, 별도 PR 에서 코드 삭제 정리)
2. **BP**:
   - `BP_Liftgate_Manual` (ManualFull 또는 ManualAssist 의 base)
   - `BP_Liftgate_PowerAuto`
   - `BP_Liftgate_PowerHybrid`
   - `BP_PowerButton` (parent = `BigRedButton`, ISDK plugin BP — ADR-012)
   - `WBP_TestProgress`, `WBP_Comparison`, `WBP_FinalRanking`
3. **Data**:
   - `DA_TestSet_Default` (UVehicleTestSet 자산)
4. **Level**:
   - `L_Main` 에 spawn anchor + widget anchor + session driver 통합
5. **Logging**:
   - `Saved/EvaluationLogs/session_*.json` 자동 생성

---

## 7. Verification Criteria

사용자 직접 확인:

- [ ] C++ 모듈 컴파일 OK (VS 2022)
- [ ] VR 진입 → Calibration → 평가 모드 진입
- [ ] Test 1 widget 표시 + Vehicle 1 spawn
- [ ] Test 1 Next → Vehicle 1 destroy + Vehicle 2 spawn, Test 2 widget
- [ ] Test 2 Previous → Vehicle 2 destroy + Vehicle 1 spawn, Test 1 widget
- [ ] Test 1 ↔ Test 2 자유 토글 동작
- [ ] Test 2 Next → Comparison 1v2 widget 표시 (vehicle destroy)
- [ ] Comparison click 1 또는 2 → 즉시 Test 3 widget + Vehicle 3 spawn
- [ ] Test 3 ↔ Test 4 토글 동작
- [ ] Test 4 Next → Final Ranking widget (vehicle destroy)
- [ ] Final Ranking: 첫 click → 해당 버튼 rank=1 표시 + disabled. 둘째 click → rank=2 + disabled. 이런 식으로 4 개 click 까지
- [ ] Reset → 4 버튼 모두 enabled, ranking 비움
- [ ] 4 개 모두 click → 자동 confirm → JSON 파일이 `Saved/EvaluationLogs/` 에 생성
- [ ] JSON 내용 검증 (schema_version, session_id UUID, tests, comparison_1v2_winner_slot, final_ranking_by_slot)
- [ ] 세션 자동 reset → Test 1 widget 다시 표시 (Vehicle 1 spawn)
- [ ] Power-Auto vehicle: Power button poke → AutoOpening
- [ ] Power-Hybrid: button poke → 일정 각도 → Idle (grab 받음) → grab → ManualAssist 처럼 완전 개방
- [ ] 모든 UPROPERTY 가 BP child 의 Class Defaults 에서 조정 가능

---

## 8. Out of Scope (Phase 4 또는 별도)

- 실차 CAD mesh (Datasmith) — Phase 4
- Height ruler (차량 옆 키 표시 막대) — Phase 4
- Zone visualizer (Red/Green/Yellow) — Phase 4
- 평가자 onboarding (첫 진입 안내) — Phase 4
- 평가 결과 in-VR 시각화 — Phase 4 이후
- 세부 데이터 (grab 시간, hand trajectory, 각속도 등) logging — 명시적 금지 (ADR-006)
- 평가자 ID 입력 — anonymous 원칙 (ADR-006)
- 세션 도중 quit / 일시정지 — 자동 reset 으로 단순화
- Manual-Full 의 중력 / damping (release 후 닫힘) — Phase 3 단순화: 그 자리 멈춤. 실차 충실 필요 시 별도 ADR

---

## 9. 확정된 Decision

| 항목 | 확정 |
|---|---|
| 세션 흐름 | Test1→Test2→Compare1v2→Test3→Test4→FinalRanking (3v4 comparison 없음) |
| Comparison 선택 | Click = 즉시 결정 (별도 confirm 버튼 없음) |
| Final Ranking | 4 버튼 순서대로 click. Pressed 시 disabled. Reset 으로 정정 |
| 4 ranking 후 | 자동 confirm + JSON 저장 + 세션 reset |
| Logging | JSON, `Saved/EvaluationLogs/`, schema_version=1 |
| 평가자 ID | anonymous (UUID v4 만) |
| Vehicle swap | destroy → spawn (in-progress 상태 유지 안 함) |
| Vehicle 데이터 | UDataAsset (`DA_TestSet_Default`) |
| 세션 상태 머신 | `UEvaluationSessionSubsystem` (`UGameInstanceSubsystem`) |
| Phase 3 Mode 자유 토글 | 아님 — 각 vehicle 이 고정 mode (ADR-002 재해석) |
| Phase 3 와 4 사이 boundary | Phase 3: Mode 동작 + session UI + logging. Phase 4: real CAD + ruler + zone + polish |

---

## 10. 작업 순서 (권장)

1. **PR 1** (별도 branch) — ALiftgate root pattern refactor (ADR-009) ✓
2. **PR 2** (현재) — 본 문서 + ADR-006/009/010/011 + CLAUDE.md v0.6 + Phase3_Checklist.md
3. **PR 3** — C++ scaffolding 일괄:
   - ALiftgate 4 mode 지원 + RequestPowerOpen
   - FVehicleTestEntry, UVehicleTestSet
   - UEvaluationSessionSubsystem + JSON writer
   - 3 widget C++ bases
   - (ADR-012 후속) APowerButtonActor 코드 삭제 정리
4. **사용자 작업** (PR 3 머지 후, Editor 에서):
   - BP_Liftgate_Manual / _PowerAuto / _PowerHybrid 4 variant 생성
   - BP_PowerButton 생성 (parent = `BigRedButton`, ADR-012)
   - WBP_TestProgress / _Comparison / _FinalRanking 생성
   - DA_TestSet_Default 자산 만들기
   - L_Main 에 spawn anchor 배치 + GameMode 또는 별도 actor 가 StartSession() 호출
5. **VR Preview 검증** — W3.8

---

## 11. 막힐 시 디버그

| 증상 | 1차 확인 |
|---|---|
| Vehicle spawn 안 됨 | `DA_TestSet_Default` 의 LiftgateClass 가 valid 한 BP class 인지, Subsystem 의 SpawnActor 호출 시 transform 이 valid 한지 |
| Phase 전환 안 됨 | Subsystem 의 OnPhaseChanged 가 broadcast 되는지, widget 이 구독하는지 |
| JSON 저장 안 됨 | `Saved/EvaluationLogs/` 디렉토리 자동 생성 코드, FFileHelper::SaveStringToFile 의 return 값 |
| Power button click 안 잡힘 | BP_PowerButton 의 parent (`BigRedButton`) 가 ISDK PokeInteractable 셋업을 가지고 있는지, BP_PowerButton 의 Event Graph 에서 `Select` type 분기가 정확한지, `TargetLiftgate` ref 가 spawn 시 set 됐는지 (ADR-012) |
| Power-Hybrid 의 handoff 안 일어남 | `PowerHybridHandoffAngle_deg` 의 값, Tick 의 AutoOpening 끝 state 전환 로직 |
| 세션 자동 reset 안 됨 | ConfirmSession 의 끝에서 ResetState() 호출, OnPhaseChanged broadcast |

해결 안 되는 이슈 → `Docs/lessons_learned/<topic>.md`.

---

## 12. Phase 3 완료 후 회고

Phase 4 진입 전 회고:

1. Vehicle swap 시점 / fresh state vs in-progress 유지 — 평가자 피드백 따라 재검토
2. Comparison click = 즉시 결정 UX — 실수 빈도 / iteration 필요한지
3. Final Ranking Reset 사용 빈도 — UX 개선 여지
4. JSON schema — 평가 분석 시 부족한 필드 / 필요한 추가 필드
5. Power-Hybrid 의 handoff 자연스러움
6. 4 mode 의 평가자 첫 시도 시 어색함

회고 결과는 `Phase4_Kickoff.md` 작성 시 반영.

---

## 13. Claude Code 작업 원칙

Phase 1/2 와 동일.

- Commit format: `phase3: <work_item> <action>`
- 영어 commit message
- 함정 / 우회는 `Docs/lessons_learned/` 즉시 박제
