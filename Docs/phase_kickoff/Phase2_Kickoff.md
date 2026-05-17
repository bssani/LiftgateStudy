# Phase 2 Kickoff — Liftgate Core

> **For Claude Code**: Phase 2 의 작업 명세. 모든 작업은 `CLAUDE.md` 를 우선한다. 충돌 시 사용자에게 확인 후 진행.

---

## 1. Phase 2 Goal

평가자가:
1. 더미 차량 (`L_Vehicle_Dummy`) 후면의 liftgate 를 ISDK grab 으로 잡고
2. 손을 위로 들어올려 liftgate 를 회전시키고
3. **40° 이상에서 손을 놓으면 자동으로 완전 개방 (85°)** — Manual-Assist mode 핵심

이 동작을 검증한다.

본 phase 는 **Manual-Assist mode 만** 다룬다. Manual-Full / Power-Auto / Power-Hybrid 는 Phase 3.

---

## 2. Pre-conditions

- [ ] Phase 1 §7 검증 통과 (calibration, dummy 차량, BP_VRPawn 등)
- [ ] C++ 모듈 `LiftGateStudy` 컴파일 통과
- [ ] `BP_VRPawn` = `IsdkSamplePawn` 의 child — ISDK Grab Interactor 가 양손에 활성 (ADR-004)
- [ ] ISDK Sample 의 grab 예제 (`BP_GrabbableObject` 등) 확인 가능

---

## 3. Folder Structure (Phase 2 종료 시)

```
Source/LiftGateStudy/
├── Public/
│   ├── Calibration/      ← Phase 1
│   ├── Liftgate/         ← Phase 2 신규
│   │   ├── LiftgateTypes.h        (ELiftgateMode enum)
│   │   └── Liftgate.h             (ALiftgate)
│   └── LiftgateStudyGameMode.h
└── Private/
    ├── Calibration/      ← Phase 1
    ├── Liftgate/         ← Phase 2 신규
    │   └── Liftgate.cpp
    └── LiftgateStudyGameMode.cpp

Content/Blueprints/
├── BP_CalibrationGate.uasset   ← Phase 1
├── BP_LiftgateStudyGameMode    ← Phase 1
├── BP_VRPawn.uasset            ← Phase 1
└── BP_Liftgate.uasset          ← Phase 2 신규
```

---

## 4. Work Items

### W2.0 — Phase 1 Leftover Cleanup

> Phase 1 검증 중 keyboard Space 임시 wiring + `HandlePinchInput()` misleading 이름을 정리.

**작업**:
1. **C++ rename**: `ACalibrationGateActor::HandlePinchInput()` → `HandleConfirm()`
   - 헤더 / cpp 모두 변경. 주석도 업데이트
   - 컴파일 OK 확인
2. **WBP_CalibrationCheck Layout 수정**:
   - `Button_Confirm` widget 추가 (Vertical Box 하단)
   - Visibility binding: `Status == Pass` 일 때만 visible
   - "Press Confirm" 텍스트
3. **BP_CalibrationGate Event Graph 수정**:
   - 기존 Space 키 → HandlePinchInput wiring 제거 (Phase 1 임시)
   - cached widget reference 의 Button_Confirm 의 OnClicked → `HandleConfirm()` 호출 wiring
4. (선택) Phase 1 검증에서 발견된 다른 함정이 있으면 `Docs/lessons_learned/` 추가

**검증**:
- VR Preview → Calibration widget 표시 → Status=Pass 시 Confirm button 표시
- 손가락으로 Confirm button poke → widget 사라짐 (BP_CalibrationGate Destroy)
- Keyboard Space 는 더 이상 동작하지 않음

---

### W2.1 — ALiftgate C++ Base Class

**작업**:
1. `Source/LiftGateStudy/Public/Liftgate/LiftgateTypes.h` 생성:
   ```cpp
   UENUM(BlueprintType)
   enum class ELiftgateMode : uint8
   {
       ManualFull   UMETA(DisplayName="Manual-Full"),
       ManualAssist UMETA(DisplayName="Manual-Assist"),
       PowerAuto    UMETA(DisplayName="Power-Auto"),
       PowerHybrid  UMETA(DisplayName="Power-Hybrid")
   };

   UENUM(BlueprintType)
   enum class ELiftgateState : uint8
   {
       Closed       UMETA(DisplayName="Closed"),
       Grabbed      UMETA(DisplayName="Grabbed"),
       AutoOpening  UMETA(DisplayName="AutoOpening"),
       Open         UMETA(DisplayName="Open")
   };
   ```
2. `Source/LiftGateStudy/Public/Liftgate/Liftgate.h` 생성. Base: `AActor`, `Blueprintable`.
   - Components:
     - `USceneComponent* HingePivot` (root) — 회전 중심
     - `UStaticMeshComponent* LiftgateMesh` — HingePivot 자식, mesh 본체
   - UPROPERTY (Category="Mode"):
     - `ELiftgateMode CurrentMode = ELiftgateMode::ManualAssist;` (Phase 2 는 Assist 만)
   - UPROPERTY (Category="Liftgate", EditAnywhere, BlueprintReadWrite):
     - `float MaxAngle_deg = 85.f;`
     - `float ThresholdAngle_deg = 40.f;`     // Manual-Assist auto-complete 분기
     - `float AutoOpenDuration_sec = 1.2f;`
     - `float GrabSensitivity_deg_per_cm = 1.5f;`   // 손 1cm 당 회전 각도
   - UPROPERTY (Category="Liftgate", BlueprintReadOnly):
     - `float CurrentAngle_deg = 0.f;`
     - `ELiftgateState State = ELiftgateState::Closed;`
   - 함수 (BlueprintCallable):
     - `void BeginGrab(const FVector& HandWorldLocation)`
     - `void UpdateGrab(const FVector& HandWorldLocation)`
     - `void EndGrab()`
   - Tick: State == AutoOpening 이면 lerp angle → MaxAngle
   - 내부: GrabStartAngle, GrabStartHandLocation 캐싱
3. `Source/LiftGateStudy/Private/Liftgate/Liftgate.cpp` 구현:
   - Constructor: Components 생성, root 설정
   - BeginGrab: State = Grabbed, GrabStartAngle = CurrentAngle, GrabStartHandLocation = handLoc
   - UpdateGrab: delta = (handLoc.Z - GrabStartHandLocation.Z) cm → angle delta = delta × GrabSensitivity. Clamp [0, MaxAngle]. HingePivot 회전 갱신
   - EndGrab: 
     - Manual-Assist + CurrentAngle ≥ ThresholdAngle → State = AutoOpening
     - 그 외 → State = Closed (Phase 2 는 멈춤. 중력 / damping 없음)
   - Tick: AutoOpening 이면 lerp / timer 로 CurrentAngle → MaxAngle, 도달 시 State = Open

**검증**:
- C++ 모듈 컴파일 OK
- BP child 의 Class Defaults 에서 모든 UPROPERTY 보임 + 조정 가능

---

### W2.2 — BP_Liftgate (BP Child + ISDK Grab Wiring)

**작업**:
1. Content/Blueprints/ → Blueprint Class → All Classes → `Liftgate` 선택
2. 이름: `BP_Liftgate`
3. **Mesh 지정**: `LiftgateMesh` 의 Static Mesh 를 다음 중 하나로
   - **(MVP)** Engine 기본 Cube, Scale (0.8, 1.4, 0.08) — 80×140×8cm placeholder
   - 또는 `SM_Liftgate` 가 있다면 그것
4. **IsdkGrabbableComponent 추가** (R5, ADR-001):
   - Components 패널 → Add Component → `IsdkGrabbable...` 검색 → ISDK Grabbable Component 추가
   - LiftgateMesh 의 자식으로 (또는 root 자식)
   - Configurable: hand chirality (Both / Left / Right)
5. **Event Graph 에서 ISDK Grab event wiring**:
   - ISDK Grabbable 의 `OnGrabBegin` (또는 ISDK v78 의 동등 event) → 손의 World location 가져와 `BeginGrab(handLoc)` 호출
   - ISDK Grabbable 의 `OnGrabUpdate` (또는 Tick during grab) → `UpdateGrab(handLoc)`
   - ISDK Grabbable 의 `OnGrabEnd` → `EndGrab()`
6. **Handle marker** (선택): mesh 의 grab 가능 영역 시각화 — 작은 sphere 또는 material 표시

**ISDK 정확한 event 이름은 v78 sample 확인 필요**. 만약 IsdkGrabbableComponent 가 BlueprintAssignable delegate 가 아닌 다른 방식이면 ADR-009 신규 작성.

**검증**:
- BP_Liftgate 컴파일 OK
- Editor 에서 Place Actor → Mesh visible
- ISDK Grabbable Component 가 hand interactor 와 hit 됨 (debug 표시)

---

### W2.3 — Grab-driven Rotation (Manual-Full 동작 핵심)

**작업**:
1. BP_Liftgate 의 grab event wiring (W2.2 step 5) 가 ALiftgate 의 BeginGrab/UpdateGrab/EndGrab 호출 확인
2. `L_Vehicle_Dummy` 의 후면 marker mesh 위치에 BP_Liftgate 배치 (또는 W2.5 에서 본격 배치)
3. VR Preview → 평가자가 liftgate handle 위치 잡으면 → liftgate 회전 확인

**튜닝**:
- `GrabSensitivity_deg_per_cm` 가 너무 둔감하면 손 많이 들어야 함, 너무 예민하면 미세 조작 어려움
- 시작값 1.5 (즉 손을 60cm 들면 90° 회전). 실측 후 조정

**검증**:
- 손으로 잡고 위로 → liftgate 가 회전하여 위로 들림
- 손 놓으면 (Phase 2 는 Manual-Assist 만이라) 40° 미만에서는 그 자리에 멈춤, 40° 이상이면 W2.4 의 auto-complete

---

### W2.4 — Manual-Assist 40° Auto-Complete

**작업**:
1. ALiftgate.cpp 의 `EndGrab()` 에서:
   - `CurrentMode == ManualAssist && CurrentAngle_deg >= ThresholdAngle_deg` → State = AutoOpening
2. Tick 에서 State == AutoOpening:
   - timer / lerp 로 `AutoOpenDuration_sec` 동안 `CurrentAngle_deg` → `MaxAngle_deg`
   - 도달 시 State = Open
3. 시각 피드백 (선택): AutoOpening 상태일 때 hand visual 이 grab 자세에서 풀림

**검증**:
- 40° 이상에서 손 놓으면 1.2 초 동안 자동 개방 (smooth lerp)
- 40° 미만에서 손 놓으면 그 자리 멈춤
- ThresholdAngle / MaxAngle / AutoOpenDuration 모두 BP child Class Defaults 에서 조정 가능

---

### W2.5 — L_Vehicle_Dummy 에 BP_Liftgate 배치

**작업**:
1. `L_Vehicle_Dummy` 열기
2. 기존 marker mesh (handle 예상점) 제거 또는 hide
3. BP_Liftgate 배치:
   - Location: 차량 박스의 후면 상단 (mid-size SUV 기준 약 Z=1.2m, X=후면)
   - Rotation: hinge axis 가 차량 좌우 (Y 축) 방향
   - 즉 liftgate 가 닫혀있을 때 차량 후면을 막고 있고, 위로 회전하면 열리는 자세
4. 평가자 (BP_VRPawn) 위치에서 liftgate handle 이 닿는 거리인지 확인 (R8: proximity)

**검증**:
- 평가자가 차량 뒤로 가서 liftgate 앞에 섬
- Handle 영역이 평가자 손에 닿는 거리
- Grab → 회전 → release → auto-complete 전체 흐름 동작

---

### W2.6 — VR Preview 전체 검증

Phase 2 §7 verification 의 모든 항목 통과 확인.

---

## 5. 관련 ADR

| ADR | 주제 | Phase 2 영향 |
|---|---|---|
| ADR-001 | ISDK 기반 Hand Grab | W2.2 `IsdkGrabbableComponent` 사용 핵심 |
| ADR-002 | Mode 자유 토글 | Phase 3 에서 활용. Phase 2 는 Manual-Assist 고정 |
| ADR-003 | 평가 데이터 로깅 없음 | grab time / angle / speed 등 절대 로깅 금지 |
| ADR-004 | BP_VRPawn = IsdkSamplePawn child | grab interactor 가 BP_VRPawn 에 이미 활성 |
| ADR-005 | C++ 로직 + BP 레이아웃 hybrid | ALiftgate C++ + BP_Liftgate child 패턴 |
| ADR-007 | Button poke confirmation | W2.0 의 Confirm button cleanup |
| ADR-008 | World-space UI | Phase 2 직접 영향 없음 |

(ISDK grab event API 가 spec 과 크게 다르면 ADR-009 신규 작성)

---

## 6. Deliverables

Phase 2 완료 시점:

1. **C++**: `ELiftgateMode`, `ELiftgateState`, `ALiftgate` 컴파일
2. **BP**: `BP_Liftgate` (parent = `ALiftgate`) + ISDK grab wiring
3. **Level**: `L_Vehicle_Dummy` 후면에 `BP_Liftgate` 배치
4. **Working build**: VR Preview 에서
   - Calibration 통과 → 차량 후면 접근
   - Liftgate handle grab → 위로 들면 회전 → 40°+ 에서 release → auto-complete to 85°
5. **Phase 1 leftover** cleanup: `HandleConfirm` rename, Confirm button wiring
6. **ADR-009** (필요 시): grab-rotation API decisions

---

## 7. Verification Criteria

사용자 직접 확인:

- [ ] C++ 모듈 컴파일 (VS 2022)
- [ ] BP_Liftgate Class Defaults 의 모든 UPROPERTY 노출 + 조정 가능
- [ ] L_Vehicle_Dummy 진입 → 차량 후면 liftgate visible + handle 위치 자연스러움
- [ ] Liftgate 를 손으로 잡을 수 있음 (ISDK grab 트리거)
- [ ] 손 들어올리면 liftgate 회전 (smooth, 끊김 없음)
- [ ] 40° 미만에서 손 놓으면 그 자리 멈춤
- [ ] 40° 이상에서 손 놓으면 1.2 초 동안 자동 완전 개방 (85°)
- [ ] 양손 grab 가능 (Phase 2 는 single-hand 우선, two-hand 는 향후)
- [ ] Phase 1 leftover: Confirm button poke 로 calibration 통과 (keyboard Space 더 이상 동작 안 함)

---

## 8. Out of Scope (Phase 3+ 또는 별도)

- **Manual-Full mode** (Phase 3) — 손 놓으면 멈춤 또는 중력 닫힘
- **Power-Auto mode** (Phase 3) — 차량의 Power button 1 회 누름 → 끝까지 자동 개방
- **Power-Hybrid mode** (Phase 3) — Power button → 일정 각도 → 이후 hand grab
- **Mode toggle UI** (Phase 3) — World-space widget (ADR-008 이후)
- **Zone visualizer** (Phase 3) — Red/Green/Yellow 영역
- **Height ruler** (Phase 3) — 차량 옆 키 표시
- **Close (closing) animation** (Phase 3 이후) — 현재는 한 번 열면 끝
- **Gravity / damping** for partial-open release (Manual-Full 도입 시 검토)
- **Two-hand grab** (Phase 2 는 single-hand)
- **Real CAD liftgate mesh** (Phase 4 — Datasmith)
- **Liftgate weight / inertia feedback** (햅틱 — out of scope per CLAUDE.md §10)

---

## 9. 확정된 Decision

| 항목 | 확정 |
|---|---|
| Mode | Manual-Assist 만 (Phase 2) |
| Grab | ISDK 기반 (`IsdkGrabbableComponent`) |
| Hinge axis | 차량 Y 축 (좌우), liftgate 가 위로 열림 |
| Rotation mapping | 손 World Z delta × `GrabSensitivity_deg_per_cm` (선형, simple) |
| Threshold | 40° (release 시 auto-complete 분기) |
| MaxAngle | 85° |
| Auto-open duration | 1.2 초 (lerp / linear) |
| 닫는 동작 | 없음 (Phase 2) |
| 중력 | 없음 (Phase 2) |
| Two-hand grab | 안 함 (Phase 2 single-hand 만) |
| Power button | Phase 3 |

---

## 10. 작업 순서 (권장)

1. **W2.0** — Phase 1 leftover (HandleConfirm rename + Confirm button) — 작은 cleanup 먼저
2. **W2.1** — C++ ALiftgate + types — base 컴파일
3. **W2.2** — BP_Liftgate (BP child + ISDK Grab wiring) — 가장 위험 영역 (ISDK API 정확성)
4. **W2.3** — Grab → rotation 동작 검증 (Manual-Full 효과적으로 작동, 일단 단순 멈춤)
5. **W2.4** — Manual-Assist 40° auto-complete
6. **W2.5** — L_Vehicle_Dummy 배치 + 자세 / 위치 튜닝
7. **W2.6** — VR Preview 전체 검증

각 W 완료 시 commit (`phase2: W2.x <action>`). C++ 변경은 별도 commit.

---

## 11. 막힐 시 디버그 우선순위

| 증상 | 1차 확인 |
|---|---|
| ISDK Grab event 가 fire 안 됨 | IsdkGrabbableComponent 가 hand interactor 와 충돌 가능 collision 셋업, ISDK Sample 의 BP_GrabbableObject 와 비교 |
| Grab 됐는데 rotation 안 됨 | BeginGrab → UpdateGrab → ALiftgate 의 함수 호출 확인 (print). HingePivot 의 RelativeRotation 변경 확인 |
| Rotation 이 너무 빠름 / 느림 | `GrabSensitivity_deg_per_cm` 조정. 시작 1.5 |
| 40° 에서 auto-complete 안 됨 | `EndGrab()` 에서 `CurrentMode == ManualAssist` 조건 / threshold 비교 확인 |
| Auto-complete 가 너무 빠름 / 느림 | `AutoOpenDuration_sec` 조정 |
| Hinge axis 가 어긋남 | HingePivot 의 RelativeRotation 설정. liftgate mesh 의 pivot point 가 회전 중심이 되도록 mesh import 시 origin 조정 |
| Liftgate 가 차량 박스 안에 들어감 | mesh thickness / location 조정 |
| 두 손 동시 grab 시 이상 동작 | Phase 2 는 single-hand. `IsdkGrabbableComponent` 의 chirality 를 Left 또는 Right 만 |

해결 안 되는 이슈는 `Docs/lessons_learned/<topic>.md` 로 박제.

---

## 12. Phase 2 완료 후 회고

Phase 3 진입 전 다음 항목 회고:

1. ISDK Grab API 셋업에서 가장 시간 든 부분 (Phase 3 의 Power button 도 ISDK input)
2. Rotation 매핑 (`deg_per_cm`) 의 직관성 — 평가자 첫 시도 시 어색함 있는지
3. Manual-Assist 의 40° threshold — 실차와 비교 시 자연스러운지
4. Phase 3 진입 전 보완 필요 사항 (mode toggle UI 위치, Power button placement 등)
5. CLAUDE.md / Phase 2 spec 수정 필요 항목

회고 결과는 `Docs/phase_kickoff/Phase3_Kickoff.md` 작성 시 반영.

---

## 13. Claude Code 작업 원칙

Phase 1 과 동일:
- 사용자에게 결정 요구 시 **선택지 명확히** 제시
- 본 문서에 명시된 확정 사항은 **재확인 없이** 진행
- 본 문서에 없는 결정 사항은 **반드시 사용자 확인** 후 진행
- 작업 중 발견된 함정 / 학습 / 우회는 `Docs/lessons_learned/` 에 즉시 박제
- 모든 commit 메시지는 영어, format: `phase2: <work_item> <action>`
- 한 work item 완료 시 사용자에게 검증 요청
