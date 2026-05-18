# ADR-012 — BP_PowerButton Extends BigRedButton (ISDK Sample Prop)

**Date**: 2026-05-18
**Status**: Accepted
**Phase**: 3

## Context

Phase 3 (`Phase3_Kickoff.md` W3.2) 의 Power-Auto / Power-Hybrid mode 는 평가자가 차량 옆 World-space 버튼을 손가락으로 poke 해서 `ALiftgate::RequestPowerOpen()` 을 호출하도록 설계됨 (R8 / ADR-007).

기존 spec 의 `APowerButtonActor` (C++) 는 root 가 `UWidgetComponent` (World-space UMG) 이고, BP child `BP_PowerButton` 의 widget OnClicked → `OnButtonClicked` UFUNCTION → `TargetLiftgate->RequestPowerOpen()` 흐름 (`Source/LiftGateStudy/Public/Liftgate/PowerButtonActor.h:31`, `Source/LiftGateStudy/Private/Liftgate/PowerButtonActor.cpp:12`).

조사 결과 `OculusInteractionSamples` plugin 이 `BigRedButton` (`Plugins/OculusInteractionSamples/Content/Objects/Props/BigRedButton/BigRedButton.uasset`) 을 이미 제공한다:

- ISDK native `PokeInteractable` + `PointablePlane` + `PokeButtonVisual` + `ClippedPlaneSurface` 셋업 완료
- 시각 mesh 3 종 (`BigRedButton_Button`, `BigRedButton_ButtonRing`, `BigRedButton_ButtonRingShadow`) 포함
- Poke press / release audio cue + `IsdkPointerEventAudioPlayer` 내장
- BP graph: Pointable plane size / bounds clippers, Poke interactable surface patch 셋업, `InteractableStateChanged` → `Normal` / `Hover` / `Select` / `Disabled` 색상 전이, `InteractorPointerEvent` 디버그 출력
- `Select` type 이 "버튼 눌림 확정" 시그널

평가:

- ADR-007 (button poke, no gesture) 와 정확히 부합. UMG WidgetComponent 의 OnClicked 는 ISDK poke 와 우회 연동되는 반면, `BigRedButton` 은 ISDK native pipeline
- ADR-004 (BP_VRPawn ⊂ IsdkSamplePawn) 와 동일 패턴 — plugin BP 를 base 로 child BP 작성
- ISDK 가 검증한 visual / audio / state 전이 그대로 활용 — UMG 버튼보다 affordance ↑

선택지:

| Option | 방식 | 평가 |
|---|---|---|
| **A** | **`BP_PowerButton` 을 `BigRedButton` (plugin BP) 의 child** | **ADR-004 와 동일 패턴. ISDK native. 가장 간결. `APowerButtonActor` C++ 불필요** |
| B | `APowerButtonActor` 를 ISDK C++ API (PokeInteractable, PointablePlane …) 로 재구성 | ADR-005 C++ 로직 우선 원칙 충실. 단 ISDK C++ API 학습 필요, BigRedButton 의 BP-only 셋업 (Make Bounds Clippers, audio player) 을 C++ 로 재작성 부담 |
| C | `APowerButtonActor` 유지 + ChildActorComponent 로 `BigRedButton` spawn, InteractorPointerEvent 를 BP 에서 forwarding | 두 layer 공존, nested actor 구조로 spawn / cleanup 복잡 |

## Decision

**Option A 채택.** `BP_PowerButton` 은 `BigRedButton` (plugin BP) 을 parent class 로 가지는 child Blueprint.

### 변경 사항

1. **`APowerButtonActor` (C++) 제거** — Phase 3 spec 의 C++ deliverable 에서 빠짐. `Source/LiftGateStudy/Public/Liftgate/PowerButtonActor.h` / `Source/LiftGateStudy/Private/Liftgate/PowerButtonActor.cpp` 는 별도 정리 PR 에서 삭제 (본 ADR 은 planning, 코드 변경 없음).
2. **`BP_PowerButton` 신규 — parent = `BigRedButton`**:
   - **BP variable 추가**: `TargetLiftgate` (`ALiftgate*`, `Category="PowerButton"`, `Instance Editable`, `Expose On Spawn`)
   - **Event Graph 추가** (parent 의 Construction / IsdkPointerEvent 셋업 위에 덧붙임):
     - Parent 의 `InteractorPointerEvent` (또는 `Bind Event to Interactor Pointer Event`) 를 그대로 활용
     - `Break Isdk Interaction Pointer Event` → `Type` 이 `Select` 인 경우에 한해 `TargetLiftgate.RequestPowerOpen()` 호출
     - (`Hover`, `Move`, `Unselect`, `Unhover`, `Cancel` 은 무시)
3. **Spawn 측** (`UEvaluationSessionSubsystem` 또는 `BP_Liftgate_PowerAuto` / `BP_Liftgate_PowerHybrid`):
   - `BP_PowerButton` spawn 시 `TargetLiftgate` 를 자기 자신 (해당 ALiftgate instance) 으로 set
   - Expose On Spawn 이므로 SpawnActor 노드에서 직접 wiring 가능

### `RequestPowerOpen()` 호출 책임의 위치

| 책임 | 위치 | 비고 |
|---|---|---|
| Poke detection | ISDK (`PokeInteractable`) | BigRedButton 내장 |
| 버튼 press 시각 / 사운드 | `BigRedButton` parent BP | StateChanged → color, Cue 자동 |
| Select event 라우팅 | `BP_PowerButton` Event Graph | parent event 구독, Select type 분기 |
| Liftgate 상태 머신 | `ALiftgate::RequestPowerOpen()` (C++) | 변경 없음 |

ADR-005 (C++ 로직 + BP 레이아웃) 정신은 유지 — 로직 (mode 분기, AutoOpening lerp, target angle 계산) 은 `ALiftgate` C++ 에 있고, BP_PowerButton 은 **이벤트 forwarding 1 줄** 만 BP 에 둠. 이 정도의 BP 노드 (Bind + Branch + Function Call) 는 ADR-005 §6 의 "단순 connection" 범위.

## Consequences

**Positive**:
- ISDK native poke pipeline → R8 / ADR-007 정신에 정확히 부합 (UMG OnClicked 우회 없음)
- BigRedButton 의 visual / audio / state 전이 (Hover / Select 색상, press / release cue) 무료 획득
- `APowerButtonActor` C++ 삭제로 Phase 3 deliverable 1 종 감소
- ADR-004 와 동일 패턴 — plugin BP 활용으로 plugin 업데이트 benefit 자동 반영
- 평가자 affordance ↑ (실제 물리 버튼처럼 보이는 빨간 버튼)

**Negative**:
- `OculusInteractionSamples` plugin 의존 — Phase 4+ 에서 plugin 제거 / 교체 시 fallback 경로 필요 (ADR-004 와 동일 risk)
- `BigRedButton` 내부 구조 변경 시 child BP 영향 — plugin upgrade 시 release note 확인 필요
- BigRedButton 의 visual (빨간 버튼 + ring) 이 차량 옆 인터랙션에 시각적으로 과하다고 판단되면 mesh / material override 필요 (parent BP override 가능)
- `Saved/` 의 binary BP 자산 추가 — git diff 추적 어려움 (`.uasset` 한계, ADR-004 와 동일)

**Neutral**:
- `OculusInteractionSamples/Content/Objects/Props/BigRedButton/` 는 그대로 plugin 안에 있음 (커밋 대상 아님)
- ISDK Sample 의 button widget (UMG) 채택안은 폐기 — BigRedButton 이 더 ISDK 정신에 가까움
- BigRedButton 의 BP graph 안에 있는 `Enable Debug Output` flag 는 child 에서 false 로 (성능 / 로그 노이즈 회피)

## Verification

본 ADR 채택 후 Phase 3 W3.2 / W3.3 / W3.7 검증 시 다음 추가 확인:

1. `BP_PowerButton` 의 parent class 가 `BigRedButton` 으로 표시됨 (Class Settings)
2. `TargetLiftgate` 가 Instance Editable + Expose On Spawn
3. VR Preview 에서 평가자 손가락이 BigRedButton 표면에 닿으면 색상이 Hover → Select 로 전이 (시각 피드백)
4. Select 트리거 시 `ALiftgate::RequestPowerOpen()` 이 호출됨 (BP_Liftgate_PowerAuto 가 AutoOpening 시작)
5. Power-Hybrid: 동일 트리거가 AutoOpening to `PowerHybridHandoffAngle_deg` (Liftgate.h:84) 까지 실행 후 grab idle
6. 한 세션 안에서 Power-Auto vehicle 과 Power-Hybrid vehicle 의 버튼이 각자의 ALiftgate 에 정확히 라우팅 (TargetLiftgate ref 가 올바르게 set)

검증 실패 시 즉시 `Docs/lessons_learned/<topic>.md` 박제.

## Notes

- `BigRedButton` 의 `Pointable Plane Size`, `Poke Button Visual`, `Poke Interactable` 등 expose 된 변수는 BP_PowerButton 의 Class Defaults 에서 차량별로 조정 가능 (예: 버튼 plane size 를 차량 디자인에 맞게)
- 차량 1, 2 (Manual) 는 Power button 없음 — `BP_Liftgate_Manual` / `BP_Liftgate_ManualFull` 의 Construction 에서 BP_PowerButton spawn 하지 않음
- 차량 3 (Power-Auto) / 차량 4 (Power-Hybrid) 만 BP_PowerButton 동반 spawn (`Phase3_Kickoff.md` §W3.2.C / §W3.2.D)
- Power button 의 World-space placement 는 평가자가 닿을 수 있는 위치 — CLAUDE.md §8 (정면 30~60cm, 어깨~허리 높이). `BP_Liftgate_PowerAuto` / `_PowerHybrid` 의 Construction Script 또는 BP 직접 배치
- 본 ADR 은 ADR-007 (button poke modality) 의 구체화. ADR-007 supersede 가 아니라 specialization
- Plugin 의 `BigRedButton_Reset_Transforms` 자산도 동일 폴더에 있음 — 향후 "버튼 누른 후 spring back" 애니메이션 검토 시 활용 가능
- 향후 차량별 버튼 visual 차별화 필요 시: `BigRedButton` mesh / material 을 BP_PowerButton 의 child variant 에서 override (예: `BP_PowerButton_Sleek` 등). 단 Phase 3 범위 외
