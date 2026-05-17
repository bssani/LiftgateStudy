// Copyright Notice: Internal evaluation tool. GMTCK PQDQ.

#include "Calibration/CalibrationGateActor.h"

#include "Calibration/CalibrationCheckWidget.h"
#include "Components/WidgetComponent.h"

ACalibrationGateActor::ACalibrationGateActor()
{
	PrimaryActorTick.bCanEverTick = false;

	CalibrationWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("CalibrationWidgetComp"));
	RootComponent = CalibrationWidgetComp;

	// World-space VR widget default (R6: HUD 금지)
	CalibrationWidgetComp->SetWidgetSpace(EWidgetSpace::World);
	CalibrationWidgetComp->SetDrawSize(FVector2D(400.f, 300.f));
	// Widget Class 는 BP child (BP_CalibrationGate) 에서 WBP_CalibrationCheck 로 지정
}

void ACalibrationGateActor::BeginPlay()
{
	Super::BeginPlay();

	if (!CalibrationWidgetComp)
	{
		return;
	}

	// WidgetComponent 가 World-space 일 때 user widget instance 는 BeginPlay 후 1 tick 이내 생성됨.
	// 즉시 가져와 캐시. nullptr 이면 Tick 후 재시도 패턴이 필요할 수 있음 (향후 lessons_learned).
	UUserWidget* UserWidget = CalibrationWidgetComp->GetUserWidgetObject();
	CachedWidget = Cast<UCalibrationCheckWidget>(UserWidget);

	if (CachedWidget)
	{
		CachedWidget->OnCalibrationComplete.AddDynamic(this, &ACalibrationGateActor::HandleComplete);
	}
}

void ACalibrationGateActor::HandleConfirm()
{
	// BP 의 Confirm 버튼 OnClicked → 본 UFUNCTION 으로 라우팅 (R8 / ADR-007)
	if (CachedWidget)
	{
		CachedWidget->RequestComplete();
	}
}

void ACalibrationGateActor::HandleComplete()
{
	// CLAUDE.md §7: 검증 통과 → 평가 진입. Phase 1 에서는 Gate 자신만 정리
	Destroy();
}
