# ADR-004 — BP_VRPawn Extends IsdkSamplePawn (Child BP Pattern)

**Date**: 2026-05-13
**Status**: Accepted
**Phase**: 1

## Context

Phase1_Kickoff.md W1.2 spec은 `Pawn` 기반으로 `BP_VRPawn`을 생성하고 ISDK Sample에서 hand rig prefab을 migrate해 child component로 붙이는 흐름이었다.

조사 결과 `OculusInteractionSamples` plugin이 `IsdkSamplePawn` (`Plugins/OculusInteractionSamples/Content/Blueprints/IsdkSamplePawn.uasset`) 을 이미 제공한다:

- C++ base `AIsdkSamplePawnClass`: EnhancedInputComponent + ISDK console parser 등록 (최소)
- BP 본체: Hand tracking, hand rig (양손), ISDK input 바인딩이 이미 셋업되어 동작 가능

Sample pawn을 활용하면 W1.2의 hand rig migrate 단계가 통째로 사라지고, Meta가 검증한 setup 위에 우리 customization만 추가하면 된다.

선택지:

| Option | 방식 | 평가 |
|---|---|---|
| A | `IsdkSamplePawn`을 Default Pawn으로 직접 사용 | 가장 빠름. Customization 어려움 / plugin path 종속 |
| **B** | **`BP_VRPawn` 을 `IsdkSamplePawn`의 child BP로** | **Hand tracking 상속 + override 자유. Plugin upgrade 안전** |
| C | `IsdkSamplePawn`을 `BP_VRPawn`으로 Migrate (duplicate) | 완전 소유. Plugin upgrade benefit 없음 |

## Decision

**Option B 채택.** `BP_VRPawn`은 `IsdkSamplePawn`을 parent class로 가지는 child Blueprint로 생성한다.

`BP_VRPawn`에서 추가로 override / 보강할 것:

1. **BeginPlay override**:
   - `Set Tracking Origin → Floor Level` 명시 호출 (CLAUDE.md §3 R1, 필수)
     - parent에서 이미 호출한다 해도, 우리 BP에서 한 번 더 보장
   - 우리 `IMC_IsdkHand` (또는 ISDK 기본 IMC가 parent에 없으면 직접 추가)
2. **Wrist UI attach point** (W1.5):
   - 좌측 hand mesh의 wrist socket에 WidgetComponent 추가
3. **Calibration entry hook** (W1.4):
   - 평가 진입 전 `WBP_CalibrationCheck` spawn 트리거
4. **Custom variables** (Category=Calibration, UI, Debug 등 CLAUDE.md §6 규칙)

ADR-001 (ISDK 기반)을 더 강하게 따르는 결정이다. R5 (Hand grab은 ISDK 기반) 와 정렬됨.

## Consequences

**Positive**:
- W1.2 작업량 대폭 감소 (hand rig migrate 단계 삭제)
- Meta 검증된 hand tracking setup 그대로 활용 → 안정성 ↑
- ISDK 업데이트 시 hand tracking 개선이 우리 pawn에 자동 반영
- Plugin source는 건드리지 않으므로 plugin upgrade 안전
- Sample / 예제 코드와 동일 패턴 → 학습 자료 풍부

**Negative**:
- `IsdkSamplePawn` 내부 구조 변경 시 우리 child BP 영향 가능 (parent contract 변경 위험)
- `IsdkSamplePawn`이 갖고 있는 sample용 debug / demo 로직이 남아있을 수 있음 (필요 시 child에서 override / disable)
- `OculusInteractionSamples` plugin 의존성 추가 — Phase 4 이후 plugin 제거 시 fallback 경로 필요

**Neutral**:
- `Plugins/OculusInteractionSamples/` 는 그대로 git에 포함 (Plugin source는 commit 대상)
- W1.2 검증 기준은 유지 (HMD = Camera 일치, 손 트래킹, 손목 회전)

## Verification

Child BP 채택 후 다음 R1 검증이 필수:

1. PIE 또는 VR Preview에서 BP_VRPawn 의 `Set Tracking Origin → Floor Level` 호출 후 HMD Z 값 확인
2. Camera world location Z 가 평가자 실제 키 (mm) 와 일치 (±5cm)
3. ISDK Hand tracking 양손 모두 동작
4. Quest Guardian 재설정 후에도 floor 가 발 밑에 정확히 위치

위 검증이 실패하면 Option C (Migrate / duplicate) 로 전환 고려.

## Notes

- `IsdkSamplePawn` BP 내부 (Component 트리, BeginPlay 그래프) 는 binary `.uasset` 이라 git diff로 보이지 않음. 변경 추적은 UE Editor에서 직접 확인 필요
- Plugin upgrade (v78 → 차후 버전) 시 `IsdkSamplePawn` parent contract 변경 여부를 release note에서 확인. 변경 있으면 본 ADR 재검토
- `IsdkSamplePawn`이 만약 향후 plugin에서 제거되거나 deprecated 되면 Option C로 즉시 전환 (Migrate하여 `BP_VRPawn` 안에 흡수)
- 본 결정은 Phase1_Kickoff.md §9 "확정된 Decision" 의 "Hand grab = ISDK 기반" 항목을 더 구체화한 형태
