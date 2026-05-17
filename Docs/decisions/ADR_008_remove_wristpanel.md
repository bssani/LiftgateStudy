# ADR-008 — Remove WristPanel: World-space UI as Primary

**Date**: 2026-05-15
**Status**: Accepted
**Phase**: 1
**Supersedes (partial)**: CLAUDE.md §8 "Primary UI = Wrist Panel" (v0.4 까지)

## Context

Phase 1 W1.5 에서 좌측 손목에 Wrist Panel (`WBP_WristPanel` + `UWristPanelWidget`) 을 띄우는 방식을 채택했었다. CLAUDE.md §8 v0.4 까지 "Primary UI = Wrist Panel" 로 박제됨.

Phase 1 검증 단계에서 다음 사항이 드러남:

- **Visibility 로직 fragile**: dot(handUp, WorldUp) ≥ threshold 방식이 ISDK hand mesh 의 up vector 축 매핑에 의존. SDK 버전 / hand pose 마다 조정 필요. 평가자마다 threshold 튜닝이 미세하게 다를 수 있음
- **Attach socket 복잡**: MotionController 에는 socket 이 없고, hand skeletal mesh 의 bone (`wrist_l` 등) 에 직접 attach 해야 함. ISDK hand rig 의 component wrapping 으로 socket dropdown 비어있는 경우 발생
- **Material orientation 함정**: `Widget3DPassThrough_Masked_OneSided` 가 한 방향만 보임. Rotation 잘못 잡으면 평가자 쪽이 아닌 hand mesh 뒤쪽으로 향함
- **UX 가치 의문**: 평가 도구의 본질은 **liftgate 인터랙션** (Phase 2~). Calibration 통과 후 평가 중에 HMD Height / Floor 상태를 실시간으로 볼 가치가 낮음. Recalibrate 도 자주 필요한 동작이 아님
- **ADR-007 (R8) 와의 정합성**: button poke 기반인데, wrist 의 작은 버튼은 다른 손이 닿기 어색함. Wrist 가 닿기 쉽다는 가정이 실제로는 양손 hand-tracking 의 조작 정확도 한계로 깨짐

평가 도구의 UI 영역:

1. **Calibration 진입** (Phase 1 W1.4) — World-space widget 정면 1.5m → 정상 (변경 없음)
2. **평가 중 상태 확인** — 가치 낮음, 제거 가능
3. **Recalibrate trigger** — 빈도 낮음, 다른 방법 가능 (예: 차량 옆 작은 버튼, 또는 calibration widget 재출현 트리거)
4. **Mode toggle** (Phase 3) — World-space widget 평가자 정면 / 차량 옆 가능
5. **Vehicle swap** (Phase 4) — World-space widget 평가자 정면

→ 모든 UI 를 World-space + button poke 로 통일 가능. Wrist Panel 은 over-engineering.

## Decision

**WristPanel 을 프로젝트에서 완전히 제거한다.**

- `UWristPanelWidget` (C++ class) 삭제
- `WBP_WristPanel` (BP asset) 삭제
- `BP_VRPawn` 의 `WristUIAnchor` WidgetComponent 삭제 + Event Graph 의 cache / Tick wiring 제거

**Primary UI 정의 변경**: Wrist Panel → **World-space widget (평가자 proximity 내)**. 모든 UI 는 ADR-007 (R8) 의 button poke 원칙 + §8 proximity 배치 규칙을 따른다.

Phase 3 mode toggle / Phase 4 vehicle swap UI 는 World-space widget 으로 새로 설계 (해당 phase 진입 시 ADR 추가 가능).

## Consequences

**Positive**:
- Phase 1 단순화 — W1.5 work item 제거, 검증 항목 감소
- Hand-pose visibility 복잡도 제거
- ADR-007 button poke 원칙과 정합성 ↑ (모든 UI 가 World-space, proximity 명확)
- C++ 모듈 의존성 표면 감소 (`SetHandUpVector` 등 API surface 제거)
- 평가자 onboarding 단순화 (학습 부담 ↓)

**Negative**:
- 평가 중 HMD Height / Floor 상태 실시간 확인 불가 (Calibration 단계에서만 표시). 평가 진행 중 환경 변화 (예: 평가자가 옮긴 위치, recenter) 감지 어려움
- Recalibrate trigger 별도 설계 필요 (Phase 1 검증 단계에서는 PIE 재시작으로 대체)
- 향후 mode toggle UI 의 World-space 위치 결정이 새 ADR 대상

**Neutral**:
- ADR-007 (button poke) 영향 없음 — 오히려 강화
- ADR-001 (ISDK grab), ADR-004 (BP_VRPawn = IsdkSamplePawn child) 영향 없음
- BP_VRPawn 의 `WristUIAnchor` 컴포넌트만 제거; pawn 자체 구조 유지

## Implementation Order (중요)

**Linux 쪽 (이 PR)** 과 **Windows 쪽 UE Editor 작업** 의 순서를 맞춰야 compile error 회피 가능. 다음 순서로 진행:

1. **사용자 (Windows, UE Editor)**: 본 PR 머지 **전에** 다음 작업:
   - BP_VRPawn 열기 → `WristUIAnchor` Component 우클릭 → Delete
   - BP_VRPawn 의 Event Graph 에서 WristWidget 관련 노드 (SetHandUpVector, SetCalibrationSnapshot, cache, OnRecalibrateRequested bind 등) 모두 제거
   - Content Browser → `WBP_WristPanel.uasset` 삭제
   - 컴파일 OK 확인 후 commit & push (BP 변경분)
2. **본 PR 머지** (C++ 파일 삭제 + docs 업데이트)
3. **사용자 (Windows)**: `git pull` → 컴파일 OK 확인

## Notes

- 향후 비슷한 wrist-attached UI 가 다시 필요해지면 본 ADR 재검토
- Calibration widget (Phase 1 W1.4) 은 변경 없음 — 정면 1.5m World-space 유지
- Phase 3 진입 시 mode toggle UI 위치 결정 ADR 추가 (예: 평가자 좌측 50cm 떠있는 panel, 또는 차량 옆 stand)
- ADR-007 의 R8 (button poke) + §8 proximity 배치 규칙은 그대로 유효
