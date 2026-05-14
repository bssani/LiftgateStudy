// Copyright Notice: Internal evaluation tool. GMTCK PQDQ.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Calibration/CalibrationTypes.h"
#include "CalibrationCheckWidget.generated.h"

/**
 * Calibration 검증 위젯.
 * - CLAUDE.md §7 의 강제 검증 로직 (HMD height + floor ray cast) 을 매 frame 평가
 * - 평가 결과를 ECalibrationStatus / FailReason 으로 노출
 * - Pass 상태에서 외부 (ACalibrationGateActor 의 Pinch input 핸들러 등) 가 RequestComplete()
 *   호출 시 OnCalibrationComplete delegate 발행
 *
 * BP child (WBP_CalibrationCheck) 는 UMG 레이아웃만 담당. 로직 변경 금지 (ADR-005).
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCalibrationCompleteSignature);

UCLASS(Abstract, BlueprintType, Blueprintable)
class LIFTGATESTUDY_API UCalibrationCheckWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// ─────────────────────────────────────────────────────────────────────────
	// Tunable parameters (BP child 의 Class Defaults 에서 조정)
	// CLAUDE.md §6 Magic Number 예시 기본값 사용
	// ─────────────────────────────────────────────────────────────────────────

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calibration")
	float HMDHeightMin_mm = 1400.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calibration")
	float HMDHeightMax_mm = 2000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calibration")
	float FloorZTolerance_mm = 5.f;

	// Floor ray cast 의 거리 (cm). HMD 아래로 이 거리만큼 trace
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calibration")
	float FloorRayLength_cm = 300.f;

	// ─────────────────────────────────────────────────────────────────────────
	// Runtime 상태 (UMG Bind 함수에서 참조)
	// ─────────────────────────────────────────────────────────────────────────

	UPROPERTY(BlueprintReadOnly, Category = "Calibration")
	float HMDHeight_mm = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Calibration")
	float FloorZ_mm = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Calibration")
	ECalibrationStatus Status = ECalibrationStatus::Checking;

	UPROPERTY(BlueprintReadOnly, Category = "Calibration")
	FText FailReason;

	// ─────────────────────────────────────────────────────────────────────────
	// Event Dispatcher — BP_CalibrationGate 가 바인딩하여 widget 종료 / actor destroy 처리
	// ─────────────────────────────────────────────────────────────────────────

	UPROPERTY(BlueprintAssignable, Category = "Calibration")
	FOnCalibrationCompleteSignature OnCalibrationComplete;

	// Status == Pass 일 때만 OnCalibrationComplete 발행. Pinch input 핸들러에서 호출.
	UFUNCTION(BlueprintCallable, Category = "Calibration")
	void RequestComplete();

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// HMD location 측정 + floor ray cast + Status / FailReason 갱신
	void EvaluateOnce();
};
