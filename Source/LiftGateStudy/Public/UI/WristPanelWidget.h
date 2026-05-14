// Copyright Notice: Internal evaluation tool. GMTCK PQDQ.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WristPanelWidget.generated.h"

/**
 * 좌측 손목 Panel UI.
 * - 손바닥이 위로 향할 때 (Up vector vs World Up dot product >= threshold) Visible
 * - HMD Height (mm), Floor 상태 (✓ / ✗) 표시
 * - Recalibrate 버튼: BP child 의 OnClicked → OnRecalibrateClicked() → delegate broadcast
 *
 * 손 방향 정보는 BP_VRPawn 이 매 frame SetHandUpVector() 로 주입 (C++ 가 hand mesh 를
 * 직접 알지 못하게 분리 — ADR-005, dependency 단방향).
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRecalibrateRequestedSignature);

UCLASS(Abstract, BlueprintType, Blueprintable)
class LIFTGATESTUDY_API UWristPanelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// ─────────────────────────────────────────────────────────────────────────
	// Tunable parameters
	// ─────────────────────────────────────────────────────────────────────────

	// 손바닥 위 방향 임계값. dot >= threshold 이면 Visible
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	float WristVisibleDotThreshold = 0.5f;

	// ─────────────────────────────────────────────────────────────────────────
	// Runtime 상태 (UMG Bind 에서 참조)
	// ─────────────────────────────────────────────────────────────────────────

	UPROPERTY(BlueprintReadOnly, Category = "UI")
	float HMDHeight_mm = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "UI")
	bool bFloorOK = false;

	// ─────────────────────────────────────────────────────────────────────────
	// External update API — BP_VRPawn 이 Tick 에서 호출
	// ─────────────────────────────────────────────────────────────────────────

	// 손바닥 방향 (mesh 의 up vector world-space). BP_VRPawn 이 매 frame 제공
	UFUNCTION(BlueprintCallable, Category = "UI")
	void SetHandUpVector(const FVector& InHandUp);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void SetCalibrationSnapshot(float InHMDHeight_mm, bool bInFloorOK);

	// ─────────────────────────────────────────────────────────────────────────
	// Recalibrate flow
	// ─────────────────────────────────────────────────────────────────────────

	UPROPERTY(BlueprintAssignable, Category = "UI")
	FOnRecalibrateRequestedSignature OnRecalibrateRequested;

	// BP child 의 Recalibrate Button OnClicked 에서 호출
	UFUNCTION(BlueprintCallable, Category = "UI")
	void OnRecalibrateClicked();

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// 가장 최근 SetHandUpVector 값 캐시
	UPROPERTY(Transient)
	FVector CachedHandUp = FVector::ZeroVector;
};
