# ADR-013 — Single Session UI Umbrella Widget with WidgetSwitcher

**Date**: 2026-05-18
**Status**: Accepted
**Phase**: 3

## Context

Phase3_Kickoff.md §W3.5 spec 은 Test / Compare / FinalRanking 세 phase 마다 별도 top-level widget (`WBP_TestProgress`, `WBP_Comparison`, `WBP_FinalRanking`) 을 두고, `L_Main` 의 "widget host actor (1 개)" 가 phase 따라 swap 하는 흐름이다 (§W3.6.B).

검토 결과 본 분리는 **over-engineered**:

- 세 widget 모두 **동일 World-space anchor** (정면 1.5m, Z=1.5m, §W3.6.B)
- **Mutually exclusive** (phase 정의상 동시 visible 없음)
- 모두 `UEvaluationSessionSubsystem::OnPhaseChanged` 단일 source 로 구동
- 각 widget 이 OnPhaseChanged 를 3중 subscribe → broadcast 순서 의존 race 발생 가능
- ADR-008 의 Note "Phase 3 진입 시 mode toggle UI 위치 결정 ADR 추가 (예: 평가자 좌측 50cm 떠있는 panel, 또는 차량 옆 stand)" — 본 ADR 이 그 자리

또한 기존 spec 의 placement (정면 1.5m, vehicle 은 정면 3m) 는 평가자 시야선상에 widget 이 들어와 **liftgate 조작 시 시야 방해** 가능성이 있다.

## Decision

### D1. Widget 구조: Single Umbrella + WidgetSwitcher

**WBP_SessionUI** (신규, C++ base `USessionUIWidget`) 가 Phase 3 의 유일한 session-level widget. 내부에 WidgetSwitcher 1 개, panel 3 개:

| Slot | Panel | C++ base |
|---|---|---|
| 0 | TestProgress | `UTestProgressWidget` (기존 유지, `TestProgressWidget.h:18`) |
| 1 | Comparison | `UComparisonWidget` (기존 유지, `ComparisonWidget.h:17`) |
| 2 | FinalRanking | `UFinalRankingWidget` (기존 유지, `FinalRankingWidget.h:22`) |

`USessionUIWidget` 책임:
- `OnPhaseChanged` 단일 subscribe
- Phase → switcher active index 매핑:
  - `Test1`, `Test2`, `Test3`, `Test4` → slot 0
  - `Compare1v2` → slot 1
  - `FinalRanking` → slot 2
  - `Idle`, `WritingLog` → hidden (visibility=Collapsed)
- 활성화되는 panel 의 `RefreshFromSubsystem()` 호출 (필요 시 — 예: FinalRanking 의 button enabled 상태 sync)
- Subsystem reference 1 회 cache, panel 에 propagate

세 panel C++ base 는 **그대로 유지** — 각 phase 의 책임이 명확히 다름 (Next/Prev vs 2-button choice vs 4-slot ranking). 한 class 에 합치면 god widget (CLAUDE.md §6 "단일 책임" 위반).

### D2. Placement: Pawn 측면, configurable

**Default**:
- World-space anchor in `L_Main` (별도 actor `BP_SessionUIAnchor` 또는 session driver 의 child SceneComponent)
- Position: `PlayerStart` 기준 우측 +50cm, 정면 +20cm, Z=120cm (chest height)
- Rotation: widget normal 이 pawn 쪽 (slight inward yaw)
- **시야선 outside**: pawn 이 liftgate (정면 3m, Z≈1.6m) 를 바라볼 때 widget 이 peripheral vision 안에 들어와 시각 방해 최소

**Configurable**: 위 값은 `UEvaluationSessionSubsystem` 또는 `BP_SessionUIAnchor` 의 UPROPERTY 로 노출. 평가자 피드백 따라 조정 가능 (좌측 / 우측 / 거리 / 높이).

**Constraint** (CLAUDE.md §8 / R8 / ADR-007):
- 평가자가 손가락으로 닿을 수 있어야 함 — pawn standing position 에서 팔 뻗어 30~60cm 거리
- ADR-008 World-space 원칙 — head-locked 금지, body-locked (pawn attach) 도 피함. 순수 world-fixed anchor

**Vehicle 과의 관계**:
- Vehicle (정면 3m) 과 widget (우측 50cm) 은 spatial 으로 분리 — widget 이 vehicle 옆이 아님
- 평가자가 liftgate grab 하러 1~2m 걸어가도 widget 은 PlayerStart 옆에 남음 → Next/Prev 누르려면 약간 돌아서야 함
- 본 trade-off 는 의도된 것 — "조작 vs UI" 의 mental mode switch 가 spatial 으로 명확
- Iteration 후 "너무 멀다" 피드백 시 vehicle anchor 측면으로 anchor 이동 (UPROPERTY 만 변경)

### D3. Calibration widget 은 변경 없음

`WBP_CalibrationCheck` (Phase 1) 는 본 ADR 영향 받지 않음. Pre-session 단계 + R3 recenter re-validation 흐름이 session UI 와 분리. **총 widget = 2 종** (Calibration + SessionUI).

## Consequences

**Positive**:
- Widget anchor 1 개 — placement / lifecycle 관리 단순
- OnPhaseChanged subscribe 1 회 → race 없음
- WidgetSwitcher 의 native animation (fade / slide curve) 사용 가능
- C++ 책임 분리 유지 (panel C++ base 3 개) — SRP 보존
- Placement 가 liftgate 시야선 outside → R6 / §8 의 "조작 방해 금지" 명시화
- Phase 4+ 에 phase 추가 시 switcher slot 만 추가 — 확장 쉬움
- ADR-008 의 미해결 note ("Phase 3 mode toggle UI 위치 ADR") 해소

**Negative**:
- 평가자가 grab 위해 1~2m 걸어가면 widget 이 뒤에 — Next/Prev 누르려고 돌아서야 함 (mental mode switch 의도이지만 첫 평가자는 어색할 수 있음)
- Panel 이 hidden 일 때 자체 상태 (FinalRanking 의 ranking) 자동 reset 안 됨 — Subsystem.ResetRanking() 명시 호출 필요 (이미 ConfirmSession 시 호출됨)
- WidgetSwitcher 의 inner panel 은 즉시 instantiated → 약간의 idle memory (Phase 3 규모 < 1 MB, 무시 가능)
- Designer 협업 시 한 WBP 안에 3 panel 이 들어있어 layout 충돌 가능성 — 단 1인 개발이므로 무관

**Neutral**:
- 기존 W3.5 의 3 widget C++ base 는 그대로 유지 (DELETE 아님). BP 만 inner panel 화
- `WBP_TestProgress` / `WBP_Comparison` / `WBP_FinalRanking` 의 자산명도 유지 — 단지 top-level placement 가 아니라 `WBP_SessionUI` 의 child widget 으로 instance

## Verification

본 ADR 채택 후 Phase 3 W3.5 / W3.7 검증 시 다음 추가 확인:

1. `WBP_SessionUI` 가 L_Main 에 1 개만 spawn (host actor 1 개)
2. WidgetSwitcher 의 active index 가 `OnPhaseChanged` broadcast 따라 0/1/2 로 정확 전환
3. Idle / WritingLog phase 에서 SessionUI 가 hidden
4. Pawn 이 PlayerStart 에 서서 정면 (liftgate 방향) 을 바라볼 때, widget 이 시각 정면을 가리지 않음 (peripheral 가능)
5. Pawn 이 손 뻗어 widget 의 모든 버튼 (Next, Prev, 1, 2, 4-slot, Reset) poke 가능
6. Vehicle 의 Liftgate grab 동작 중 widget 이 시야 방해하지 않음
7. Vehicle (정면 3m) 과 widget 의 공간 분리 — 동시에 한 시야에 들어오나 overlap 없음

검증 실패 (특히 #4, #6) 시 anchor UPROPERTY 조정 또는 후속 ADR 로 placement 재검토.

## Notes

- 본 ADR 은 ADR-008 의 미해결 placement note 해소 — 평가자 좌측 50cm panel 보다 우측 50cm 채택 (right-hand dominant 가정, configurable 이므로 양손 모두 대응 가능)
- Calibration widget 의 placement (정면 1.5m, Z=1.6m, Phase1_Checklist:387) 는 그대로 — Session UI 와 anchor 다름. 두 widget 이 동시 visible 인 시점 없음 (Calibration → Session 순차)
- 향후 "평가자가 widget 까지 돌아서기 귀찮음" 피드백 받으면 anchor 를 vehicle 측면으로 이동 가능 — UPROPERTY 만 변경, C++ / BP 구조 변경 없음
- Phase 4 의 height ruler / zone visualizer 는 session UI 와 별도 World-space (차량 옆) — 본 ADR 영향 없음
- Panel switching 시 animation 필요 여부는 Phase 3 검증 후 결정. Default = instant switch (가장 단순)
- 본 commit 은 planning only — `USessionUIWidget` C++ scaffolding, `WBP_SessionUI` 자산 생성, BP_SessionUIAnchor 배치 는 후속 PR / Editor 작업
