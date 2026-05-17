// Copyright Notice: Internal evaluation tool. GMTCK PQDQ.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Liftgate/LiftgateTypes.h"
#include "Liftgate.generated.h"

class UStaticMeshComponent;

/**
 * Liftgate base actor (Phase 2 — Manual-Assist).
 *
 * 구조:
 * - HingePivot (root SceneComponent) — 회전 중심. local Pitch 가 liftgate 각도
 * - LiftgateMesh — HingePivot 자식. 실제 mesh 본체
 *
 * BP child (BP_Liftgate) 가 다음을 wiring:
 * - LiftgateMesh 의 Static Mesh / Material
 * - IsdkGrabbableComponent (R5, ADR-001) 추가 + grab event 를 BeginGrab/UpdateGrab/EndGrab 으로 라우팅
 *
 * 로직 (CLAUDE.md §6 / ADR-005, 모두 C++):
 * - 손 World Z delta × GrabSensitivity → 각도 변화 (선형, Phase 2 MVP)
 * - Manual-Assist: EndGrab 시 CurrentAngle ≥ ThresholdAngle 이면 AutoOpening 진입,
 *   AutoOpenDuration 동안 MaxAngle 까지 lerp
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Liftgate")
	float GrabSensitivity_deg_per_cm = 1.5f;

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

	// HingePivot 의 RelativeRotation 갱신 + CurrentAngle_deg 동기화
	void SetAngle(float NewAngle_deg);
};
