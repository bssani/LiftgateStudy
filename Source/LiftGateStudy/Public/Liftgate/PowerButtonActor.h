// Copyright Notice: Internal evaluation tool. GMTCK PQDQ.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PowerButtonActor.generated.h"

class ALiftgate;
class UWidgetComponent;

/**
 * Power-Auto / Power-Hybrid mode liftgate 의 trigger 버튼.
 * 차량 옆 World-space 에 배치되어 평가자가 손가락으로 poke (R8 / ADR-007).
 *
 * BP child (BP_PowerButton) 가 wiring:
 * - ButtonWidgetComp.WidgetClass 에 간단한 button widget (WBP)
 * - widget 의 OnClicked → Self.OnButtonClicked() UFUNCTION 호출
 *
 * Spawn 시 또는 Editor 에서 TargetLiftgate reference 가 set 되어 있어야 동작.
 */
UCLASS(Blueprintable)
class LIFTGATESTUDY_API APowerButtonActor : public AActor
{
	GENERATED_BODY()

public:
	APowerButtonActor();

	// World-space button widget host
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowerButton")
	TObjectPtr<UWidgetComponent> ButtonWidgetComp;

	// 클릭 시 RequestPowerOpen 호출 대상.
	// Subsystem 이 spawn 시 set, 또는 Level 에서 수동 set.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerButton")
	TObjectPtr<ALiftgate> TargetLiftgate;

	// BP child 의 Button OnClicked 가 호출
	UFUNCTION(BlueprintCallable, Category = "PowerButton")
	void OnButtonClicked();
};
