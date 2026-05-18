// Copyright Notice: Internal evaluation tool. GMTCK PQDQ.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "VehicleTestSet.generated.h"

class ALiftgate;

/**
 * 평가 세션 한 슬롯의 차량 entry (ADR-010).
 *
 * 평가자에게는 1/2/3/4 슬롯 번호만 노출 (anonymous, ADR-006).
 * Id / ModelCode 는 logging / Editor 디버그 한정.
 */
USTRUCT(BlueprintType)
struct LIFTGATESTUDY_API FVehicleTestEntry
{
	GENERATED_BODY()

	// 익명 식별자. JSON log 에 기록. 평가자 화면에는 표시 안 함.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TestSet")
	FName Id;

	// (선택) Editor 디버그 용. 평가자에게는 표시 안 함.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TestSet")
	FText DebugName;

	// BP_Liftgate variant
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TestSet")
	TSubclassOf<ALiftgate> LiftgateClass;

	// L_Main 의 spawn anchor 기준 relative transform
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TestSet")
	FTransform SpawnRelativeTransform;

	// 차량 model code (JSON log 에 기록).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TestSet")
	FName ModelCode;
};

/**
 * 평가 세션 1 회분의 4 차량 set (ADR-010).
 * Editor 에서 DA_TestSet_Default 자산 1 개 생성하여 4 entry 등록.
 * UEvaluationSessionSubsystem 이 본 asset 을 reference 하여 vehicle swap 수행.
 *
 * Phase 3 는 hardcoded 4 entry. 더 / 덜 한 흐름은 향후 ADR 후 변경.
 */
UCLASS(BlueprintType)
class LIFTGATESTUDY_API UVehicleTestSet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// 정확히 4 entry (Phase 3)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TestSet")
	TArray<FVehicleTestEntry> Tests;
};
