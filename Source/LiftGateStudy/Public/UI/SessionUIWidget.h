// Copyright Notice: Internal evaluation tool. GMTCK PQDQ.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Session/EvaluationSessionSubsystem.h"
#include "SessionUIWidget.generated.h"

class UWidgetSwitcher;

/**
 * Phase 3 의 단일 session-level umbrella widget (ADR-013).
 *
 * Top-level widget 은 본 클래스 1 종 (`WBP_SessionUI`). 내부 WidgetSwitcher 가
 * 3 inner panel 을 phase 따라 전환:
 *   - slot 0 = TestProgress (Test 1~4)
 *   - slot 1 = Comparison (1 vs 2)
 *   - slot 2 = FinalRanking
 *
 * `UEvaluationSessionSubsystem::OnPhaseChanged` 를 1 회 subscribe.
 * Idle / WritingLog phase 에서는 자기 자신을 Collapsed 로.
 *
 * BP child (`WBP_SessionUI`) 가 wiring:
 * - `SessionSwitcher` 라는 이름의 `UWidgetSwitcher` 를 Designer 에 배치 (BindWidget)
 *   → switcher slot 0/1/2 에 각각 `WBP_TestProgress` / `WBP_Comparison` /
 *     `WBP_FinalRanking` instance 를 child widget 으로 추가
 *
 * Placement: `BP_SessionUIAnchor` (L_Main, ADR-013 D2) 의 WidgetComponent 가
 * `WBP_SessionUI` 를 표시. Pawn 우측 +50cm, 정면 +20cm, Z=120cm (chest height).
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class LIFTGATESTUDY_API USessionUIWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Designer 의 WidgetSwitcher 와 동일 이름으로 BindWidget (필수)
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "UI")
	TObjectPtr<UWidgetSwitcher> SessionSwitcher;

	// Phase → switcher slot index 매핑
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	int32 SlotIndex_TestProgress = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	int32 SlotIndex_Comparison = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	int32 SlotIndex_FinalRanking = 2;

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

	// OnPhaseChanged broadcast 시 호출 (UFUNCTION 필수 — AddDynamic 용)
	UFUNCTION()
	void HandlePhaseChanged(EEvaluationPhase NewPhase);

	// Subsystem reference (NativeOnInitialized 에서 1 회 cache)
	UPROPERTY(Transient)
	TObjectPtr<UEvaluationSessionSubsystem> CachedSubsystem;
};
