// Copyright Notice: Internal evaluation tool. GMTCK PQDQ.

#include "Liftgate/Liftgate.h"

#include "Components/StaticMeshComponent.h"

ALiftgate::ALiftgate()
{
	// AutoOpening lerp 가 Tick 에서 동작하므로 활성화
	PrimaryActorTick.bCanEverTick = true;

	// 회전 안 하는 anchor (Meta ISDK GrabbableBox sample 패턴, ADR-009).
	// root 가 회전 안 해야 HingePivot 의 RelativeLocation/Rotation 이 BP 에서 편집 가능.
	USceneComponent* DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent = DefaultSceneRoot;

	// Hinge — child 라서 Location / Rotation 편집 가능. ISDK GrabTransformer 가
	// 이 component 를 회전 대상으로 사용 (IsdkGrabbable 을 HingePivot 자식으로 두면).
	HingePivot = CreateDefaultSubobject<USceneComponent>(TEXT("HingePivot"));
	HingePivot->SetupAttachment(RootComponent);

	// Mesh — Hinge 자식, hinge 회전에 따라 swing
	LiftgateMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LiftgateMesh"));
	LiftgateMesh->SetupAttachment(HingePivot);
}

void ALiftgate::BeginGrab(const FVector& HandWorldLocation)
{
	// Phase 3: ISDK GrabTransformer 가 회전을 처리하므로 본 함수는 대부분 미사용.
	// 향후 legacy / fallback 또는 Power-Hybrid 의 grab 추적 용도로 남겨둠.
	if (State == ELiftgateState::AutoOpening || State == ELiftgateState::Open)
	{
		return;
	}

	State = ELiftgateState::Grabbed;
	GrabStartAngle_deg = CurrentAngle_deg;
	GrabStartHandLocation = HandWorldLocation;
}

void ALiftgate::UpdateGrab(const FVector& HandWorldLocation)
{
	// Phase 3: ISDK GrabTransformer 가 회전 처리. legacy 미사용.
	if (State != ELiftgateState::Grabbed)
	{
		return;
	}

	const float DeltaZ_cm = HandWorldLocation.Z - GrabStartHandLocation.Z;
	const float DeltaAngle_deg = DeltaZ_cm * GrabSensitivity_deg_per_cm;
	const float NewAngle = FMath::Clamp(
		GrabStartAngle_deg + DeltaAngle_deg, 0.f, MaxAngle_deg);

	SetAngle(NewAngle);
}

void ALiftgate::EndGrab()
{
	// Phase 3: ISDK GrabTransformer 가 HingePivot 을 직접 회전시킨 상태.
	// 현재 각도를 hinge 의 RelativeRotation 에서 읽어 CurrentAngle_deg 동기화.
	SyncAngleFromHinge();

	// PowerAuto 는 grab event 무시 (button 만 사용). 들어와도 무시
	if (CurrentMode == ELiftgateMode::PowerAuto)
	{
		return;
	}

	// Manual-Assist 또는 Power-Hybrid 의 manual 단계 (handoff 이후):
	// ThresholdAngle 초과 시 자동 완전 개방
	const bool bAssistEligible =
		(CurrentMode == ELiftgateMode::ManualAssist) ||
		(CurrentMode == ELiftgateMode::PowerHybrid &&
		 CurrentAngle_deg >= PowerHybridHandoffAngle_deg);

	if (bAssistEligible && CurrentAngle_deg >= ThresholdAngle_deg)
	{
		State = ELiftgateState::AutoOpening;
		AutoOpenElapsed_sec = 0.f;
		AutoOpenStartAngle_deg = CurrentAngle_deg;
		AutoOpenTargetAngle_deg = MaxAngle_deg;
	}
	else
	{
		// Manual-Full, threshold 미만 등 → 그 자리 멈춤
		State = ELiftgateState::Closed;
	}
}

void ALiftgate::RequestPowerOpen()
{
	if (State == ELiftgateState::AutoOpening || State == ELiftgateState::Open)
	{
		// 이미 진행 중이면 무시 (중복 클릭 방지)
		return;
	}

	if (CurrentMode == ELiftgateMode::PowerAuto)
	{
		// 끝까지 자동 개방
		State = ELiftgateState::AutoOpening;
		AutoOpenElapsed_sec = 0.f;
		AutoOpenStartAngle_deg = CurrentAngle_deg;
		AutoOpenTargetAngle_deg = MaxAngle_deg;
	}
	else if (CurrentMode == ELiftgateMode::PowerHybrid)
	{
		// Handoff 각도까지만 자동, 그 다음 평가자가 grab
		State = ELiftgateState::AutoOpening;
		AutoOpenElapsed_sec = 0.f;
		AutoOpenStartAngle_deg = CurrentAngle_deg;
		AutoOpenTargetAngle_deg = FMath::Min(PowerHybridHandoffAngle_deg, MaxAngle_deg);
	}
	// Manual* mode 는 Power button 자체가 없거나 무시
}

void ALiftgate::ResetLiftgate()
{
	State = ELiftgateState::Closed;
	AutoOpenElapsed_sec = 0.f;
	SetAngle(0.f);
}

void ALiftgate::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (State != ELiftgateState::AutoOpening)
	{
		return;
	}

	// 0 / 음수 duration 보호 (UPROPERTY 가 BP 에서 잘못 셋업될 수 있음)
	if (AutoOpenDuration_sec <= 0.f)
	{
		SetAngle(AutoOpenTargetAngle_deg);
		// PowerHybrid 의 handoff 도달 시 grab 받는 idle 로 복귀.
		// PowerAuto / ManualAssist 의 MaxAngle 도달 시 Open 으로.
		const bool bIsHybridHandoff =
			(CurrentMode == ELiftgateMode::PowerHybrid &&
			 AutoOpenTargetAngle_deg < MaxAngle_deg);
		State = bIsHybridHandoff ? ELiftgateState::Closed : ELiftgateState::Open;
		return;
	}

	AutoOpenElapsed_sec += DeltaTime;
	const float Alpha = FMath::Clamp(
		AutoOpenElapsed_sec / AutoOpenDuration_sec, 0.f, 1.f);
	const float NewAngle = FMath::Lerp(
		AutoOpenStartAngle_deg, AutoOpenTargetAngle_deg, Alpha);
	SetAngle(NewAngle);

	if (Alpha >= 1.f)
	{
		// PowerHybrid 의 handoff 도달 → grab 대기 (Closed = idle).
		// 그 외 (PowerAuto, ManualAssist) → Open
		const bool bIsHybridHandoff =
			(CurrentMode == ELiftgateMode::PowerHybrid &&
			 AutoOpenTargetAngle_deg < MaxAngle_deg);
		State = bIsHybridHandoff ? ELiftgateState::Closed : ELiftgateState::Open;
	}
}

void ALiftgate::SetAngle(float NewAngle_deg)
{
	CurrentAngle_deg = NewAngle_deg;

	// HingePivot 의 local Pitch 회전 (음수 = 위로 열림, UE 기본 컨벤션).
	// 차량 hinge 축이 좌우 (Y) 일 때 작동. ISDK GrabTransformer 의 axis 설정과 일치 필요.
	HingePivot->SetRelativeRotation(FRotator(-CurrentAngle_deg, 0.f, 0.f));
}

void ALiftgate::SyncAngleFromHinge()
{
	// ISDK GrabTransformer 가 HingePivot 을 회전시킴. Pitch 가 음수 (위로 열림 컨벤션).
	const FRotator HingeRot = HingePivot->GetRelativeRotation();
	CurrentAngle_deg = FMath::Abs(HingeRot.Pitch);
}
