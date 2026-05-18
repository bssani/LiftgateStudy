// Copyright Notice: Internal evaluation tool. GMTCK PQDQ.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FinalRankingWidget.generated.h"

/**
 * FinalRanking phase 의 widget (ADR-011).
 * - 4 vehicle slot 버튼 (0~3) — 선호 순서로 클릭
 * - 클릭 시 해당 버튼 disabled (pressed state, rank order 표시)
 * - Reset 버튼 — 모두 enabled 로 복원, ranking 비움
 * - 4 개 모두 클릭 시 자동 confirm (Subsystem 이 JSON write + 세션 reset)
 *
 * BP child (WBP_FinalRanking) wiring:
 * - 각 vehicle 버튼 OnClicked → OnRankButtonClicked(slot)
 * - 각 버튼의 IsEnabled binding → IsButtonEnabled(slot)
 * - 각 버튼의 label binding → GetButtonLabel(slot) (예: "[2]" rank 2 면)
 * - Reset OnClicked → OnResetClicked
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class LIFTGATESTUDY_API UFinalRankingWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Slot 0~3 의 vehicle 을 ranking 에 추가
	UFUNCTION(BlueprintCallable, Category = "UI")
	void OnRankButtonClicked(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void OnResetClicked();

	// Widget bind 용: 해당 slot 이 아직 ranked 안 됐으면 true (button enabled)
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UI")
	bool IsButtonEnabled(int32 SlotIndex) const;

	// Widget bind 용: rank 표시 텍스트 (예: "1순위" / "2순위" / 아직이면 빈 문자열)
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UI")
	FText GetRankLabel(int32 SlotIndex) const;
};
