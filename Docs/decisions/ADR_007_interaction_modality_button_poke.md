# ADR-007 — Interaction Modality: Button Poke Over Gesture

**Date**: 2026-05-15
**Status**: Accepted
**Phase**: 1

## Context

Phase 1 W1.4 (Calibration) 의 confirm 단계 spec 은 "ISDK Pinch action 으로 확인" 이었고, 일반적으로 VR / hand-tracking 환경에서는 다음 입력 양식이 있다:

- **Gesture / pose detection**: pinch, thumbs-up, point 등 손 모양 감지
- **Button poke**: 평가자가 손가락으로 World-space 의 버튼을 직접 누름 (ISDK Poke Interactor)
- **Ray pointer**: 멀리 있는 버튼을 ray + pinch / trigger 로 선택
- **Voice / external**: 본 프로젝트 범위 외

평가 도구 (PQDQ subjective evaluation) 에서 confirm / discrete 입력 (calibration pass, mode 변경, recalibrate 등) 을 어떤 양식으로 통일할지 결정 필요.

본 ADR 작성 시점 (Phase 1 W1.4 검증) 에서 발견된 사항:

- ISDK Pinch detection 은 false positive 가 잦음 (특히 tracking confidence 낮을 때, 자연스러운 손 동작 중 부정확)
- 평가자가 "어떤 gesture 를 해야 하는지" 학습 부담 — 평가 시작 전 onboarding 필요
- 평가자마다 손 크기 / pose 습관 차이 → 일관성 결여 가능
- Button poke 는 시각적 affordance 명확 (보면 누르고 싶음), 물리적 metaphor 직관적
- 다만 hand tracking 에서 button poke 는 손가락이 버튼에 **물리적으로 닿아야** 동작 → 버튼 위치 / 크기 / 평가자 자세 고려 필요

## Decision

**모든 confirmation / discrete 입력 (calibration pass, mode 토글, recalibrate, Phase 2 이후 mode 버튼 등) 은 button poke 로 통일한다.**

- Custom pose / gesture / pinch detection 작성 금지 (R8 신규)
- ISDK pinch detection 은 **grab interaction 의 일부로서만** 허용 (R5 / ADR-001 — 물건을 잡는 자연스러운 동작)
- Confirm / 선택 / 토글 → **항상 button poke**
- 버튼은 평가자가 닿을 수 있는 위치에 배치 (Wrist UI = 항상 닿음, World-space 버튼 = 평가자 정면 30~60cm 정도)

Phase 1 W1.4 의 "Pinch 확인" 도 button poke 로 변경:
- WBP_CalibrationCheck 에 Confirm 버튼 추가 (평가자 정면 가까이에 widget 배치)
- ISDK Poke Interactor (IsdkSamplePawn 에 이미 내장) 가 button OnClicked 발생시킴
- BP_CalibrationGate 는 button OnClicked → `HandlePinchInput()` (Phase 2 진입 전 `HandleConfirm()` 등으로 rename 권장)

## Consequences

**Positive**:
- 평가자 학습 부담 ↓ (gesture cheatsheet 불필요)
- False positive ↓ (의도하지 않은 confirm 감소)
- 평가자 간 일관성 보장 (gesture variance 없음)
- 시각적 affordance 명확 — 버튼이 보이면 누르고 싶음 (UX 직관성)
- Phase 2 이후 mode toggle / 평가 비교 등에서 일관된 UX

**Negative**:
- 버튼이 평가자가 닿을 수 있는 위치에 배치돼야 함 — UI 설계 제약
- 멀리 있는 object (예: 차량 뒤 어딘가) 와의 직접 인터랙션은 ray pointer 등 별도 UI 필요 (Phase 2 이후 검토)
- Phase 1 W1.4 spec 변경 (Pinch → button) — kickoff 문서 갱신 필요
- C++ `HandlePinchInput()` 이름 misleading — 향후 rename 필요

**Neutral**:
- ADR-001 (ISDK grab) 영향 없음 — grab 은 별도 modality 로 유지
- Wrist Panel (W1.5) 은 손목에 항상 있으므로 proximity 문제 없음

## Notes

- 향후 mode 비교 / 차량 swap UI (Phase 4) 도 본 ADR 따라 button poke 기반
- "버튼이 너무 멀어서 닿기 힘듦" 등 UX 이슈는 `Docs/lessons_learned/` 에 박제 → 위치 / 크기 조정
- Phase 5 (가정) 의 comparison logging 위젯도 본 ADR 적용
- ISDK Pinch 가 절대적으로 필요한 use case 가 향후 발견되면 본 ADR 재검토 (예: 양손 grab 의 두 번째 hand 의 confirm)
- C++ `ACalibrationGateActor::HandlePinchInput()` 은 기능상 BP wiring 그대로 두고 (호출자만 button OnClicked), 이름은 Phase 2 진입 전 `HandleConfirm()` 으로 rename
