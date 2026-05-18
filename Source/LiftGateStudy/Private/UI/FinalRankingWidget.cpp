// Copyright Notice: Internal evaluation tool. GMTCK PQDQ.

#include "UI/FinalRankingWidget.h"

#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Session/EvaluationSessionSubsystem.h"

namespace
{
	UEvaluationSessionSubsystem* GetSession(const UWidget* WorldContext)
	{
		if (!WorldContext) return nullptr;
		UGameInstance* GI = UGameplayStatics::GetGameInstance(WorldContext);
		return GI ? GI->GetSubsystem<UEvaluationSessionSubsystem>() : nullptr;
	}
}

void UFinalRankingWidget::OnRankButtonClicked(int32 SlotIndex)
{
	if (UEvaluationSessionSubsystem* Session = GetSession(this))
	{
		Session->AddRanking(SlotIndex);
	}
}

void UFinalRankingWidget::OnResetClicked()
{
	if (UEvaluationSessionSubsystem* Session = GetSession(this))
	{
		Session->ResetRanking();
	}
}

bool UFinalRankingWidget::IsButtonEnabled(int32 SlotIndex) const
{
	if (UEvaluationSessionSubsystem* Session = GetSession(this))
	{
		return !Session->IsRanked(SlotIndex);
	}
	return true;
}

FText UFinalRankingWidget::GetRankLabel(int32 SlotIndex) const
{
	if (UEvaluationSessionSubsystem* Session = GetSession(this))
	{
		const int32 Order = Session->GetRankOrder(SlotIndex);
		if (Order > 0)
		{
			return FText::AsNumber(Order);
		}
	}
	return FText::GetEmpty();
}
