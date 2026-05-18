// Copyright Notice: Internal evaluation tool. GMTCK PQDQ.

#include "UI/TestProgressWidget.h"

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

int32 UTestProgressWidget::GetCurrentTestNumber() const
{
	if (UEvaluationSessionSubsystem* Session = GetSession(this))
	{
		// 'Slot' 은 UWidget::Slot (base class member) 와 이름 충돌 → SlotIndex 사용
		const int32 SlotIndex = Session->GetCurrentSlotIndex();
		return (SlotIndex >= 0) ? (SlotIndex + 1) : 0;
	}
	return 0;
}

bool UTestProgressWidget::ShouldShowPrevious() const
{
	if (UEvaluationSessionSubsystem* Session = GetSession(this))
	{
		const EEvaluationPhase Phase = Session->GetCurrentPhase();
		return Phase == EEvaluationPhase::Test2 || Phase == EEvaluationPhase::Test4;
	}
	return false;
}

void UTestProgressWidget::OnNextClicked()
{
	if (UEvaluationSessionSubsystem* Session = GetSession(this))
	{
		Session->RequestNext();
	}
}

void UTestProgressWidget::OnPreviousClicked()
{
	if (UEvaluationSessionSubsystem* Session = GetSession(this))
	{
		Session->RequestPrevious();
	}
}
