# Liftgate Study

GMTCK PQDQ — VR-based liftgate interaction evaluation tool.

## Purpose

주관적 평가 도구. 다양한 차량의 liftgate 개폐 동작을 4가지 mode (Manual-Full / Manual-Assist / Power-Auto / Power-Hybrid) 로 VR에서 체험.

평가 데이터 로깅 없음. 평가자의 직접 체험이 목적.

## Requirements

- Unreal Engine **5.5.4**
- Meta XR Plugin **v78**
- Meta XR Interaction SDK **v78**
- OculusInteractionSamples plugin (ADR-004 — `IsdkSamplePawn` parent)
- Meta Quest 3 + Quest Link (PCVR)
- **Visual Studio 2022** with `Game development with C++` workload — **required** (ADR-005: C++ 로직 + BP layout hybrid)

## Setup

1. Install Visual Studio 2022 with **Game development with C++** workload (ADR-005)
2. Clone this repo to `C:\Git\LiftgateStudy`
3. Download Meta XR Plugin **v78** and Meta XR Interaction SDK **v78** from Meta Quest Developer Hub
4. Unzip both to `Plugins/` folder (project-local, not engine-wide)
5. Open `LiftGateStudy.uproject`
6. If prompted to compile modules, click **Yes** (VS 2022 가 sln 생성 후 컴파일)
7. Edit → Plugins → confirm Meta XR / Meta XR Interaction SDK / OculusInteractionSamples / OpenXR / OpenXRHandTracking are enabled
8. Project Settings → Plugins → Meta XR → Hand Tracking Support = **Controllers and Hands**
9. Restart Editor

See `Docs/Project_Setup_Prompt.md` for full setup details (Step A + Step B).

### C++ Module

본 프로젝트는 **C++ 로직 + BP 레이아웃 hybrid** 패턴을 사용 (ADR-005). 새 로직은 `Source/LiftGateStudy/` 에 C++ class 로 작성하고, BP child 가 layout / Designer-tunable default 를 담당함. 자세한 내용은 `Docs/decisions/ADR_005_cpp_adoption.md` 참조.

## Documentation

- `CLAUDE.md` — Project governance rules (Critical Rules R1~R7, naming, phase plan). 현재 **v0.3**
- `Docs/Project_Setup_Prompt.md` — Initial project setup (Path B)
- `Docs/phase_kickoff/` — Phase plans (Phase1_Kickoff.md, Phase1_Checklist.md)
- `Docs/decisions/` — Architecture Decision Records (ADR-001 ~ ADR-005)
- `Docs/lessons_learned/` — Discovered pitfalls, workarounds

## Current Phase

**Phase 1 — Foundation** (in progress)

See `Docs/phase_kickoff/Phase1_Kickoff.md` for current work plan.

## ISDK Sample Reference

The Meta ISDK Sample (`5.5.4-v78` branch) is used as reference for hand rig BP migration.

Not included in this repo. Clone separately to a sibling folder, e.g. `C:\Reference\Unreal-InteractionSDK-Sample\`:

```bash
git clone -b 5.5.4-v78 https://github.com/oculus-samples/Unreal-InteractionSDK-Sample.git
```

## License / Ownership

GMTCK PQDQ internal evaluation tool. Not for external distribution.
