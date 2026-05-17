# Phase 2 — Work Item Checklist

> Phase2_Kickoff.md 의 W2.0~W2.6 을 UE Editor 에서 따라갈 수 있는 체크리스트.
> 모든 항목은 CLAUDE.md (R1~R8, Naming, Coding Style) 준수.
> 작업 순서: W2.0 → W2.1 → W2.2 → W2.3 → W2.4 → W2.5 → W2.6.

---

## Pre-flight

- [ ] `git pull origin main`
- [ ] Phase 1 §7 검증 통과 (calibration 동작 OK)
- [ ] C++ 모듈 `LiftGateStudy` VS 2022 컴파일 OK
- [ ] ISDK Sample 의 grab 예제 (`BP_GrabbableObject` 등) 열어서 IsdkGrabbableComponent 사용 패턴 확인 — 자체 작성 금지 (R5)

---

## W2.0 — Phase 1 Leftover Cleanup

### W2.0.A — C++ rename `HandlePinchInput` → `HandleConfirm` ✅ (Phase 2 kickoff PR 에 포함)

- C++ rename 은 이번 PR (Phase 2 kickoff) 에 이미 반영. 사용자는 별도 작업 없음
- **단**: PR 머지 후 `BP_CalibrationGate` 의 Event Graph 에 `HandlePinchInput` 호출 노드가 **broken** 으로 표시됨 — W2.0.C 에서 `HandleConfirm` 으로 교체
- VS 2022 빌드 시 컴파일 통과 확인만 하면 됨

### W2.0.B — WBP_CalibrationCheck 에 Confirm Button 추가

- [ ] WBP_CalibrationCheck 열기
- [ ] Designer 의 Vertical Box 하단에 **Button** widget 추가, 이름 `Button_Confirm`
- [ ] Button 자식으로 TextBlock → "Press Confirm"
- [ ] Button_Confirm 의 **Visibility** 속성 → Bind → Create Binding:
  - Function: `Status == ECalibrationStatus::Pass` → return `Visible`, else `Collapsed`
- [ ] Designer 우상단 Compile + Save

### W2.0.C — BP_CalibrationGate Event Graph 갱신

- [ ] BP_CalibrationGate 열기
- [ ] Event Graph 의 기존 Space 키 → HandlePinchInput wiring 제거 (Phase 1 임시)
- [ ] BeginPlay 에서 widget cache 후 다음 추가:
  ```
  Cached Widget → Get Button_Confirm (변수) → On Clicked Event → 
      Bind to Custom Event "HandleConfirmClicked"

  HandleConfirmClicked (custom event):
      Self → HandleConfirm
  ```
- [ ] 또는 widget designer 에서 Button_Confirm 의 OnClicked 에서 직접 GameMode 또는 gate 의 HandleConfirm 호출
- [ ] 컴파일 OK

### W2.0.D — 검증

- [ ] VR Preview → Calibration widget 표시
- [ ] Status = Fail 일 때 Confirm button 안 보임
- [ ] Status = Pass 가 되면 Confirm button visible
- [ ] Confirm button 을 손가락으로 poke → widget 사라짐 (gate Destroy)
- [ ] 평가자 정면 위치에서 button 이 닿는 거리 (R8 / ADR-007)
- [ ] Keyboard Space 는 더 이상 동작하지 않음 (제거됨)

---

## W2.1 — ALiftgate C++ Base

### W2.1.A — LiftgateTypes.h 작성

- [ ] `Source/LiftGateStudy/Public/Liftgate/LiftgateTypes.h` 새 파일:
  ```cpp
  #pragma once
  #include "CoreMinimal.h"
  #include "LiftgateTypes.generated.h"

  UENUM(BlueprintType)
  enum class ELiftgateMode : uint8
  {
      ManualFull   UMETA(DisplayName = "Manual-Full"),
      ManualAssist UMETA(DisplayName = "Manual-Assist"),
      PowerAuto    UMETA(DisplayName = "Power-Auto"),
      PowerHybrid  UMETA(DisplayName = "Power-Hybrid")
  };

  UENUM(BlueprintType)
  enum class ELiftgateState : uint8
  {
      Closed       UMETA(DisplayName = "Closed"),
      Grabbed      UMETA(DisplayName = "Grabbed"),
      AutoOpening  UMETA(DisplayName = "AutoOpening"),
      Open         UMETA(DisplayName = "Open")
  };
  ```

### W2.1.B — ALiftgate.h 작성

- [ ] `Source/LiftGateStudy/Public/Liftgate/Liftgate.h`:
  ```cpp
  #pragma once
  #include "CoreMinimal.h"
  #include "GameFramework/Actor.h"
  #include "Liftgate/LiftgateTypes.h"
  #include "Liftgate.generated.h"

  class UStaticMeshComponent;

  UCLASS(Blueprintable)
  class LIFTGATESTUDY_API ALiftgate : public AActor
  {
      GENERATED_BODY()
  public:
      ALiftgate();

      // Components
      UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Liftgate")
      USceneComponent* HingePivot;

      UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Liftgate")
      UStaticMeshComponent* LiftgateMesh;

      // Mode
      UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mode")
      ELiftgateMode CurrentMode = ELiftgateMode::ManualAssist;

      // Tunable
      UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Liftgate")
      float MaxAngle_deg = 85.f;

      UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Liftgate")
      float ThresholdAngle_deg = 40.f;

      UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Liftgate")
      float AutoOpenDuration_sec = 1.2f;

      UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Liftgate")
      float GrabSensitivity_deg_per_cm = 1.5f;

      // Runtime
      UPROPERTY(BlueprintReadOnly, Category="Liftgate")
      float CurrentAngle_deg = 0.f;

      UPROPERTY(BlueprintReadOnly, Category="Liftgate")
      ELiftgateState State = ELiftgateState::Closed;

      // API for BP grab wiring
      UFUNCTION(BlueprintCallable, Category="Liftgate")
      void BeginGrab(const FVector& HandWorldLocation);

      UFUNCTION(BlueprintCallable, Category="Liftgate")
      void UpdateGrab(const FVector& HandWorldLocation);

      UFUNCTION(BlueprintCallable, Category="Liftgate")
      void EndGrab();

  protected:
      virtual void Tick(float DeltaTime) override;

      // Cache during grab
      float GrabStartAngle_deg = 0.f;
      FVector GrabStartHandLocation = FVector::ZeroVector;

      // Cache during auto-opening
      float AutoOpenElapsed_sec = 0.f;
      float AutoOpenStartAngle_deg = 0.f;

      void SetAngle(float NewAngle_deg);
  };
  ```

### W2.1.C — ALiftgate.cpp 구현

- [ ] `Source/LiftGateStudy/Private/Liftgate/Liftgate.cpp`:
  ```cpp
  #include "Liftgate/Liftgate.h"
  #include "Components/StaticMeshComponent.h"

  namespace { constexpr float kCmToMm = 10.f; }

  ALiftgate::ALiftgate()
  {
      PrimaryActorTick.bCanEverTick = true;

      HingePivot = CreateDefaultSubobject<USceneComponent>(TEXT("HingePivot"));
      RootComponent = HingePivot;

      LiftgateMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LiftgateMesh"));
      LiftgateMesh->SetupAttachment(HingePivot);
  }

  void ALiftgate::BeginGrab(const FVector& HandWorldLocation)
  {
      // 이미 자동 개방 중이거나 완전 개방이면 무시
      if (State == ELiftgateState::AutoOpening || State == ELiftgateState::Open)
      {
          return;
      }
      State = ELiftgateState::Grabbed;
      GrabStartAngle_deg = CurrentAngle_deg;
      GrabStartHandLocation = HandWorldLocation;
  }

  void ALiftgate::UpdateGrab(const FVector& HandWorldLocation)
  {
      if (State != ELiftgateState::Grabbed) return;

      // UE 의 cm 단위 vertical delta
      const float DeltaZ_cm = HandWorldLocation.Z - GrabStartHandLocation.Z;
      const float DeltaAngle = DeltaZ_cm * GrabSensitivity_deg_per_cm;
      const float NewAngle = FMath::Clamp(
          GrabStartAngle_deg + DeltaAngle, 0.f, MaxAngle_deg);

      SetAngle(NewAngle);
  }

  void ALiftgate::EndGrab()
  {
      if (State != ELiftgateState::Grabbed) return;

      // Manual-Assist + threshold 이상 → 자동 완전 개방
      if (CurrentMode == ELiftgateMode::ManualAssist
          && CurrentAngle_deg >= ThresholdAngle_deg)
      {
          State = ELiftgateState::AutoOpening;
          AutoOpenElapsed_sec = 0.f;
          AutoOpenStartAngle_deg = CurrentAngle_deg;
      }
      else
      {
          // 그 자리 멈춤 (Phase 2 는 중력 없음)
          State = ELiftgateState::Closed;  // semantic: "not actively grabbed"
      }
  }

  void ALiftgate::Tick(float DeltaTime)
  {
      Super::Tick(DeltaTime);
      if (State != ELiftgateState::AutoOpening) return;

      AutoOpenElapsed_sec += DeltaTime;
      const float Alpha = FMath::Clamp(
          AutoOpenElapsed_sec / AutoOpenDuration_sec, 0.f, 1.f);
      const float NewAngle = FMath::Lerp(
          AutoOpenStartAngle_deg, MaxAngle_deg, Alpha);
      SetAngle(NewAngle);

      if (Alpha >= 1.f)
      {
          State = ELiftgateState::Open;
      }
  }

  void ALiftgate::SetAngle(float NewAngle_deg)
  {
      CurrentAngle_deg = NewAngle_deg;
      // HingePivot 의 local Y 축 회전 (좌우 hinge, 위로 열림)
      // Phase 2 시작값: Y 회전. 차량 hinge 방향에 따라 조정
      HingePivot->SetRelativeRotation(
          FRotator(/*Pitch=*/ -CurrentAngle_deg, 0.f, 0.f));
  }
  ```

  주: Pitch 음수 = 위로 회전 (UE 의 Pitch 방향 컨벤션). 차량 hinge 축에 따라 부호 / 축 조정.

- [ ] VS 2022 컴파일 OK

### W2.1.D — Build.cs 점검

- [ ] `Source/LiftGateStudy/LiftGateStudy.Build.cs` 가 `Engine`, `Core`, `CoreUObject`, `UMG` 포함 — Phase 1 에서 이미 셋업. Liftgate 단독으로는 추가 dep 불필요.
- [ ] ISDK API 를 C++ 에서 직접 호출 시 (W2.2 진행 중 발견되면) `MetaXRInteraction` 등 추가

---

## W2.2 — BP_Liftgate (ISDK Grab Wiring)

### W2.2.A — BP_Liftgate 생성

- [ ] Content/Blueprints/ → Blueprint Class → All Classes → `Liftgate` 선택
- [ ] 이름: `BP_Liftgate`

### W2.2.B — Mesh 지정

- [ ] BP_Liftgate 열기 → Components 패널 → `LiftgateMesh` 선택
- [ ] Static Mesh 속성:
  - **(MVP)** `Engine/BasicShapes/Cube` 선택
  - Scale: `(0.8, 1.4, 0.08)` — 80cm × 140cm × 8cm placeholder
  - Material: 단색 (회색 또는 차량 색)
- [ ] LiftgateMesh 의 RelativeLocation 조정:
  - Z = -4 (mesh 두께의 절반) 정도로 mesh 가 HingePivot 의 위쪽에 자리하도록
  - X, Y 는 0 — hinge 가 mesh 의 한쪽 끝에 오도록 조정 가능

### W2.2.C — IsdkGrabbableComponent 추가

- [ ] Components 패널 → Add Component → 검색 `IsdkGrabbable...`
- [ ] **Isdk Grabbable Component** (정확한 이름은 ISDK v78 따라 다를 수 있음) 추가
- [ ] LiftgateMesh 의 자식으로 (또는 HingePivot 자식)
- [ ] Details 에서 chirality / grab volume 설정 — ISDK Sample 의 BP_GrabbableObject 참조

### W2.2.D — Grab Event Wiring (Event Graph)

- [ ] BP_Liftgate 의 Event Graph 에서 IsdkGrabbableComponent 의 event 들 노출:
  - `OnGrabBegin` (또는 v78 의 동등 event, ISDK Sample 의 grab 예제와 동일 이름)
  - `OnGrabUpdate` (또는 Tick during grab)
  - `OnGrabEnd`
- [ ] OnGrabBegin → 손의 World location 가져오기 → Self 의 `BeginGrab(handLoc)` 호출
- [ ] OnGrabUpdate (또는 Event Tick 중 grab 상태 확인) → `UpdateGrab(handLoc)`
- [ ] OnGrabEnd → `EndGrab()`

**손 World location 가져오는 방법** (ISDK API 따라 다름):
- ISDK event payload 에 InteractorActor / Hand 가 있을 가능성 → 그 location 사용
- 또는 BP_VRPawn 의 LeftHandMotionController / RightHandMotionController 의 World location 둘 중 가까운 것
- 가장 정확: ISDK event 의 grabbing-hand reference 사용 (Sample 참조)

만약 v78 ISDK event payload 가 `Pose` / `HandRig` 등의 reference 를 주면 거기서 World location 추출. 정확한 API 는 W2.2 진행 중 sample 확인.

### W2.2.E — 검증

- [ ] BP_Liftgate 컴파일 OK
- [ ] Editor 의 Place Actor 메뉴에서 BP_Liftgate 보임
- [ ] L_Vehicle_Dummy 또는 L_Main 에 임시 배치
- [ ] VR Preview → grab interactor 가 mesh 와 hit 되는 debug 표시 (ISDK debug)

---

## W2.3 — Grab → Rotation 검증

- [ ] VR Preview → liftgate 위치에 손 가져가 grab (ISDK pinch 또는 grab gesture)
- [ ] 손 위로 들어올림 → liftgate 회전 발생 확인
- [ ] 회전 방향이 자연스러운지 (위로 열림). 어긋나면 `SetAngle()` 의 Rotator 축 조정
- [ ] Print String 으로 `CurrentAngle_deg` 출력 → 손 움직임 따라 변화 확인
- [ ] 너무 둔감 / 예민하면 `GrabSensitivity_deg_per_cm` (BP_Liftgate Class Defaults) 조정

---

## W2.4 — Manual-Assist Auto-Complete 검증

- [ ] 40° 미만에서 손 놓기 → 그 자리 멈춤 확인
- [ ] 40° 이상에서 손 놓기 → 1.2 초 동안 smooth lerp 로 85° 까지 자동 개방
- [ ] `ThresholdAngle_deg`, `AutoOpenDuration_sec` 를 BP_Liftgate Class Defaults 에서 조정 → 즉시 반영 확인

---

## W2.5 — L_Vehicle_Dummy 배치

- [ ] `L_Vehicle_Dummy` 열기
- [ ] 기존 marker mesh (Phase 1 W1.6.4 의 handle 예상점) 삭제 또는 hide
- [ ] BP_Liftgate 배치:
  - [ ] Location: 차량 박스의 **후면 상단**. 예: 박스가 X=−1.5m, 박스 크기 (4.5, 1.8, 1.6) m 라면 BP_Liftgate 의 hinge 가 차량 후면 (X 음수 끝) 의 상단 (Z ≈ 1.6m) 에 오도록
  - [ ] Rotation: hinge axis (HingePivot 의 local 회전축) 가 차량 좌우 (Y 축) 와 일치하도록. 일반적으로 actor rotation 은 (0, 0, 0) 이고 SetAngle 의 Pitch 가 위쪽 회전을 만들도록
  - [ ] BP_Liftgate 의 LiftgateMesh RelativeLocation / Scale 로 차량 후면 크기에 맞게 fit
- [ ] 평가자 (PlayerStart) 위치에서 차량 뒤로 돌아가 liftgate 앞에 섰을 때 handle 영역이 어깨~허리 높이 + 30~60cm 거리 (§8 R8 proximity)

---

## W2.6 — VR Preview 전체 검증

Phase2_Kickoff.md §7 verification 의 모든 항목 통과 확인.

- [ ] Calibration → Confirm button poke → 평가 진입
- [ ] 차량 후면으로 이동 → liftgate visible
- [ ] Liftgate grab → 위로 들기 → 회전 smooth
- [ ] 40° 미만 release → 멈춤
- [ ] 40° 이상 release → 1.2 초 자동 완전 개방
- [ ] 모든 UPROPERTY 가 Class Defaults 에서 조정 가능
- [ ] C++ 모듈 컴파일 OK, Live Coding 또는 Editor restart 정상

---

## Phase 2 작업 중 막힐 때

| 증상 | 1차 확인 |
|---|---|
| ISDK Grab event 가 fire 안 됨 | IsdkGrabbableComponent collision setup, Sample 의 BP_GrabbableObject 와 component 트리 비교 |
| BeginGrab 은 되는데 UpdateGrab 안 호출 | ISDK 가 OnGrabUpdate 를 별도 event 로 제공 안 할 수 있음 → Event Tick + "is grabbed" flag 패턴으로 변경. ISDK Sample 참조 |
| Grab 시 양손 모두 잡혀 짖궂은 동작 | `IsdkGrabbableComponent` 의 chirality 를 Left 또는 Right 만, 또는 single-hand 모드 |
| Liftgate 회전 방향 반대 | `SetAngle` 의 Rotator 의 Pitch 부호 또는 회전 축 조정 |
| Rotation Pivot 이 mesh 중앙에서 일어남 | LiftgateMesh 의 RelativeLocation 으로 mesh 를 한 쪽으로 offset → HingePivot 이 mesh 의 한쪽 끝 (hinge 위치) 에 자리 |
| 자동 개방이 끝나도 손 놓은 자리에서 다시 시작됨 | `State` 가 Open 으로 잘 전환되는지 / Tick 에서 AutoOpening 끝 확인 |
| BP child 의 ALiftgate parent 가 안 보임 | C++ 컴파일 OK 인지, ALiftgate 가 `Blueprintable` UCLASS 인지 |
| C++ Tick 이 호출 안 됨 | Constructor 의 `PrimaryActorTick.bCanEverTick = true;` 확인 |

해결 안 되는 이슈는 `Docs/lessons_learned/<topic>.md` 에 박제.

---

## Commit 규칙

Phase 1 과 동일:
- Work item 완료 시 commit
- C++ / BP 변경은 가능하면 별도 commit
- Format: `phase2: <work_item> <action>`
  - 예: `phase2: W2.1 add ALiftgate skeleton`
  - 예: `phase2: W2.2 wire IsdkGrabbableComponent in BP_Liftgate`
  - 예: `phase2: W2.4 implement Manual-Assist auto-complete lerp`
- 영어 commit message
