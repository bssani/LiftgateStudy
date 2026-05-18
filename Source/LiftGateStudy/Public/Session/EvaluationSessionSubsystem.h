// Copyright Notice: Internal evaluation tool. GMTCK PQDQ.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EvaluationSessionSubsystem.generated.h"

class ALiftgate;
class UVehicleTestSet;

/**
 * 평가 세션 진행 단계 (ADR-011 상태 머신).
 */
UENUM(BlueprintType)
enum class EEvaluationPhase : uint8
{
	// 세션 시작 전 (또는 reset 직후)
	Idle         UMETA(DisplayName = "Idle"),
	Test1        UMETA(DisplayName = "Test 1"),
	Test2        UMETA(DisplayName = "Test 2"),
	Compare1v2   UMETA(DisplayName = "Compare 1 vs 2"),
	Test3        UMETA(DisplayName = "Test 3"),
	Test4        UMETA(DisplayName = "Test 4"),
	FinalRanking UMETA(DisplayName = "Final Ranking"),
	// JSON 파일 write 중 (1 frame 정도). 끝나면 자동 reset
	WritingLog   UMETA(DisplayName = "Writing Log")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnPhaseChangedSignature,
	EEvaluationPhase, NewPhase);

/**
 * 평가 세션 전체 상태 머신 (ADR-011).
 *
 * 책임:
 * - Phase 전환 (Test1→Test2→Compare1v2→Test3→Test4→FinalRanking→reset)
 * - Vehicle Actor spawn / destroy (Phase 전환 시)
 * - Comparison winner / final ranking 저장
 * - 세션 완료 시 JSON 결과 file write (ADR-006)
 * - 세션 자동 reset
 *
 * Level / Map 독립 (UGameInstanceSubsystem) — Map 바뀌어도 상태 유지.
 *
 * BP / Widget 측은 OnPhaseChanged 구독 + 본 클래스의 RequestNext/Previous/
 * RequestComparisonChoice/AddRanking/ResetRanking 호출.
 */
UCLASS(Blueprintable)
class LIFTGATESTUDY_API UEvaluationSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// ─────────────────────────────────────────────────────────────────────────
	// Session lifecycle
	// ─────────────────────────────────────────────────────────────────────────

	// Test 1 진입. Calibration 통과 후 GameMode 등이 호출.
	UFUNCTION(BlueprintCallable, Category = "EvaluationSession")
	void StartSession(UVehicleTestSet* InTestSet);

	// Test 1/2/3/4 phase 에서 사용
	UFUNCTION(BlueprintCallable, Category = "EvaluationSession")
	void RequestNext();

	UFUNCTION(BlueprintCallable, Category = "EvaluationSession")
	void RequestPrevious();

	// Comparison 1v2 에서 winner click (0 = test1 slot, 1 = test2 slot)
	UFUNCTION(BlueprintCallable, Category = "EvaluationSession")
	void RequestComparisonChoice(int32 LocalWinnerIndex);

	// Final ranking 단계: vehicle 의 slot index (0~3) 를 다음 ranking 슬롯에 추가
	UFUNCTION(BlueprintCallable, Category = "EvaluationSession")
	void AddRanking(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "EvaluationSession")
	void ResetRanking();

	// 현재 slot 이 ranked 됐는지 (widget 의 button enabled binding 용)
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "EvaluationSession")
	bool IsRanked(int32 SlotIndex) const;

	// Slot 의 rank order (1~4, 0=unranked). Widget 표시용.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "EvaluationSession")
	int32 GetRankOrder(int32 SlotIndex) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "EvaluationSession")
	EEvaluationPhase GetCurrentPhase() const { return CurrentPhase; }

	// Current test slot (0~3) — Test1/2/3/4 phase 에서만 의미 있음
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "EvaluationSession")
	int32 GetCurrentSlotIndex() const { return CurrentSlotIndex; }

	// L_Main 의 spawn anchor 를 register (GameMode BeginPlay 등에서 호출)
	UFUNCTION(BlueprintCallable, Category = "EvaluationSession")
	void SetSpawnAnchor(AActor* InAnchor) { SpawnAnchor = InAnchor; }

	// ─────────────────────────────────────────────────────────────────────────
	// Events
	// ─────────────────────────────────────────────────────────────────────────

	UPROPERTY(BlueprintAssignable, Category = "EvaluationSession")
	FOnPhaseChangedSignature OnPhaseChanged;

protected:
	// ─────────────────────────────────────────────────────────────────────────
	// State
	// ─────────────────────────────────────────────────────────────────────────

	UPROPERTY(Transient)
	EEvaluationPhase CurrentPhase = EEvaluationPhase::Idle;

	UPROPERTY(Transient)
	int32 CurrentSlotIndex = -1;   // 0~3 (Test 1~4), -1 if not in test phase

	UPROPERTY(Transient)
	TObjectPtr<UVehicleTestSet> ActiveTestSet;

	UPROPERTY(Transient)
	TObjectPtr<ALiftgate> ActiveLiftgate;

	UPROPERTY(Transient)
	TObjectPtr<AActor> SpawnAnchor;

	// Comparison 1v2 의 winner slot index (0 또는 1). -1 = 미선택
	UPROPERTY(Transient)
	int32 Comparison1v2WinnerSlot = -1;

	// Final ranking: order[N] = slot index. 길이 0~4.
	UPROPERTY(Transient)
	TArray<int32> FinalRankingSlots;

	// 세션 식별자 (UUID, anonymous per ADR-006)
	UPROPERTY(Transient)
	FString SessionId;

	UPROPERTY(Transient)
	FDateTime SessionStartTimeUtc;

	// ─────────────────────────────────────────────────────────────────────────
	// Helpers
	// ─────────────────────────────────────────────────────────────────────────

	void SetPhase(EEvaluationPhase NewPhase);
	void SpawnVehicleForSlot(int32 SlotIndex);
	void DestroyActiveLiftgate();

	// JSON write (ADR-006). Saved/EvaluationLogs/session_*.json
	void WriteSessionLogToDisk();

	// 모든 ranking 끝, JSON write 후 호출. Idle 로 reset 후 자동 StartSession 호출.
	void FinalizeAndAutoReset();
};
