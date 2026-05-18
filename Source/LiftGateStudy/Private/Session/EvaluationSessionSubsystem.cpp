// Copyright Notice: Internal evaluation tool. GMTCK PQDQ.

#include "Session/EvaluationSessionSubsystem.h"

#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "HAL/FileManager.h"
#include "Liftgate/Liftgate.h"
#include "Misc/FileHelper.h"
#include "Misc/Guid.h"
#include "Misc/Paths.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Session/VehicleTestSet.h"

void UEvaluationSessionSubsystem::StartSession(UVehicleTestSet* InTestSet)
{
	// 새 세션 식별자 (ADR-006: UUID v4, anonymous)
	SessionId = FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphens).ToLower();
	SessionStartTimeUtc = FDateTime::UtcNow();

	ActiveTestSet = InTestSet;
	Comparison1v2WinnerSlot = -1;
	FinalRankingSlots.Reset();

	// Test 1 진입
	CurrentSlotIndex = 0;
	SetPhase(EEvaluationPhase::Test1);
	SpawnVehicleForSlot(0);
}

void UEvaluationSessionSubsystem::RequestNext()
{
	switch (CurrentPhase)
	{
	case EEvaluationPhase::Test1:
		CurrentSlotIndex = 1;
		SetPhase(EEvaluationPhase::Test2);
		SpawnVehicleForSlot(1);
		break;

	case EEvaluationPhase::Test2:
		// Comparison 진입 — vehicle destroy (Comparison phase 에서는 차량 안 보임)
		DestroyActiveLiftgate();
		CurrentSlotIndex = -1;
		SetPhase(EEvaluationPhase::Compare1v2);
		break;

	case EEvaluationPhase::Test3:
		CurrentSlotIndex = 3;
		SetPhase(EEvaluationPhase::Test4);
		SpawnVehicleForSlot(3);
		break;

	case EEvaluationPhase::Test4:
		// Final Ranking 진입
		DestroyActiveLiftgate();
		CurrentSlotIndex = -1;
		SetPhase(EEvaluationPhase::FinalRanking);
		break;

	default:
		// 다른 phase 에서는 Next 무의미
		break;
	}
}

void UEvaluationSessionSubsystem::RequestPrevious()
{
	switch (CurrentPhase)
	{
	case EEvaluationPhase::Test2:
		CurrentSlotIndex = 0;
		SetPhase(EEvaluationPhase::Test1);
		SpawnVehicleForSlot(0);
		break;

	case EEvaluationPhase::Test4:
		CurrentSlotIndex = 2;
		SetPhase(EEvaluationPhase::Test3);
		SpawnVehicleForSlot(2);
		break;

	default:
		// Test1 / Test3 / Compare1v2 / FinalRanking 은 Previous 없음
		break;
	}
}

void UEvaluationSessionSubsystem::RequestComparisonChoice(int32 LocalWinnerIndex)
{
	if (CurrentPhase != EEvaluationPhase::Compare1v2)
	{
		return;
	}

	// LocalWinnerIndex: 0 = Test1 의 slot (slot 0), 1 = Test2 의 slot (slot 1)
	Comparison1v2WinnerSlot = FMath::Clamp(LocalWinnerIndex, 0, 1);

	// Test 3 진입
	CurrentSlotIndex = 2;
	SetPhase(EEvaluationPhase::Test3);
	SpawnVehicleForSlot(2);
}

void UEvaluationSessionSubsystem::AddRanking(int32 SlotIndex)
{
	if (CurrentPhase != EEvaluationPhase::FinalRanking)
	{
		return;
	}

	// 이미 ranked 됐으면 무시 (widget 측 button disable 보강)
	if (FinalRankingSlots.Contains(SlotIndex))
	{
		return;
	}

	if (SlotIndex < 0 || SlotIndex > 3)
	{
		return;
	}

	FinalRankingSlots.Add(SlotIndex);

	// 4 개 다 ranked → 자동 confirm (JSON write + reset)
	if (FinalRankingSlots.Num() >= 4)
	{
		SetPhase(EEvaluationPhase::WritingLog);
		WriteSessionLogToDisk();
		FinalizeAndAutoReset();
	}
}

void UEvaluationSessionSubsystem::ResetRanking()
{
	if (CurrentPhase != EEvaluationPhase::FinalRanking)
	{
		return;
	}
	FinalRankingSlots.Reset();
}

bool UEvaluationSessionSubsystem::IsRanked(int32 SlotIndex) const
{
	return FinalRankingSlots.Contains(SlotIndex);
}

int32 UEvaluationSessionSubsystem::GetRankOrder(int32 SlotIndex) const
{
	const int32 Found = FinalRankingSlots.IndexOfByKey(SlotIndex);
	return (Found == INDEX_NONE) ? 0 : (Found + 1);
}

void UEvaluationSessionSubsystem::SetPhase(EEvaluationPhase NewPhase)
{
	CurrentPhase = NewPhase;
	OnPhaseChanged.Broadcast(NewPhase);
}

void UEvaluationSessionSubsystem::SpawnVehicleForSlot(int32 SlotIndex)
{
	DestroyActiveLiftgate();

	if (!ActiveTestSet || !ActiveTestSet->Tests.IsValidIndex(SlotIndex))
	{
		return;
	}

	const FVehicleTestEntry& Entry = ActiveTestSet->Tests[SlotIndex];
	if (!Entry.LiftgateClass)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Spawn transform = anchor world transform * entry relative transform.
	// anchor 미설정 시 world origin 기준.
	FTransform AnchorWorldTransform = FTransform::Identity;
	if (SpawnAnchor)
	{
		AnchorWorldTransform = SpawnAnchor->GetActorTransform();
	}
	const FTransform SpawnWorldTransform =
		Entry.SpawnRelativeTransform * AnchorWorldTransform;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ActiveLiftgate = World->SpawnActor<ALiftgate>(
		Entry.LiftgateClass,
		SpawnWorldTransform,
		Params);

	if (ActiveLiftgate)
	{
		ActiveLiftgate->ResetLiftgate();
	}
}

void UEvaluationSessionSubsystem::DestroyActiveLiftgate()
{
	if (ActiveLiftgate)
	{
		ActiveLiftgate->Destroy();
		ActiveLiftgate = nullptr;
	}
}

void UEvaluationSessionSubsystem::WriteSessionLogToDisk()
{
	// ADR-006: JSON, anonymous, schema_version=1
	if (!ActiveTestSet || ActiveTestSet->Tests.Num() < 4)
	{
		return;
	}

	TSharedPtr<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetNumberField(TEXT("schema_version"), 1);
	Root->SetStringField(TEXT("session_id"), SessionId);
	Root->SetStringField(TEXT("timestamp_utc"), SessionStartTimeUtc.ToIso8601());

	TArray<TSharedPtr<FJsonValue>> TestsArray;
	for (int32 i = 0; i < 4; ++i)
	{
		const FVehicleTestEntry& Entry = ActiveTestSet->Tests[i];
		TSharedPtr<FJsonObject> TestEntry = MakeShared<FJsonObject>();
		TestEntry->SetNumberField(TEXT("slot"), i + 1);
		TestEntry->SetStringField(TEXT("model_code"), Entry.ModelCode.ToString());
		TestsArray.Add(MakeShared<FJsonValueObject>(TestEntry));
	}
	Root->SetArrayField(TEXT("tests"), TestsArray);

	// Comparison winner slot: LocalWinnerIndex (0/1) → slot (1/2)
	Root->SetNumberField(TEXT("comparison_1v2_winner_slot"),
		Comparison1v2WinnerSlot + 1);

	// final_ranking_by_slot: array of slot indices (1-based) in order of preference
	TArray<TSharedPtr<FJsonValue>> RankingArray;
	for (int32 Slot : FinalRankingSlots)
	{
		RankingArray.Add(MakeShared<FJsonValueNumber>(Slot + 1));
	}
	Root->SetArrayField(TEXT("final_ranking_by_slot"), RankingArray);

	// Serialize → string
	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(Root.ToSharedRef(), Writer);

	// 파일 경로: Saved/EvaluationLogs/session_<UTCstamp>_<shortUUID>.json
	const FString DirPath = FPaths::ProjectSavedDir() / TEXT("EvaluationLogs");
	IFileManager& FileMgr = IFileManager::Get();
	FileMgr.MakeDirectory(*DirPath, /*Tree=*/true);

	const FString Timestamp = SessionStartTimeUtc.ToString(TEXT("%Y%m%d_%H%M%S"));
	const FString ShortUuid = SessionId.Left(8);
	const FString FileName = FString::Printf(TEXT("session_%s_%s.json"), *Timestamp, *ShortUuid);
	const FString FullPath = DirPath / FileName;

	const bool bSaved = FFileHelper::SaveStringToFile(OutputString, *FullPath);
	if (!bSaved)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("EvaluationSession: failed to write log to %s"),
			*FullPath);
	}
}

void UEvaluationSessionSubsystem::FinalizeAndAutoReset()
{
	// 차량 / 위젯 상태 reset 후 새 세션 자동 시작
	DestroyActiveLiftgate();

	UVehicleTestSet* TestSetToRestart = ActiveTestSet;
	Comparison1v2WinnerSlot = -1;
	FinalRankingSlots.Reset();
	CurrentSlotIndex = -1;

	SetPhase(EEvaluationPhase::Idle);

	// 다음 평가자를 위해 즉시 새 세션 (사용자 spec: 자동 리셋)
	if (TestSetToRestart)
	{
		StartSession(TestSetToRestart);
	}
}
