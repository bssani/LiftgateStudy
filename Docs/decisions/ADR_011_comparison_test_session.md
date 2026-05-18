# ADR-011 — Comparison-based Test Session

**Date**: 2026-05-18
**Status**: Accepted
**Phase**: 3

## Context

Phase 3 의 평가 흐름이 단순 "4 차량 순차 체험" 이 아니라 **paired comparison + final ranking** 으로 확정:

1. Test 1 체험
2. Test 2 체험 (Test 1 ↔ Test 2 자유 토글)
3. Comparison 1 vs 2 — 더 나은 것 즉시 선택
4. Test 3 체험
5. Test 4 체험 (Test 3 ↔ Test 4 자유 토글)
6. Final Ranking — 1~4 차량을 선호 순서대로 click
7. JSON 저장 + 세션 자동 리셋

(Phase 3 와 4 사이의 비교 widget 은 **없음** — 사용자 spec)

흐름 / 상태 관리를 어디에 둘지 결정 필요:

| Option | 방식 | 평가 |
|---|---|---|
| A | GameMode 안에 모두 | GameMode 는 level 종속. Map 바뀌면 reset |
| **B** | **UGameInstanceSubsystem** | level / map 독립, single instance |
| C | UObject manager + GameMode 가 owns | 부가 작업 |
| D | BP-only state machine | 디버그 / git diff 어려움 (ADR-005 위반) |

## Decision

**Option B 채택**. `UEvaluationSessionSubsystem (UGameInstanceSubsystem)` 가 세션 전체 상태 머신 책임.

### 상태 머신

```
Idle
  │ StartSession(TestSet)
  ▼
Test1Active   ──Next──▶   Test2Active   ──Next──▶   Compare1v2
  ▲                          │                       │
  │                       Previous                   Click1│Click2
  │                          │                       ▼
  └──────────────────────────┘                  Test3Active   ──Next──▶   Test4Active   ──Next──▶   FinalRanking
                                                                              ▲                       │
                                                                              │                    AllPicked
                                                                          Previous                    │
                                                                              │                       ▼
                                                                              └────────────────  WritingLog
                                                                                                       │
                                                                                                  Complete
                                                                                                       │
                                                                                                  AutoReset
                                                                                                       │
                                                                                                       ▼
                                                                                                     Idle
```

### 주요 API (Subsystem 측)

```cpp
UCLASS()
class LIFTGATESTUDY_API UEvaluationSessionSubsystem : public UGameInstanceSubsystem
{
    UFUNCTION(BlueprintCallable) void StartSession(UVehicleTestSet* TestSet);

    // Test 1 / 2 / 3 / 4 phase 에서 Next/Previous 누름
    UFUNCTION(BlueprintCallable) void RequestNext();
    UFUNCTION(BlueprintCallable) void RequestPrevious();

    // Comparison phase 에서 클릭 (winnerIndex: 0=첫번째, 1=두번째)
    UFUNCTION(BlueprintCallable) void RequestComparisonChoice(int32 LocalWinnerIndex);

    // Final ranking phase 에서 순서대로 클릭
    UFUNCTION(BlueprintCallable) void AddRanking(int32 VehicleIndex);   // 0~3
    UFUNCTION(BlueprintCallable) void ResetRanking();
    UFUNCTION(BlueprintCallable) bool IsRanked(int32 VehicleIndex) const;
    UFUNCTION(BlueprintCallable) int32 GetRankOrder(int32 VehicleIndex) const;  // 1~4, 0=unranked

    // 세션 종료 / 자동 리셋
    UFUNCTION(BlueprintCallable) void ConfirmSession();

    // 현재 상태 query
    UFUNCTION(BlueprintCallable, BlueprintPure) EEvaluationPhase GetCurrentPhase() const;

    // Widget 측 binding
    UPROPERTY(BlueprintAssignable) FOnPhaseChangedSignature OnPhaseChanged;
};
```

### Vehicle swap 동작

Phase 전환 시 spawn 된 BP_Liftgate Actor 를 **destroy → spawn** 으로 교체:
- 평가자가 grab 했더라도 fresh 상태 부여 (각 차량 처음부터 체험)
- Test 1 ↔ Test 2 토글 시도 동일 — 빠른 fresh swap

Spawn 위치는 L_Main 의 SpawnAnchor (PlayerStart 정면 등) + `FVehicleTestEntry::SpawnRelativeTransform`.

### UI 측 (widget 3 종)

| Widget | 표시 | 액션 |
|---|---|---|
| `UTestProgressWidget` | "1번 테스트" ~ "4번 테스트" + Next / Previous (조건부) | RequestNext, RequestPrevious |
| `UComparisonWidget` | "1 vs 2" 두 버튼 | RequestComparisonChoice |
| `UFinalRankingWidget` | 4 버튼 + pressed-state lock + Reset 버튼 | AddRanking, ResetRanking, ConfirmSession |

각 widget 은 OnPhaseChanged 구독해서 자동 visible / hidden. ADR-007 (button poke), ADR-008 (World-space) 준수.

### Final Ranking UX

- 4 버튼 모두 초기 enabled
- 클릭 시:
  - 해당 vehicle 의 Rank = 다음 빈 슬롯 (1, 2, 3, 4 순)
  - 버튼은 disabled (pressed state) + 순서 숫자 (1~4) 시각 표시
- 모두 클릭 (4 개) 시:
  - `ConfirmSession()` 호출 (또는 "Confirm" 버튼 노출 후 click)
  - **결정**: 모든 4 버튼 click 시 자동 confirm (별도 confirm 버튼 없음 — 1 step less)
- 실수 정정: **Reset** 버튼 — 4 버튼 모두 enabled 로 복원, ranking 비움. 다시 처음부터

## Consequences

**Positive**:
- 상태 머신 분리 — UI 가 단순, 로직이 테스트 가능
- Subsystem 은 level 독립 → L_Main 외 다른 level 추가 시에도 호환
- 평가 세션 자동 재시작 → continuous evaluation 가능 (한 평가자 끝 → 다음 평가자 바로)

**Negative**:
- 4 test hardcoded (Phase 3 가정). ADR-010 의 UDataAsset 도 4 entry 가정 — 동기화 필요
- BP_Liftgate Actor lifetime 책임이 subsystem 에 — destroy/spawn 시점 정확해야 (memory leak 방지)
- Comparison click = 즉시 결정 → 평가자가 실수하기 쉬움. UX iteration 필요할 수 있음

**Neutral**:
- 1 ↔ 2 토글 시 vehicle reset 됨. "in-progress 상태 유지" 는 안 함 (의도). 매 토글마다 fresh experience
- Anonymous (ADR-006) — Subsystem 은 user identity 추적 안 함

## Notes

- Phase 4 에서 paired comparison 횟수 / 흐름 변경 시 본 ADR supersede
- 평가자가 도중 quit 하면 미저장 상태로 종료 — 자동 reset 으로 다음 평가자 영향 없음
- Logging 형식 / 위치는 ADR-006 참조
