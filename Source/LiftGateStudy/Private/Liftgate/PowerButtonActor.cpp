// Copyright Notice: Internal evaluation tool. GMTCK PQDQ.

#include "Liftgate/PowerButtonActor.h"

#include "Components/WidgetComponent.h"
#include "Liftgate/Liftgate.h"

APowerButtonActor::APowerButtonActor()
{
	PrimaryActorTick.bCanEverTick = false;

	ButtonWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("ButtonWidgetComp"));
	RootComponent = ButtonWidgetComp;

	// World-space (R6 / ADR-008). 평가자 proximity 내 배치 (R8 / §8)
	ButtonWidgetComp->SetWidgetSpace(EWidgetSpace::World);
	ButtonWidgetComp->SetDrawSize(FVector2D(100.f, 100.f));
}

void APowerButtonActor::OnButtonClicked()
{
	if (TargetLiftgate)
	{
		TargetLiftgate->RequestPowerOpen();
	}
}
