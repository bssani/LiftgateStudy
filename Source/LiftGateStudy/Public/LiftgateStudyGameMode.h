// Copyright Notice: Internal evaluation tool. GMTCK PQDQ.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LiftgateStudyGameMode.generated.h"

/**
 * Phase 1 GameMode.
 * Default Pawn / HUD / Player Controller 는 BP child (BP_LiftgateStudyGameMode) 에서 지정.
 * Phase 2 진입 시 vehicle / mode 상태 관리 책임을 본 클래스로 이관할 수 있음.
 */
UCLASS()
class LIFTGATESTUDY_API ALiftgateStudyGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ALiftgateStudyGameMode();
};
