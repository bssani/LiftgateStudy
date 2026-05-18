// Copyright Notice: Internal evaluation tool. GMTCK PQDQ.

#include "UI/ComparisonWidget.h"

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

void UComparisonWidget::OnChoice1Clicked()
{
	if (UEvaluationSessionSubsystem* Session = GetSession(this))
	{
		Session->RequestComparisonChoice(0);
	}
}

void UComparisonWidget::OnChoice2Clicked()
{
	if (UEvaluationSessionSubsystem* Session = GetSession(this))
	{
		Session->RequestComparisonChoice(1);
	}
}
