# ADR-010 — Vehicle Test Set as Data Asset

**Date**: 2026-05-18
**Status**: Accepted
**Phase**: 3

## Context

Phase 3 에서 평가 흐름이 **4 개 차량 (각자 다른 mode) 의 순차 / 비교 평가** 로 확정. CLAUDE.md §5 Phase 4 의 "Multi-vehicle + Polish" 와 §10 Out of Scope 의 "차량 swap UX" 가 Phase 3 로 통합 (CLAUDE.md v0.6).

차량 (= BP_Liftgate variant) 을 평가 세션 중 어떻게 swap 할지 결정 필요:

| Option | 방식 | Pros | Cons |
|---|---|---|---|
| A | Level streaming — 4 sublevel | 차량별 환경 / lighting 다르게 가능 | streaming hitch, sublevel 관리 부담 |
| B | Single level + spawn/destroy | 단순, dynamic | 환경 차이 없음 (Phase 3 범위에서 OK) |
| **C** | **Single level + UDataAsset + spawn/destroy** | B + data-driven, test set 추가 / 순서 변경 쉬움 | 작은 구현 비용 |

Phase 3 의 dummy car 는 동일 (L_Vehicle_Dummy 또는 후속의 환경), liftgate 만 swap. 차량별 환경 변화 (lighting, layout) 는 Phase 4 의 real CAD 단계로 미룸.

## Decision

**Option C** 채택. 차량 test set 을 `UDataAsset` 으로 정의:

```cpp
USTRUCT(BlueprintType)
struct FVehicleTestEntry
{
    GENERATED_BODY()

    // 익명 식별자 (logging 에 사용). 평가자 화면에는 표시 안 함
    UPROPERTY(EditAnywhere, Category="TestSet")
    FName Id;

    // (선택) Editor 디버그 용 display name. 평가자에게는 1/2/3/4 만 노출
    UPROPERTY(EditAnywhere, Category="TestSet")
    FText DebugName;

    // BP_Liftgate variant
    UPROPERTY(EditAnywhere, Category="TestSet")
    TSubclassOf<class ALiftgate> LiftgateClass;

    // L_Main 의 SpawnAnchor 기준 RelativeTransform
    UPROPERTY(EditAnywhere, Category="TestSet")
    FTransform SpawnRelativeTransform;

    // 차량 metadata (logging 에 포함 가능, 평가자에 노출 안 함)
    UPROPERTY(EditAnywhere, Category="TestSet")
    FName ModelCode;   // "C1YC", "Equinox" 등
};

UCLASS(BlueprintType)
class LIFTGATESTUDY_API UVehicleTestSet : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    // 정확히 4 개 entry (Phase 3 hardcoded)
    UPROPERTY(EditAnywhere, Category="TestSet")
    TArray<FVehicleTestEntry> Tests;
};
```

Editor 에서 `DA_TestSet_Default` 자산 1 개 만들어 4 entry 등록. `UEvaluationSessionSubsystem` (ADR-011) 이 본 asset 을 reference 하여 swap 진행.

## Consequences

**Positive**:
- 평가 세션 정의가 데이터 (단순 asset 편집) — 코드 변경 없이 차량 순서 / 종류 교체 가능
- 4 개 BP_Liftgate variant 가 자산 reference 로 명확히 묶임
- Phase 4 의 real CAD 차량 도입 시 entry 만 추가 / 교체

**Negative**:
- 4 개 hardcoded (TArray 의 Phase 3 가정). 더 / 덜 한 평가 흐름은 별도 UDataAsset 또는 코드 변경
- 차량별 환경 (lighting 등) 다르게 못 함 (Phase 4 면 추가)

**Neutral**:
- `Id` 와 `ModelCode` 의 차이: `Id` 는 sessionwide 의 인덱스 (예: "Test1"), `ModelCode` 는 실차 코드 (logging 에 기록)
- Anonymous 원칙 (ADR-006) — 평가자에게는 1/2/3/4 만 노출, 차량명 / `DebugName` 등은 Editor / log 한정

## Notes

- 평가 세션 동안 동일 vehicle 이 반복 spawn 될 수 있음 (Prev/Next 토글). Subsystem 은 idempotent 하게 처리
- `SpawnRelativeTransform` 의 anchor 는 L_Main 의 정해진 위치 (Phase 3 W3.7 에서 결정)
- 향후 Phase 4 에서 차량별 환경 필요 시 entry 에 `TSoftObjectPtr<UWorld>` 추가하여 level streaming 도 지원 가능
