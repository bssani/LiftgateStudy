# Phase 1 — Retrospective

> Phase 1 (Foundation) 진행 중 발견한 함정 / 학습 / 우회. 차후 phase 의 의사결정 자료.

---

## L1. Case-sensitive fs (Linux) vs Case-insensitive fs (Windows) 충돌

**증상**: PR #5 (`LiftgateStudy` lowercase 모듈) + 사용자 UE Editor 마법사 (`LiftGateStudy` capital G 모듈) 가 같이 main 에 들어가면서 Linux 에서는 폴더 두 개 / Windows 에서는 한 폴더가 보임. PR pull 시 git 이 lowercase 파일 삭제를 시도 → Windows 가 실제 capital G 파일을 삭제함.

**원인**: case-only rename (예: `Source/LiftgateStudy/` → `Source/LiftGateStudy/`) 은 git history 상에서는 다른 path 지만 Windows fs 에서는 같은 path. Pull 시 git 의 delete 가 Windows 에서 실제 파일을 날림.

**우회 / 예방**:
- 모듈 이름은 `.uproject` 파일명과 일치시킨다 (UE Editor 마법사가 만드는 default 따름)
- C++ scaffolding 을 Linux 에서 미리 만들 때 ADR 단계에서 module 이름 합의 후 진행
- Case-only rename 이 필요하면 **임시 이름** 으로 한 번 거쳐서 두 단계로 (예: `LiftgateStudy` → `LiftgateStudyTemp` → `LiftGateStudy`)

**관련 PR**: #5, #6
**관련 ADR**: ADR-005 (C++ adoption — 모듈 이름 결정)

---

## L2. ISDK Pinch 기반 confirm 의 한계

**증상**: Phase 1 W1.4 spec 은 "Pinch 로 calibration 확인" 이었으나, ISDK Pinch detection 의 wiring 이 brittle. 자연스러운 손 동작 중 false positive 잦음. 평가자마다 pinch 정도 차이.

**우회**: ADR-007 채택 — **모든 confirmation 은 button poke**. Gesture / pose detection 금지.

**관련 ADR**: ADR-007
**관련 R**: R8

**Side effect**: `ACalibrationGateActor::HandlePinchInput()` UFUNCTION 이름이 misleading 으로 남음 (이제는 button poke 가 호출). Phase 2 W2.0 에서 `HandleConfirm()` 으로 rename.

---

## L3. Wrist-attached UI 의 fragility

**증상**: Phase 1 W1.5 의 `WBP_WristPanel` 가 visible 하게 만드는데 ISDK hand mesh 의 up vector 축 매핑 / attach socket / one-sided material orientation 등 여러 함정. 평가자마다 dot threshold 튜닝 다름.

**우회**: ADR-008 채택 — **WristPanel 폐기, Primary UI = World-space widget**. 향후 mode toggle / vehicle swap UI 도 World-space 기준.

**관련 ADR**: ADR-008
**관련 R**: R6 갱신, §8 UI Conventions

**Side effect**: Phase 3 의 mode toggle UI 위치 결정 ADR 필요할 가능성 (예: 평가자 좌측 World-space panel).

---

## L4. UE Editor 마법사 flat-layout vs Public/Private split

**증상**: UE Editor → New C++ Class 마법사가 `Source/LiftGateStudy/LiftgateStudyGameMode.h/.cpp` (flat layout, 모듈 root 에 직접) 로 생성. 내 scaffolding 은 `Source/LiftGateStudy/Public/...`, `Private/...` split. 두 layout 이 main 에 공존 → 중복.

**우회**: PR #6 cleanup 에서 flat-layout 파일 삭제 + Public/Private 으로 통일.

**예방**: 마법사가 만드는 file 위치를 wizard 단계에서 "Public" 옵션으로 강제. 또는 마법사로 생성 후 즉시 Public/ 로 git mv.

**관련 PR**: #6

---

## L5. PR 머지 순서 — C++ 삭제 전 BP cleanup 선행

**증상**: ADR-008 wristpanel 제거 시 C++ 클래스 (`UWristPanelWidget`) 를 먼저 삭제하고 pull 하면 BP_WristPanel.uasset 이 parent class 못 찾아 load 실패. BP_VRPawn 의 reference 도 broken.

**우회**: ADR-008 §Implementation Order 명시 — Windows 쪽 BP / asset cleanup **선** 진행, 그 다음 PR 머지.

**예방**: 향후 C++ 클래스 삭제 PR 마다 "선행 cleanup 절차" 를 ADR / PR description 에 명시.

**관련 PR**: #8

---

## L6. UMG Binding — enum 직접 노출 불가

**증상**: WBP_CalibrationCheck 에서 `ECalibrationStatus` UPROPERTY 가 TextBlock 의 Bind dropdown 에 안 나옴.

**원인**: UMG Bind 는 same-type 또는 자동 변환 가능한 타입 (float, int, bool, FText) 만 직접 노출. Enum 은 FText 변환 함수 필요.

**우회**: 
- BP function binding 으로 enum → FText 변환 (Get Display Name (Enum))
- 또는 C++ 에서 `UFUNCTION GetStatusText() : FText` helper 제공

**향후**: Phase 2 이후 enum 노출이 흔하면 helper 패턴을 CLAUDE.md §6 에 추가 검토.

---

## L7. IsdkSamplePawn 활용으로 W1.2 절감

**증상 (긍정)**: ADR-004 채택 후 `BP_VRPawn` 의 hand rig migrate 단계 통째로 제거. Phase 1 진행 속도 ↑.

**학습**: Phase 시작 전 plugin 의 sample content 점검이 가치 큼. 향후 Phase 2 의 grab interaction 도 ISDK sample 의 `BP_GrabbableObject` 등 참조.

**관련 ADR**: ADR-004

---

## L8. Plugin Content (`.uasset`) 수정 위험

**관찰**: main 의 commit `75711fe` 에서 `Plugins/OculusInteractionSamples/Content/Blueprints/IsdkSamplePawn.uasset` 가 78KB → 84KB 로 변경됨. 의도된 변경이 아니면 plugin upgrade 시 충돌 가능.

**예방**: Plugin Content (`Plugins/*/Content/*`) 는 수정하지 않는다. 필요 시 본 프로젝트의 `Content/` 로 Migrate 한 후 수정.

**향후**: CLAUDE.md §6 또는 R 신규 항목으로 명시 검토.

---

## Phase 2 진입 전 보완 항목

- [ ] `ACalibrationGateActor::HandlePinchInput()` → `HandleConfirm()` rename (W2.0)
- [ ] BP_CalibrationGate Event Graph 의 Keyboard Space 임시 wiring → Confirm 버튼 OnClicked 로 교체 (W2.0)
- [ ] WBP_CalibrationCheck 에 `Button_Confirm` 추가 (W2.0)
- [ ] (필요 시) CLAUDE.md 에 plugin content 수정 금지 R 추가 검토
