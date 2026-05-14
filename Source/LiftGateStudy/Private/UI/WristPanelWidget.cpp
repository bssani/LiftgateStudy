// Copyright Notice: Internal evaluation tool. GMTCK PQDQ.

#include "UI/WristPanelWidget.h"

void UWristPanelWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// 손바닥이 위로 향하면 Visible. 정규화 비용 절감 위해 SetHandUpVector 가 normalized 라 가정.
	const float Dot = FVector::DotProduct(CachedHandUp, FVector::UpVector);
	const ESlateVisibility Desired =
		(Dot >= WristVisibleDotThreshold) ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;

	if (GetVisibility() != Desired)
	{
		SetVisibility(Desired);
	}
}

void UWristPanelWidget::SetHandUpVector(const FVector& InHandUp)
{
	// 호출자 (BP_VRPawn) 가 정규화하지 않을 경우 대비
	CachedHandUp = InHandUp.GetSafeNormal();
}

void UWristPanelWidget::SetCalibrationSnapshot(float InHMDHeight_mm, bool bInFloorOK)
{
	HMDHeight_mm = InHMDHeight_mm;
	bFloorOK = bInFloorOK;
}

void UWristPanelWidget::OnRecalibrateClicked()
{
	OnRecalibrateRequested.Broadcast();
}
