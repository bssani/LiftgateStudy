// Copyright Notice: Internal evaluation tool. GMTCK PQDQ.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CalibrationGateActor.generated.h"

class UCalibrationCheckWidget;
class UWidgetComponent;

/**
 * Calibration widget 을 World-space 로 띄우는 host Actor.
 * - L_Main 에 1개 배치 (PlayerStart 정면 1.5m, Z=1.6m 권장)
 * - BeginPlay 시 widget instance 의 OnCalibrationComplete 에 self 의 HandleComplete 바인딩
 * - Status==Pass 에서 Confirm 버튼 poke → widget->RequestComplete() → HandleComplete → Destroy
 *
 * BP child (BP_CalibrationGate) 가 WidgetComponent 의 Widget Class 를
 * WBP_CalibrationCheck 로 지정. 위치는 Level 에서 평가자가 배치.
 */
UCLASS(Blueprintable)
class LIFTGATESTUDY_API ACalibrationGateActor : public AActor
{
	GENERATED_BODY()

public:
	ACalibrationGateActor();

	// World-space widget host. BP child 에서 Widget Class / Draw Size 지정
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Calibration")
	TObjectPtr<UWidgetComponent> CalibrationWidgetComp;

	// BP 의 Confirm 버튼 OnClicked → 본 UFUNCTION 으로 라우팅 (R8 / ADR-007)
	UFUNCTION(BlueprintCallable, Category = "Calibration")
	void HandleConfirm();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleComplete();

	// BeginPlay 에서 캐싱한 widget instance (Confirm 핸들러에서 재사용)
	UPROPERTY(Transient)
	TObjectPtr<UCalibrationCheckWidget> CachedWidget;
};
