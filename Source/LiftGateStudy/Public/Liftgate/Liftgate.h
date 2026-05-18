// Copyright Notice: Internal evaluation tool. GMTCK PQDQ.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Liftgate/LiftgateTypes.h"
#include "Liftgate.generated.h"

class UStaticMeshComponent;

/**
 * Liftgate base actor — Phase 3 의 4 mode 지원.
 *
 * 구조 (ADR-009):
 * - DefaultSceneRoot (root SceneComponent) — anchor, 회전 안 함
 * - HingePivot (child) — 회전 컴포넌트. local Pitch 가 liftgate 각도
 * - LiftgateMesh — HingePivot 자식. 실제 mesh 본체
 *
 * BP child (BP_Liftgate_*) 가 wiring:
 * - LiftgateMesh 의 Static Mesh / Material
 * - IsdkGrabbableComponent (HingePivot 의 child) + IsdkGrabTransformer (R5, ADR-001)
 *   → ISDK GrabTransformer 가 HingePivot 의 RelativeRotation 을 직접 회전 (Manual* mode)
 * - InteractionPointerEvent 의 Unselect → EndGrab() 라우팅
 *
 * Mode 별 동작 (CLAUDE.md §9, ADR-011):
 * - ManualFull   : ISDK 회전. EndGrab 시 그 자리 멈춤
 * - ManualAssist : ISDK 회전. EndGrab 시 ≥ ThresholdAngle 이면 AutoOpening
 * - PowerAuto    : Grab 무효. RequestPowerOpen() 호출 시 AutoOpening (target=MaxAngle)
 * - PowerHybrid  : RequestPowerOpen() 시 AutoOpening (target=PowerHybridHandoffAngle).
 *                  완료 후 평가자가 grab 으로 계속 열기. ManualAssist 처럼 EndGrab 분기.
 */
UCLASS(Blueprintable)
class LIFTGATESTUDY_API ALiftgate : public AActor
{
	GENERATED_BODY()

public:
	ALiftgate();

	// ─────────────────────────────────────────────────────────────────────────
	// Components (Constructor 에서 생성. BP child 가 mesh / material 지정)
	// ─────────────────────────────────────────────────────────────────────────

	// 회전 중심. local Pitch 가 liftgate 각도 (위로 열릴 때 음의 Pitch)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Liftgate")
	TObjectPtr<USceneComponent> HingePivot;

	// Mesh 본체. HingePivot 자식이므로 함께 회전
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Liftgate")
	TObjectPtr<UStaticMeshComponent> LiftgateMesh;

	// ─────────────────────────────────────────────────────────────────────────
	// Mode (Phase 3 부터 자유 토글; Phase 2 는 ManualAssist 만)
	// ─────────────────────────────────────────────────────────────────────────

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mode")
	ELiftgateMode CurrentMode = ELiftgateMode::ManualAssist;

	// ─────────────────────────────────────────────────────────────────────────
	// Tunable parameters (BP child 의 Class Defaults 에서 조정)
	// CLAUDE.md §6 Magic Number 기본값
	// ─────────────────────────────────────────────────────────────────────────

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Liftgate")
	float MaxAngle_deg = 85.f;

	// Manual-Assist 의 auto-complete 분기 각도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Liftgate")
	float ThresholdAngle_deg = 40.f;

	// AutoOpening lerp 길이
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Liftgate")
	float AutoOpenDuration_sec = 1.2f;

	// 손 1 cm 이동 → 회전 각도 (deg). 둔감 / 예민 트레이드오프
	// (참고: ISDK GrabTransformer 사용 시 이 값 사용 안 함. BeginGrab/UpdateGrab 의 legacy API 용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Liftgate")
	float GrabSensitivity_deg_per_cm = 1.5f;

	// Power-Hybrid: 자동 개방이 멈추는 각도. 평가자가 여기부터 grab 으로 계속
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Liftgate",
		meta = (ClampMin = "0.0"))
	float PowerHybridHandoffAngle_deg = 30.f;

	// ─────────────────────────────────────────────────────────────────────────
	// Runtime 상태 (UMG bind / debug 용)
	// ─────────────────────────────────────────────────────────────────────────

	UPROPERTY(BlueprintReadOnly, Category = "Liftgate")
	float CurrentAngle_deg = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Liftgate")
	ELiftgateState State = ELiftgateState::Closed;

	// ─────────────────────────────────────────────────────────────────────────
	// API for BP grab wiring (BP_Liftgate 의 ISDK event 가 호출)
	// ─────────────────────────────────────────────────────────────────────────

	UFUNCTION(BlueprintCallable, Category = "Liftgate")
	void BeginGrab(const FVector& HandWorldLocation);

	UFUNCTION(BlueprintCallable, Category = "Liftgate")
	void UpdateGrab(const FVector& HandWorldLocation);

	UFUNCTION(BlueprintCallable, Category = "Liftgate")
	void EndGrab();

	// Power-Auto / Power-Hybrid 의 Power 버튼이 호출.
	// PowerAuto: AutoOpening to MaxAngle
	// PowerHybrid: AutoOpening to PowerHybridHandoffAngle, 도달 후 grab idle
	UFUNCTION(BlueprintCallable, Category = "Liftgate")
	void RequestPowerOpen();

	// 세션 재시작 시 호출 — 각도 0, state Closed 로 복원
	UFUNCTION(BlueprintCallable, Category = "Liftgate")
	void ResetLiftgate();

protected:
	virtual void Tick(float DeltaTime) override;

	// Grab 진입 시 캐싱 (UpdateGrab 의 delta 기준)
	UPROPERTY(Transient)
	float GrabStartAngle_deg = 0.f;

	UPROPERTY(Transient)
	FVector GrabStartHandLocation = FVector::ZeroVector;

	// AutoOpening 중 lerp 진행도
	UPROPERTY(Transient)
	float AutoOpenElapsed_sec = 0.f;

	UPROPERTY(Transient)
	float AutoOpenStartAngle_deg = 0.f;

	// AutoOpening 의 목표 각도. Power-Hybrid 는 handoff 각도, 그 외는 MaxAngle
	UPROPERTY(Transient)
	float AutoOpenTargetAngle_deg = 0.f;

	// HingePivot 의 RelativeRotation 갱신 + CurrentAngle_deg 동기화
	void SetAngle(float NewAngle_deg);

	// HingePivot 의 실제 회전값에서 CurrentAngle_deg 동기화
	// (ISDK GrabTransformer 가 회전시킨 후 EndGrab 에서 호출)
	void SyncAngleFromHinge();
};
