// Copyright Notice: Internal evaluation tool. GMTCK PQDQ.

#include "Calibration/CalibrationCheckWidget.h"

#include "Camera/PlayerCameraManager.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

namespace
{
	// CLAUDE.md §6: UE 는 cm, 우리 spec 은 mm. 변환 상수 한 곳에 모음
	constexpr float kCmToMm = 10.f;
}

void UCalibrationCheckWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// 매 frame 평가. 부담되면 향후 FTimerHandle 0.1s 로 전환 (CLAUDE.md §6)
	EvaluateOnce();
}

void UCalibrationCheckWidget::EvaluateOnce()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	APlayerCameraManager* CamMgr = UGameplayStatics::GetPlayerCameraManager(this, 0);
	if (!CamMgr)
	{
		// Player camera 가 아직 준비 안 됨 (PIE 초기 frame 등)
		return;
	}

	// ── 1) HMD height 측정 (R1: Tracking Origin = Floor Level 이어야 의미 있음) ──
	const FVector CamLoc = CamMgr->GetCameraLocation();
	HMDHeight_mm = CamLoc.Z * kCmToMm;

	// ── 2) Floor ray cast (HMD 아래 방향) ──
	const FVector TraceStart = CamLoc;
	const FVector TraceEnd = CamLoc - FVector(0.f, 0.f, FloorRayLength_cm);

	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(CalibrationFloorTrace), /*bTraceComplex=*/ false);
	Params.AddIgnoredActor(GetOwningPlayerPawn());

	const bool bHit = World->LineTraceSingleByChannel(
		Hit, TraceStart, TraceEnd, ECC_Visibility, Params);

	// ── 3) 판정 ──
	const bool bHMDInRange = (HMDHeight_mm >= HMDHeightMin_mm) && (HMDHeight_mm <= HMDHeightMax_mm);

	bool bFloorOK = false;
	if (bHit)
	{
		FloorZ_mm = Hit.Location.Z * kCmToMm;
		bFloorOK = FMath::Abs(FloorZ_mm) <= FloorZTolerance_mm;
	}
	else
	{
		// Floor 가 검출되지 않음 — 평가자 발 아래에 collision-enabled mesh 없음
		FloorZ_mm = 0.f;
		bFloorOK = false;
	}

	// ── 4) Status / FailReason 갱신 ──
	if (bHMDInRange && bFloorOK)
	{
		Status = ECalibrationStatus::Pass;
		FailReason = FText::GetEmpty();
	}
	else
	{
		Status = ECalibrationStatus::Fail;
		if (HMDHeight_mm < HMDHeightMin_mm)
		{
			FailReason = NSLOCTEXT("Calibration", "FailSitting",
				"You may be sitting. Please stand.");
		}
		else if (HMDHeight_mm > HMDHeightMax_mm)
		{
			FailReason = NSLOCTEXT("Calibration", "FailHMDHigh",
				"Calibration may be off. Re-do Quest boundary setup.");
		}
		else
		{
			FailReason = NSLOCTEXT("Calibration", "FailFloor",
				"Floor not detected at Z=0. Re-do boundary.");
		}
	}
}

void UCalibrationCheckWidget::RequestComplete()
{
	// Pass 가 아니면 무시 (안전장치)
	if (Status != ECalibrationStatus::Pass)
	{
		return;
	}
	OnCalibrationComplete.Broadcast();
}
