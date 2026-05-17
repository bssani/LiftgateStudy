// Copyright Notice: Internal evaluation tool. GMTCK PQDQ.

#pragma once

#include "CoreMinimal.h"
#include "LiftgateTypes.generated.h"

/**
 * Liftgate 의 조작 mode (CLAUDE.md §9).
 * Phase 2 는 ManualAssist 만 동작; 나머지 3 종은 Phase 3 진입 시 활성화.
 */
UENUM(BlueprintType)
enum class ELiftgateMode : uint8
{
	// 0°~MaxAngle 전 구간 hand grab 필요. 손 놓으면 멈춤
	ManualFull   UMETA(DisplayName = "Manual-Full"),
	// Hand grab 으로 올림. ThresholdAngle 이상에서 손 놓으면 auto-complete
	ManualAssist UMETA(DisplayName = "Manual-Assist"),
	// Power button 1 회 → 끝까지 자동 개방
	PowerAuto    UMETA(DisplayName = "Power-Auto"),
	// Power button → 일정 각도까지 자동 → 이후 hand grab
	PowerHybrid  UMETA(DisplayName = "Power-Hybrid")
};

/**
 * Liftgate 의 runtime 상태 머신.
 * Closed → Grabbed → (release) → Closed (멈춤) 또는 AutoOpening → Open.
 * Phase 2 는 close 동작 없음 (Open 도달 후 reset 은 PIE 재시작).
 */
UENUM(BlueprintType)
enum class ELiftgateState : uint8
{
	// 평가자가 잡지 않은 상태 (각도 무관). 초기값
	Closed       UMETA(DisplayName = "Closed"),
	// 평가자가 grab 중. UpdateGrab 호출이 회전 갱신
	Grabbed      UMETA(DisplayName = "Grabbed"),
	// Manual-Assist threshold 초과 후 release → MaxAngle 까지 lerp
	AutoOpening  UMETA(DisplayName = "AutoOpening"),
	// MaxAngle 도달 (완전 개방). Phase 2 는 여기서 종료
	Open         UMETA(DisplayName = "Open")
};
