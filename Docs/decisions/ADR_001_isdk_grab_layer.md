# ADR-001 — ISDK-based Hand Grab Layer

**Date**: 2026-05-13
**Status**: Accepted
**Phase**: 1

## Context

Hand grab 인터랙션을 어떻게 구현할지 선택이 필요했다. 두 가지 옵션:

- 자체 pinch detection / grab 로직 작성
- 검증된 SDK (Meta XR Interaction SDK) 사용

자체 구현은 tracking confidence, pinch debouncing, finger curl 판정 등 edge case가 많고 평가자 간 일관성 확보가 어렵다. ISDK는 이미 Meta가 Quest hand tracking에 최적화하여 제공하는 컴포넌트가 있다.

## Decision

Meta XR Interaction SDK **v78**의 `IsdkGrabbableComponent` 기반으로 모든 grab 인터랙션을 통일한다. 자체 pinch / grab 로직 작성을 금지한다.

CLAUDE.md §3 R5에 박제됨.

## Consequences

**Positive**:
- 안정적 grab 판정 (tracking confidence, debouncing, pinch detection이 SDK 내장)
- 평가자 간 일관성 확보 (custom 코드 차이 없음)
- Meta 공식 sample과 동일 패턴 → 학습 자료 풍부
- Phase 2 이후 추가 인터랙션 (poke, ray, two-hand grab 등) 확장 용이

**Negative**:
- SDK 버전 의존성 발생 — 현재 v78 lock-in
- SDK upgrade 시 호환성 검증 필요
- ISDK가 다루지 못하는 edge case는 우회로 해결해야 함

**Neutral**:
- ISDK Sample (5.5.4-v78 branch) 참조 필수 — hand rig prefab을 본 프로젝트로 migrate

## Notes

- Phase 2 진입 시 ISDK API surface 점검
- v85+ upgrade는 별도 ADR 후 결정
- ISDK가 제공하지 않는 컴포넌트가 필요해질 때 본 ADR 재검토
