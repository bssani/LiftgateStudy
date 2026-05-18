// Copyright Notice: Internal evaluation tool. GMTCK PQDQ.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TestProgressWidget.generated.h"

/**
 * Test 1~4 phase 의 widget (ADR-011).
 * - "테스트 N" 표시 (N=1~4)
 * - Next 버튼 (모든 phase 에서 visible)
 * - Previous 버튼 (Test 2 / Test 4 에서만 visible)
 *
 * BP child (WBP_TestProgress) 가 button OnClicked 를 본 클래스의
 * OnNextClicked / OnPreviousClicked 로 wiring.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class LIFTGATESTUDY_API UTestProgressWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// UMG bind 용 — "테스트 N" 의 N (1~4)
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UI")
	int32 GetCurrentTestNumber() const;

	// Previous 버튼 visibility binding 용 (Test 2 / Test 4 만 true)
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UI")
	bool ShouldShowPrevious() const;

	// BP child 의 Next button OnClicked → 본 함수 호출
	UFUNCTION(BlueprintCallable, Category = "UI")
	void OnNextClicked();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void OnPreviousClicked();
};
