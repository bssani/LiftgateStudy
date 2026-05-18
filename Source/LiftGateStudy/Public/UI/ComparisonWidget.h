// Copyright Notice: Internal evaluation tool. GMTCK PQDQ.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ComparisonWidget.generated.h"

/**
 * Compare1v2 phase 의 widget (ADR-011).
 * - "둘 중 어느 것이 더 좋습니까?" 텍스트
 * - "1", "2" 버튼 — 클릭 = 즉시 winner 결정 + Test 3 진입
 *
 * BP child (WBP_Comparison) 의 button OnClicked 를 본 클래스의
 * OnChoice1Clicked / OnChoice2Clicked 로 wiring.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class LIFTGATESTUDY_API UComparisonWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Test 1 의 vehicle 이 더 좋음
	UFUNCTION(BlueprintCallable, Category = "UI")
	void OnChoice1Clicked();

	// Test 2 의 vehicle 이 더 좋음
	UFUNCTION(BlueprintCallable, Category = "UI")
	void OnChoice2Clicked();
};
