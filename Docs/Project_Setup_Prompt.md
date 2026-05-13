# Project Setup — Liftgate Study

> 본 문서는 LiftgateStudy 프로젝트의 **초기 셋업**을 위한 작업 명세다.
>
> **Path B (깨끗한 새 프로젝트 + Sample은 참조용)** 기준으로 작성됨.
> Path A (Sample을 base로 사용)로 가려면 별도 안내 필요.

---

## Two-Step 구조

이 셋업은 두 단계로 진행:

| Step | 누가 | 무엇을 |
|---|---|---|
| **Step A** | 사용자 직접 | UE 프로젝트 생성, Plugin 설치, Project Settings 적용 |
| **Step B** | Claude Code | Git 초기화, 폴더 구조, 문서 배치, ADR 작성, 첫 commit |

**Step A 완료 후 Step B 진입**. 순서 바꾸지 말 것.

---

## Step A — 사용자 직접 작업

### A.1 — UE 5.5.4 프로젝트 생성

1. Epic Games Launcher → UE 5.5.4 실행
2. New Project → Games → **Blank** 선택
3. Project Type: **Blueprint**
4. Project Name: `LiftgateStudy`
5. Location: `C:\Git\` (Git repo 부모 폴더)
6. Starter Content: ❌ 해제
7. Raytracing: 기본값
8. Target Platform: Desktop

**결과**: `C:\Git\LiftgateStudy\LiftgateStudy.uproject` 생성됨.

---

### A.2 — Meta XR Plugins 설치 (v78)

#### A.2.1 — 다운로드

Meta Quest Developer Hub에서 다음 다운로드:
- **Meta XR Plugin v78**
- **Meta XR Interaction SDK Plugin v78**

⚠️ 두 plugin의 버전은 반드시 **같은 v78**. v77 + v78 같은 cross 조합 금지.

#### A.2.2 — 설치

1. `C:\Git\LiftgateStudy\` 에 `Plugins\` 폴더 생성
2. 두 zip 파일을 `Plugins\` 에 압축 해제

결과 구조:
```
LiftgateStudy\
├── LiftgateStudy.uproject
└── Plugins\
    ├── MetaXR\
    │   └── MetaXR.uplugin
    └── MetaXRInteraction\
        └── MetaXRInteraction.uplugin
```

#### A.2.3 — 컴파일

1. `.uproject` 실행
2. "Module is missing or compiled with different engine version" prompt 발생 가능 → **Yes** 선택하여 자동 컴파일
3. ⚠️ 자동 컴파일 실패 시 Visual Studio 2022 설치 필요 (`Game development with C++` workload)

---

### A.3 — ISDK Sample Clone (참조용, 별도 위치)

본인 프로젝트와 **분리**된 위치에 clone:

```bash
# 별도 reference 폴더에
mkdir C:\Reference
cd C:\Reference
git clone -b 5.5.4-v78 https://github.com/oculus-samples/Unreal-InteractionSDK-Sample.git
```

**용도**: Hand rig BP, ISDK 예제 패턴 참조 자료. **본인 프로젝트에 merge하지 않음.**

Phase 1 W1.2에서 Sample의 Hand Rig를 Migrate로 가져올 예정.

---

### A.4 — Plugin 활성화 확인

UE Editor에서:

1. Edit → Plugins
2. 다음 plugin 활성화 확인 (또는 활성화):
   - ✅ **Meta XR**
   - ✅ **Meta XR Interaction SDK**
   - ✅ **OpenXR** (UE 내장)
   - ✅ **OpenXRHandTracking** (UE 내장)
3. **Restart Editor** (활성화 변경 시 필수)

---

### A.5 — Project Settings 적용

Edit → Project Settings:

1. **Plugins → Meta XR**:
   - Hand Tracking Support: **`Controllers and Hands`**
   - (선택) Launch Meta XR Project Setup Tool → 권장 설정 자동 적용

2. **Engine → Rendering**:
   - Default RHI: **DirectX 12** (Quest 호환 우선) 또는 Vulkan

3. **Maps & Modes**:
   - 추후 Phase 1 W1.1에서 설정 (지금은 건드리지 않음)

4. **Restart Editor**

---

### A.6 — Quest 3 + PCVR 연결 확인

1. Quest 3 헤드셋 켜기
2. PC에 Meta Quest Link 앱 설치 (없으면 다운로드)
3. USB-C 케이블 연결 또는 Air Link
4. UE Editor에서 VR Preview 가능한지 확인:
   - 툴바 → Play 버튼 옆 드롭다운 → **VR Preview** 활성화 확인

---

### A.7 — Step A 완료 검증

다음 모두 ✅ 이어야 Step B 진입:

- [ ] `LiftgateStudy.uproject` 실행 시 에러 없이 진입
- [ ] Plugin 4개 활성화 상태 (Meta XR / ISDK / OpenXR / OpenXRHandTracking)
- [ ] Hand Tracking Support = "Controllers and Hands"
- [ ] Quest 3 + Quest Link 연결 정상
- [ ] VR Preview 옵션 활성화됨
- [ ] `C:\Reference\Unreal-InteractionSDK-Sample\` Sample clone 완료

전부 ✅ → **Step B 진입**.

---

## Step B — Claude Code 작업

> Claude Code 세션 시작 메시지:
> ```
> @CLAUDE.md
> @docs/phase_kickoff/Phase1_Kickoff.md
> @Project_Setup_Prompt.md
>
> Step A 완료. Step B를 시작하자.
> ```
>
> Claude Code는 본 §Step B의 항목을 **순서대로** 실행. 각 단계 완료 시 사용자에게 검증 요청.

---

### B.1 — Git 초기화

```bash
cd C:\Git\LiftgateStudy
git init
git branch -M main
git remote add origin <회사 git repo 주소>
```

⚠️ `<회사 git repo 주소>` 는 사용자에게 확인 후 입력. 사전에 빈 repo가 회사 git 서버에 생성되어 있어야 함.

---

### B.2 — .gitignore 작성

프로젝트 루트에 `.gitignore` 생성:

```gitignore
# UE generated
Binaries/
Intermediate/
Saved/
DerivedDataCache/
*.sln
*.suo
*.VC.db
*.opensdf
*.opendb
*.sdf
*.vcxproj.user

# Plugin binaries (재컴파일 가능, 용량 큼)
Plugins/*/Binaries/
Plugins/*/Intermediate/

# Python cache
__pycache__/
*.pyc

# OS / IDE
Thumbs.db
.DS_Store
.vscode/
.idea/
```

⚠️ `Plugins/MetaXR/` 와 `Plugins/MetaXRInteraction/` 의 **source**는 git에 포함. **Binaries/Intermediate**만 제외.

---

### B.3 — 폴더 구조 생성

```bash
mkdir -p docs/phase_kickoff
mkdir -p docs/decisions
mkdir -p docs/lessons_learned

# 빈 폴더 git tracking 위해
touch docs/lessons_learned/.gitkeep
```

최종 구조:
```
LiftgateStudy/
├── .gitignore                          ← B.2
├── CLAUDE.md                           ← 사용자 제공 (별도 복사)
├── README.md                           ← B.5
├── LiftgateStudy.uproject              ← UE 생성
├── docs/
│   ├── phase_kickoff/
│   │   └── Phase1_Kickoff.md          ← 사용자 제공 (별도 복사)
│   ├── decisions/
│   │   ├── ADR_001_isdk_grab_layer.md ← B.4
│   │   ├── ADR_002_modes_free_toggle.md
│   │   └── ADR_003_no_logging.md
│   └── lessons_learned/
│       └── .gitkeep
├── Config/                             ← UE 관리
├── Content/                            ← UE 관리
└── Plugins/                            ← 부분 제외 (.gitignore)
    ├── MetaXR/
    └── MetaXRInteraction/
```

---

### B.4 — ADR 3개 작성

`docs/decisions/` 폴더에 다음 ADR을 박제. 각 파일은 아래 템플릿 따름.

#### 공통 템플릿

```markdown
# ADR-NNN — <Title>

**Date**: 2026-05-13
**Status**: Accepted
**Phase**: 1

## Context
<배경 설명. 왜 이 결정이 필요했는지>

## Decision
<무엇을 결정했는지. 단호하게>

## Consequences
**Positive**:
- ...

**Negative**:
- ...

**Neutral**:
- ...

## Notes
<참고 사항, 향후 재검토 트리거 조건>
```

#### ADR_001_isdk_grab_layer.md

- **Title**: ISDK-based Hand Grab Layer
- **Context**: Hand grab 인터랙션을 어떻게 구현할지 선택 필요. 자체 pinch detection vs 검증된 SDK 사용.
- **Decision**: Meta XR Interaction SDK v78의 `IsdkGrabbableComponent` 기반으로 통일. 자체 pinch/grab 로직 작성 금지.
- **Consequences**:
  - + 안정적 grab 판정 (tracking confidence, debouncing, pinch detection SDK 내장)
  - + 평가자 간 일관성 확보 (custom 코드 차이 없음)
  - + Meta 공식 sample과 동일 패턴 → 학습 자료 풍부
  - − SDK 버전 의존성. v78 lock-in
  - − SDK upgrade 시 호환성 검증 필요
- **Notes**: Phase 2 진입 시 ISDK API surface 점검. v85+ upgrade는 별도 ADR 후 결정.

#### ADR_002_modes_free_toggle.md

- **Title**: Liftgate Modes as Free Toggle, Not Vehicle Property
- **Context**: 4가지 mode (Manual-Full / Manual-Assist / Power-Auto / Power-Hybrid) 를 차량별 고정 속성으로 둘지, 평가자가 자유 토글할지 결정 필요.
- **Decision**: 자유 토글. 차량 metadata에 `SupportedModes` 필드 없음. 모든 차량은 4가지 mode를 평가 가능.
- **Consequences**:
  - + 같은 차량을 4가지 mode로 비교 평가 가능 (핵심 평가 가치)
  - + 차량 metadata 단순화 (mode 정의 불필요)
  - + UI 단순화 (mode 가용성 분기 없음)
  - − 실차에 없는 mode도 체험 가능 → 실차 충실도 ↓
  - − 평가자가 "이 차의 실제 mode는 뭔지" 별도 인지 필요
- **Notes**: 향후 실차 충실도가 요구되면 차량별 "Recommended Mode" 표시 추가 가능 (강제 제한은 안 함).

#### ADR_003_no_logging.md

- **Title**: No Evaluation Data Logging
- **Context**: 평가 시 정량 데이터 (각도, 시간, zone hit, grab 위치 등) 로깅 필요 여부.
- **Decision**: 일체 로깅하지 않음. 평가는 평가자의 주관적 체험에만 의존. CSV/JSON/DB 출력 코드 작성 금지.
- **Consequences**:
  - + 구현 단순화 (로깅 인프라 불필요)
  - + 평가자 부담 감소 (개인정보/평가내용 익명성)
  - + 빠른 iteration (data schema 협의 불필요)
  - − 사후 정량 분석 불가
  - − 평가자 간 정량 비교 불가
  - − 시간 경과 후 평가 결과 재현/검증 불가
- **Notes**: 향후 정량 평가 필요 시 별도 Phase로 분리. 본 프로젝트는 "주관적 느낌 전달" 도구로 고정.

---

### B.5 — README.md 작성

프로젝트 루트에 `README.md` 작성:

```markdown
# Liftgate Study

GMTCK PQDQ — VR-based liftgate interaction evaluation tool.

## Purpose
주관적 평가 도구. 다양한 차량의 liftgate 개폐 동작을 4가지 mode (Manual-Full / Manual-Assist / Power-Auto / Power-Hybrid) 로 VR에서 체험.

## Requirements
- Unreal Engine **5.5.4**
- Meta XR Plugin **v78**
- Meta XR Interaction SDK **v78**
- Meta Quest 3 + Quest Link (PCVR)
- Visual Studio 2022 (plugin compilation, optional)

## Setup
1. Clone this repo to `C:\Git\LiftgateStudy`
2. Download Meta XR + Interaction SDK plugins **v78** from Meta Quest Developer Hub
3. Unzip both to `Plugins/` folder
4. Open `LiftgateStudy.uproject`
5. If prompted to compile modules, click Yes

## Documentation
- `CLAUDE.md` — Project governance rules
- `docs/phase_kickoff/` — Phase plans (Phase 1~4)
- `docs/decisions/` — Architecture Decision Records (ADRs)
- `docs/lessons_learned/` — Discovered pitfalls, workarounds

## Current Phase
**Phase 1 — Foundation** (in progress)

See `docs/phase_kickoff/Phase1_Kickoff.md` for current work plan.

## ISDK Sample Reference
The Meta ISDK Sample (5.5.4-v78 branch) is used as reference for hand rig BP migration.
Not included in this repo. Clone separately to `C:\Reference\Unreal-InteractionSDK-Sample\`.

## License / Ownership
GMTCK PQDQ internal evaluation tool. Not for external distribution.
```

---

### B.6 — CLAUDE.md 버전 수정

기존 CLAUDE.md §2 Tech Stack 표 수정:

```diff
- | Engine | Unreal Engine 5.5.2 |
+ | Engine | Unreal Engine 5.5.4 |

- | SDK | Meta XR Plugin + Meta XR Interaction SDK (v85.0 이상) |
+ | SDK | Meta XR Plugin v78 + Meta XR Interaction SDK v78 |
```

§12 Versioning의 Change Log에 추가:
```
- **v0.2** (2026-05-13) — Tech stack 버전 확정 (UE 5.5.4, ISDK v78). Path B 셋업 채택.
```

CLAUDE.md 상단의 현재 버전도 `v0.2` 로 bump.

---

### B.7 — 첫 commit

```bash
cd C:\Git\LiftgateStudy

# 문서 commit
git add CLAUDE.md README.md .gitignore docs/
git commit -m "chore: initial project setup with governance docs and Phase 1 plan"

# UE 프로젝트 파일 commit
git add LiftgateStudy.uproject Config/
git commit -m "chore: add UE 5.5.4 project files"

# Plugin source commit (Binaries/Intermediate은 .gitignore로 제외됨)
git add Plugins/
git commit -m "chore: add Meta XR plugins v78 (source only)"

# Push
git push -u origin main
```

⚠️ Plugin source 크기가 큼 (수백 MB). 회사 git 서버의 repo 크기 제한 확인.
초과 시 Git LFS 사용 또는 plugin을 외부 폴더 참조 방식으로 변경.

---

### B.8 — Step B 완료 검증

다음 모두 ✅ 이어야 Phase 1 W1.1 진입:

- [ ] git repo 초기화 완료, remote 연결됨
- [ ] CLAUDE.md (v0.2 업데이트됨) 루트 배치
- [ ] Phase1_Kickoff.md → `docs/phase_kickoff/` 배치
- [ ] ADR 3개 → `docs/decisions/` 작성 완료
- [ ] README.md 작성 완료
- [ ] `.gitignore` 에 UE 생성 파일 제외 규칙 포함
- [ ] 첫 commit + push 성공
- [ ] git log 확인 → commit 3개 (docs / project / plugins) 보임

---

## Step B 완료 후 다음 작업

**Phase 1 W1.1 (Project 초기 셋업 Blueprint 부분) 진입.**

Claude Code 세션에서:
```
@docs/phase_kickoff/Phase1_Kickoff.md

W1.1 시작하자. CLAUDE.md §3 R1 (Tracking Origin Floor Level) 잊지 말 것.
```

이후 W1.1 → W1.2 → ... → W1.6 순서대로 진행 (Phase1_Kickoff §10 권장 순서).

각 W 항목 완료 시 사용자 검증 요청 → 통과 시 다음 W로.

---

## 막힐 시 디버그

| 증상 | 1차 확인 |
|---|---|
| Plugin 자동 컴파일 실패 | Visual Studio 2022 + `Game development with C++` workload 설치 |
| `.uproject` 실행 시 module missing 에러 | Plugins 폴더 구조 (MetaXR/, MetaXRInteraction/) 정확한지 |
| Hand Tracking Support 옵션 안 보임 | Meta XR Plugin 활성화 후 Editor restart 했는지 |
| Quest Link 연결 안 됨 | Meta Quest Link PC 앱 설치, USB-C 데이터 케이블 (충전 전용 X) |
| VR Preview 메뉴 없음 | Plugin 활성화 + restart, Quest 헤드셋 연결 상태 |
| git push 시 size limit 에러 | Git LFS 도입 or Plugins/ 를 .gitignore 처리 (대신 README에 설치 안내) |

해결 안 되는 이슈는 `docs/lessons_learned/<topic>.md` 에 즉시 박제.

---

## ⚠️ 절대 하지 말 것 (셋업 중)

- ❌ Plugin을 Engine 폴더 (`C:\Program Files\Epic Games\UE_5.5\Engine\Plugins\`) 에 설치 — 프로젝트 단위로 격리 위해 반드시 **프로젝트의 Plugins/** 에만 설치
- ❌ Sample repo를 본인 프로젝트 안에 merge — 참조 자료로만 사용 (별도 폴더)
- ❌ CLAUDE.md의 Critical Rules (R1~R7) 무시 — 셋업 단계에서도 적용 (특히 R5: ISDK 외 hand grab 작성 금지)
- ❌ ADR 없이 결정 변경 — 본 문서 외 결정 사항은 Claude Code가 임의 변경 금지, 사용자 확인 후 ADR 박제
