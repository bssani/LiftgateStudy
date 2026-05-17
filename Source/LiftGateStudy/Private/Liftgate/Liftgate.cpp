// Copyright Notice: Internal evaluation tool. GMTCK PQDQ.

#include "Liftgate/Liftgate.h"

#include "Components/StaticMeshComponent.h"

ALiftgate::ALiftgate()
{
	// AutoOpening lerp 가 Tick 에서 동작하므로 활성화
	PrimaryActorTick.bCanEverTick = true;

	HingePivot = CreateDefaultSubobject<USceneComponent>(TEXT("HingePivot"));
	RootComponent = HingePivot;

	LiftgateMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LiftgateMesh"));
	LiftgateMesh->SetupAttachment(HingePivot);
}

void ALiftgate::BeginGrab(const FVector& HandWorldLocation)
{
	// 이미 자동 개방 중이거나 완전 개방이면 추가 grab 무시 (Phase 2 단순화)
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
	if (State != ELiftgateState::Grabbed)
	{
		return;
	}

	// UE 좌표계 cm 단위. 손 수직 이동 → 각도 변화 (선형, Phase 2 MVP)
	const float DeltaZ_cm = HandWorldLocation.Z - GrabStartHandLocation.Z;
	const float DeltaAngle_deg = DeltaZ_cm * GrabSensitivity_deg_per_cm;
	const float NewAngle = FMath::Clamp(
		GrabStartAngle_deg + DeltaAngle_deg, 0.f, MaxAngle_deg);

	SetAngle(NewAngle);
}

void ALiftgate::EndGrab()
{
	if (State != ELiftgateState::Grabbed)
	{
		return;
	}

	// Manual-Assist + ThresholdAngle 초과 → 자동 완전 개방 (CLAUDE.md §9)
	if (CurrentMode == ELiftgateMode::ManualAssist
		&& CurrentAngle_deg >= ThresholdAngle_deg)
	{
		State = ELiftgateState::AutoOpening;
		AutoOpenElapsed_sec = 0.f;
		AutoOpenStartAngle_deg = CurrentAngle_deg;
	}
	else
	{
		// Phase 2 는 중력 / damping 없음. 그 자리 멈춤
		// (semantic: "actively grabbed" 가 아니므로 Closed 로 마킹)
		State = ELiftgateState::Closed;
	}
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
		SetAngle(MaxAngle_deg);
		State = ELiftgateState::Open;
		return;
	}

	AutoOpenElapsed_sec += DeltaTime;
	const float Alpha = FMath::Clamp(
		AutoOpenElapsed_sec / AutoOpenDuration_sec, 0.f, 1.f);
	const float NewAngle = FMath::Lerp(
		AutoOpenStartAngle_deg, MaxAngle_deg, Alpha);
	SetAngle(NewAngle);

	if (Alpha >= 1.f)
	{
		State = ELiftgateState::Open;
	}
}

void ALiftgate::SetAngle(float NewAngle_deg)
{
	CurrentAngle_deg = NewAngle_deg;

	// HingePivot 의 local Pitch 회전 (음수 = 위로 열림, UE 기본 컨벤션).
	// 차량 hinge 축이 좌우 (Y) 일 때 작동. 다른 축이 필요하면 BP child 에서
	// HingePivot 의 RelativeRotation 초기값을 회전시켜 보정.
	HingePivot->SetRelativeRotation(FRotator(-CurrentAngle_deg, 0.f, 0.f));
}
