# ADR-009 — ALiftgate Root Component Pattern (Meta ISDK Sample)

**Date**: 2026-05-18
**Status**: Accepted
**Phase**: 2 / 3

## Context

W2.1 의 `ALiftgate` 초기 디자인은 `HingePivot` 을 **root component** 로 두었다. 사용자가 BP_Liftgate 셋업 중 Meta ISDK 의 `GrabbableBox` sample 을 점검한 결과 패턴이 다름을 발견:

```
GrabbableBox (Self)
└── DefaultSceneRoot              ← root, 회전 안 함
    └── Box_BoxBase                ← body mesh
        └── Hinge                  ← Location/Rotation 편집 가능 (child)
            ├── Box_BoxLid          ← 회전하는 mesh
            └── IsdkGrabbable      ← grabbable 도 Hinge child
```

핵심:

1. **Hinge 가 root 가 아님** — child 라서 RelativeLocation/Rotation 이 BP Details 에서 편집 가능
2. **IsdkGrabbable 이 회전 컴포넌트 (Hinge) 의 child** — ISDK GrabTransformer 가 grabbable 의 parent component 를 회전 대상으로 잡음
3. **회전 mesh 도 Hinge child** — Hinge 회전 시 함께 swing

UE 규칙상 root component 의 RelativeLocation/Rotation 은 Actor 의 Transform 과 항상 일치 → Details 에서 숨겨짐. Hinge 가 root 면 "hinge 의 actor 내 위치" 를 따로 설정 못 함.

## Decision

`ALiftgate` 의 component 구조를 Meta sample 패턴으로 변경:

```cpp
ALiftgate::ALiftgate()
{
    USceneComponent* DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
    RootComponent = DefaultSceneRoot;

    HingePivot = CreateDefaultSubobject<USceneComponent>(TEXT("HingePivot"));
    HingePivot->SetupAttachment(RootComponent);

    LiftgateMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LiftgateMesh"));
    LiftgateMesh->SetupAttachment(HingePivot);
}
```

BP_Liftgate 셋업 규칙:
- IsdkGrabbable 을 **HingePivot 의 child** 로 배치 (LiftgateMesh 의 child 아님)
- IsdkGrabTransformer 는 Actor root level 컴포넌트 (위치 무관)
- IsdkGrabbable 의 `GrabTransformerComponent` UPROPERTY 가 IsdkGrabTransformer 를 가리키도록

## Consequences

**Positive**:
- HingePivot 의 RelativeLocation/Rotation 이 BP 에서 편집 가능 → hinge 위치 / 축 actor 내에서 자유 조정
- ISDK GrabTransformer 가 grabbable 의 parent (= HingePivot) 를 자동으로 회전 → 별도 wiring 불필요
- Meta sample 과 동일 패턴 → 학습 / 디버그 참조 자료 풍부
- ADR-001 (ISDK grab) 와 자연스럽게 정합

**Negative**:
- Component 트리 한 단계 깊어짐 (DefaultSceneRoot 추가)
- Phase 2 W2.1 의 docs (이전 root=HingePivot 가정) 갱신 필요

**Neutral**:
- `SetAngle()` 의 `HingePivot->SetRelativeRotation(...)` 로직은 변경 없음
- Phase 2 의 grab → 회전 → auto-complete 흐름 그대로

## Notes

- 본 ADR 은 PR `claude/phase3-pr1-aliftgate-root-refactor` 에서 구현
- BP_Liftgate 의 IsdkGrabbable reparent 는 BP 측 작업 (PR 머지 후 Editor 에서)
