// Copyright Notice: Internal evaluation tool. GMTCK PQDQ.

#include "UI/SessionUIWidget.h"

#include "Components/WidgetSwitcher.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"

void USessionUIWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// Subsystem cache + OnPhaseChanged subscribe (1 회, ADR-013)
	if (UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this))
	{
		CachedSubsystem = GameInstance->GetSubsystem<UEvaluationSessionSubsystem>();
		if (CachedSubsystem)
		{
			CachedSubsystem->OnPhaseChanged.AddDynamic(this, &USessionUIWidget::HandlePhaseChanged);

			// 초기 phase 로 visibility / switcher 동기화 (Subsystem 이 widget 보다 먼저 시작했을 경우)
			HandlePhaseChanged(CachedSubsystem->GetCurrentPhase());
		}
	}
}

void USessionUIWidget::NativeDestruct()
{
	if (CachedSubsystem)
	{
		CachedSubsystem->OnPhaseChanged.RemoveDynamic(this, &USessionUIWidget::HandlePhaseChanged);
	}
	Super::NativeDestruct();
}

void USessionUIWidget::HandlePhaseChanged(EEvaluationPhase NewPhase)
{
	// Idle / WritingLog 는 widget 자체를 hide. 나머지는 switcher index 매핑.
	switch (NewPhase)
	{
	case EEvaluationPhase::Test1:
	case EEvaluationPhase::Test2:
	case EEvaluationPhase::Test3:
	case EEvaluationPhase::Test4:
		SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		if (SessionSwitcher)
		{
			SessionSwitcher->SetActiveWidgetIndex(SlotIndex_TestProgress);
		}
		break;

	case EEvaluationPhase::Compare1v2:
		SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		if (SessionSwitcher)
		{
			SessionSwitcher->SetActiveWidgetIndex(SlotIndex_Comparison);
		}
		break;

	case EEvaluationPhase::FinalRanking:
		SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		if (SessionSwitcher)
		{
			SessionSwitcher->SetActiveWidgetIndex(SlotIndex_FinalRanking);
		}
		break;

	case EEvaluationPhase::Idle:
	case EEvaluationPhase::WritingLog:
	default:
		SetVisibility(ESlateVisibility::Collapsed);
		break;
	}
}
