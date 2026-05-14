// Copyright Notice: Internal evaluation tool. GMTCK PQDQ.

#pragma once

#include "CoreMinimal.h"
#include "CalibrationTypes.generated.h"

/**
 * Calibration 진행 상태. CLAUDE.md §7 의 강제 검증 단계와 1:1 대응.
 */
UENUM(BlueprintType)
enum class ECalibrationStatus : uint8
{
	// 측정 / 평가 진행 중 (초기 상태)
	Checking	UMETA(DisplayName = "Checking"),
	// 모든 조건 통과 — Pinch 확인 대기
	Pass		UMETA(DisplayName = "Pass"),
	// 검증 실패 — FailReason 메시지 표시
	Fail		UMETA(DisplayName = "Fail")
};
