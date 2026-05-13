# Liftgate Study

GMTCK PQDQ — VR-based liftgate interaction evaluation tool.

## Purpose

주관적 평가 도구. 다양한 차량의 liftgate 개폐 동작을 4가지 mode (Manual-Full / Manual-Assist / Power-Auto / Power-Hybrid) 로 VR에서 체험.

평가 데이터 로깅 없음. 평가자의 직접 체험이 목적.

## Requirements

- Unreal Engine **5.5.4**
- Meta XR Plugin **v78**
- Meta XR Interaction SDK **v78**
- Meta Quest 3 + Quest Link (PCVR)
- Visual Studio 2022 with `Game development with C++` workload (plugin compilation, optional)

## Setup

1. Clone this repo to `C:\Git\LiftgateStudy`
2. Download Meta XR Plugin **v78** and Meta XR Interaction SDK **v78** from Meta Quest Developer Hub
3. Unzip both to `Plugins/` folder (project-local, not engine-wide)
4. Open `LiftGateStudy.uproject`
5. If prompted to compile modules, click **Yes**
6. Edit → Plugins → confirm Meta XR / Meta XR Interaction SDK / OpenXR / OpenXRHandTracking are enabled
7. Project Settings → Plugins → Meta XR → Hand Tracking Support = **Controllers and Hands**
8. Restart Editor

See `Docs/Project_Setup_Prompt.md` for full setup details (Step A + Step B).

## Documentation

- `CLAUDE.md` — Project governance rules (Critical Rules R1~R7, naming, phase plan)
- `Docs/Project_Setup_Prompt.md` — Initial project setup (Path B)
- `Docs/phase_kickoff/` — Phase plans (Phase 1~4)
- `Docs/decisions/` — Architecture Decision Records (ADRs)
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
