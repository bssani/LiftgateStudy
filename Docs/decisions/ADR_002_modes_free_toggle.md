# ADR-002 — Liftgate Modes as Free Toggle, Not Vehicle Property

**Date**: 2026-05-13
**Status**: Accepted
**Phase**: 1

## Context

Liftgate 동작은 4가지 mode로 정의됨:

- Manual-Full
- Manual-Assist
- Power-Auto
- Power-Hybrid

각 차량은 실제로 특정 mode만 지원한다 (예: A 차량은 Power-Auto만 있음). 이를 시뮬레이션에 어떻게 반영할지 선택이 필요했다:

- **(A) 차량 속성으로 고정**: 각 차량은 `SupportedModes` metadata를 가짐. UI는 해당 mode만 선택 가능
- **(B) 자유 토글**: 평가자가 모든 차량에서 4가지 mode 모두 시도 가능

본 도구의 목적은 "주관적 평가 비교"이고, 평가자가 같은 차량에서 다른 mode를 비교할 필요가 있다.

## Decision

**자유 토글 (B 옵션)** 채택. 차량 metadata에 `SupportedModes` 필드를 두지 않는다. 모든 차량은 4가지 mode 전부 평가 가능하다.

## Consequences

**Positive**:
- 같은 차량을 4가지 mode로 비교 평가 가능 (핵심 평가 가치)
- 차량 metadata 단순화 (mode 정의 불필요)
- UI 단순화 (mode 가용성 분기 없음)
- 신규 차량 추가 시 mode mapping 정의 부담 없음

**Negative**:
- 실차에 없는 mode도 체험 가능 → 실차 충실도 ↓
- 평가자가 "이 차의 실제 mode는 뭔지" 별도 인지 필요
- 평가 결과 해석 시 "이 mode가 실제 차량에 존재하는지" 외부 컨텍스트 필요

**Neutral**:
- mode 토글 UI는 Wrist Panel에서 통일 제공 (Phase 3)

## Notes

- 향후 실차 충실도가 요구되면 차량별 "Recommended Mode" 표시 추가 가능 (강제 제한은 안 함)
- 본 결정은 PQDQ 평가 도구로서의 비교 가치를 우선한 것. 트레이닝 / 사용법 시뮬레이션 용도가 되면 재검토.
