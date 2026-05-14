# ADR-005 — Full C++ Adoption (C++ Logic + BP Layout Hybrid)

**Date**: 2026-05-14
**Status**: Accepted
**Phase**: 1
**Supersedes (partial)**: CLAUDE.md §2 "Blueprint 우선. C++는 Phase 2 이후 필요 시 도입"

## Context

CLAUDE.md v0.2 §2 는 "Blueprint 우선, C++는 Phase 2 이후" 정책이었다. 그러나 Phase 1 W1.4 (calibration widget) 작업을 시작하면서 다음 한계가 드러남:

- `.uasset` 은 binary 라 **git diff / code review 불가** — Phase 1 산출물 대부분이 BP 라 협업 시 변경 추적이 어렵다
- Calibration / Wrist UI 의 검증 로직은 ray cast, math, state machine 등 **텍스트 코드가 더 읽기 쉬운** 영역
- Phase 2 의 ISDK grab event callback, liftgate 회전 / damping 모델은 어차피 C++ 가 자연스럽다 — 그때 가서 도입하면 Phase 1 산출물을 재작성해야 함
- 개발자가 이미 **VS 2022 + C++ workload 설치 + UE C++ 경험** 보유 → 셋업 비용 회수 가능

선택지:

| Option | 내용 | 평가 |
|---|---|---|
| A | Phase 1 BP 유지. Phase 2 진입 시 C++ 도입 | 단기 일정 ↑. 중기 재작업 비용 |
| B | W1.4 만 hybrid (C++ base + BP layout) | 일관성 결여. Phase 진행 중 패턴 혼재 |
| **C** | **CLAUDE.md 수정. Phase 1 부터 C++ 전면 도입 (hybrid 패턴)** | 셋업 비용 ↑, 그러나 governance 일관성 + 장기 비용 ↓ |

## Decision

**Option C 채택.** 이제부터 모든 새로운 Actor / Widget / GameMode / Component 는 **C++ base class** 로 작성하고, **BP 는 layout / 디자이너 wiring / asset 참조 / Designer-tunable defaults** 만 담당한다.

이 결정으로 CLAUDE.md §2 Tech Stack 의 "Blueprint 우선" 정책을 **수정**한다. v0.3 으로 bump.

### Hybrid Pattern (Standard)

```
C++ base class                   BP child (.uasset)
─────────────────                ──────────────────
ULiftgateStudyGameMode           BP_LiftgateStudyGameMode
AVehicleInfo                     BP_VehicleInfo
UCalibrationCheckWidget          WBP_CalibrationCheck
UWristPanelWidget                WBP_WristPanel
ACalibrationGateActor            BP_CalibrationGate
ALiftgate (Phase 2)              BP_Liftgate
```

규칙:
- **로직 (Tick, math, state machine, ISDK callback, ray cast, 시간 계산 등)** → C++
- **컴포넌트 트리, mesh / material 참조, 위치 / 회전 default, UI 텍스트 / 색상** → BP
- **Magic number 는 C++ 의 `UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=...)`** 로 노출. BP child 에서 default 조정.

### Exception (BP-only 유지 대상)

- `BP_VRPawn` (ADR-004): parent 가 BP `IsdkSamplePawn` 이므로 BP 로 그대로 유지. 필요 시 child component 에서 C++ 활용
- Level (`L_Main`, `L_Vehicle_Dummy`): `.umap` 은 항상 binary
- Material / Mesh / Texture: asset 본연의 영역

## Consequences

**Positive**:
- 로직이 git diff / code review 가능
- Unit testable code (예: pure functions in `CalibrationMath.h`)
- Phase 2 의 ISDK grab callback / liftgate physics 가 자연스럽게 이어짐
- IDE (VS / Rider) 의 navigation / refactoring 활용
- 평가자 간 일관된 동작 (compile-time 검증)
- UPROPERTY 노출로 Designer-tunable + runtime-safe 동시 만족

**Negative**:
- VS 2022 + C++ workload **하드 의존** (README 갱신 필요)
- Iteration 속도 ↓ (Live Coding / Hot Reload 안정성 이슈 대비 필요)
- Phase 1 진입 비용 ↑ (1~2일 셋업 + 학습)
- 모듈 / dependency 관리 부담 (`.Build.cs`)
- Plugin binary 와 C++ module 의 컴파일러 일치 필요 (UE 5.5 + VS 2022)

**Neutral**:
- ADR-001 (ISDK grab) 영향 없음 — IsdkGrabbableComponent 는 C++ 컴포넌트라 이미 호환
- ADR-002, ADR-003 영향 없음
- ADR-004 (IsdkSamplePawn parent) 영향 없음 — BP_VRPawn 만 예외로 둠
- `Plugins/OculusInteractionSamples/Source/` 는 이미 C++ 모듈 → 동일 컴파일러로 일관

## Project Module Structure (target)

```
LiftgateStudy/
├── LiftGateStudy.uproject              ← Modules entry 추가됨 (UE Editor 가 자동 생성)
└── Source/
    └── LiftGateStudy/
        ├── LiftGateStudy.Build.cs
        ├── LiftGateStudy.h
        ├── LiftGateStudy.cpp
        ├── Public/
        │   ├── LiftgateStudyGameMode.h
        │   ├── Calibration/
        │   │   ├── CalibrationTypes.h            (ECalibrationStatus enum)
        │   │   ├── CalibrationCheckWidget.h
        │   │   └── CalibrationGateActor.h
        │   └── UI/
        │       └── WristPanelWidget.h
        └── Private/
            └── (대응 .cpp)
```

`.Build.cs` 의존성:
```csharp
PublicDependencyModuleNames.AddRange(new string[] {
    "Core", "CoreUObject", "Engine", "InputCore",
    "EnhancedInput",
    "UMG", "Slate", "SlateCore",
    "HeadMountedDisplay",
});
```

(ISDK 직접 호출이 필요해지면 그 시점에 ISDK 모듈 추가)

## Verification

- [ ] `LiftGateStudy.uproject` 에 `Modules` 항목 추가됨 (UE Editor → Tools → New C++ Class 첫 실행 시 자동)
- [ ] `Source/LiftGateStudy/` 모듈이 컴파일됨 (VS 2022)
- [ ] 첫 C++ 클래스 (예: `ULiftgateStudyGameMode`) 가 BP child 와 연결 가능
- [ ] Live Coding 동작 확인 또는 Editor restart 컴파일 동작 확인
- [ ] CI / 빌드 환경 (있다면) 에서 C++ 컴파일 통과

위 verification 이 실패하면 Option A (Phase 1 BP, Phase 2 C++) 로 일시 rollback 검토.

## Naming Conventions (C++ 추가)

CLAUDE.md §4 의 BP naming 은 유지. C++ 추가 규칙:

| 항목 | 규칙 | 예시 |
|---|---|---|
| C++ Actor | `A<Name>` | `ACalibrationGateActor` |
| C++ Pawn | `A<Name>` | `AVRPawn` (사용 시) |
| C++ Component | `U<Name>Component` | `UCalibrationComponent` |
| C++ Widget | `U<Name>Widget` | `UCalibrationCheckWidget` |
| C++ Enum (Blueprintable) | `E<Name>` + `UENUM(BlueprintType)` | `ECalibrationStatus` |
| C++ Struct (Blueprintable) | `F<Name>` + `USTRUCT(BlueprintType)` | `FHeightZone` |
| BP child of C++ | `BP_<Name>` 또는 `WBP_<Name>` | `BP_CalibrationGate` from `ACalibrationGateActor` |

UPROPERTY / UFUNCTION 의 `Category` 는 CLAUDE.md §6 의 BP variable Category 와 동일 (`Calibration`, `Mode`, `Zone`, `UI`, `Debug`).

## Notes

- C++ 도입은 governance 변경이라 본 ADR 외 추가 ADR 없이 패턴 변형 금지. 예외 케이스 발생 시 별도 ADR
- Live Coding 이 불안하면 Editor 재시작 컴파일 사용 — 작업 흐름 규칙은 `Docs/lessons_learned/` 에 박제
- Plugin (`MetaXR`, `MetaXRInteraction`, `OculusInteractionSamples`) 은 자체 C++ 모듈이라 우리 module 과 별개로 관리됨. Plugin source 변경 금지.
- 향후 Module 분할 (예: `LiftgateStudyCore`, `LiftgateStudyUI`) 이 필요해지면 별도 ADR
