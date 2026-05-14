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
 * - Status==Pass 에서 Pinch 입력 시 widget->RequestComplete() 호출 → HandleComplete → Destroy
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

	// (선택) Pinch 입력 binding 을 BP 에서 ISDK Enhanced Input action 으로 처리할 때 호출
	// CLAUDE.md R5 / ADR-001: 자체 pinch 작성 금지. ISDK input action 만 사용
	UFUNCTION(BlueprintCallable, Category = "Calibration")
	void HandlePinchInput();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleComplete();

	// BeginPlay 에서 캐싱한 widget instance (Pinch 핸들러에서 재사용)
	UPROPERTY(Transient)
	TObjectPtr<UCalibrationCheckWidget> CachedWidget;
};
