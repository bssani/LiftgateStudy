# ADR-006 — Evaluation Result Logging (Partial Supersede of ADR-003)

**Date**: 2026-05-18
**Status**: Accepted
**Phase**: 3
**Supersedes (partial)**: ADR-003 (No Evaluation Data Logging) — 결과 부분만

## Context

ADR-003 (Phase 1) 은 "평가 데이터 일체 로깅 금지" 였다. Phase 1 의 단순 체험 도구 목적 에서는 합리적. 하지만 Phase 3 에서 평가 도구의 본질이 바뀜:

- **Phase 3 신규 목적**: 4 차량 paired comparison + final ranking → **외부 (Excel / Python / R) 에서 그래프 / 통계 분석**
- 평가자 머리 속에만 남기면 N 명 평가의 집계 불가능

ADR-003 §Notes 에 이미 escape hatch 명시: "향후 정량 평가 필요 시 별도 Phase 로 분리. 본 프로젝트는 '주관적 느낌 전달' 도구로 고정." → 본 ADR 이 그 trigger.

다만 **세부 데이터 (각 grab 의 시간, 각도, 속도, hand position 등) 로깅은 여전히 금지**. 본 ADR 은 **세션 결과만** 저장.

## Decision

평가 세션 완료 시 다음 데이터를 **JSON 파일** 1 개로 저장:

```json
{
  "schema_version": 1,
  "session_id": "550e8400-e29b-41d4-a716-446655440000",
  "timestamp_utc": "2026-05-18T07:23:45Z",
  "tests": [
    { "slot": 1, "model_code": "C1YC" },
    { "slot": 2, "model_code": "Equinox" },
    { "slot": 3, "model_code": "Telluride" },
    { "slot": 4, "model_code": "RAV4" }
  ],
  "comparison_1v2_winner_slot": 2,
  "final_ranking_by_slot": [2, 1, 4, 3]
}
```

저장 위치: `<ProjectDir>/Saved/EvaluationLogs/session_<UTC_yyyymmdd_HHMMSS>_<short_uuid>.json`

세부 / 비-저장 데이터 (여전히 금지):
- ❌ Grab 시간, hand trajectory, liftgate 각속도
- ❌ Calibration 시도 횟수 / 실패 메시지
- ❌ 평가자 인적사항 (anonymous)

저장 데이터 (저장 OK):
- ✅ Session 의 식별자 (UUID, anonymous)
- ✅ UTC 타임스탬프
- ✅ 4 vehicle 의 slot ↔ model_code 매핑
- ✅ Comparison 1v2 의 winner slot
- ✅ Final ranking (slot 순서 array)

## Format 선택 이유 — JSON

| 비교 | CSV | JSON |
|---|---|---|
| 평가자 1 명 = 1 row / 1 file | row 형식 자연 | file 형식 자연 |
| 다중 평가 집계 | 1 file 에 누적 | 폴더 안 N file → 외부에서 merge |
| 스키마 변경 시 | column 추가 / 호환 어려움 | nested object / version 필드로 호환 ↑ |
| Excel 친화 | 직접 import OK | pandas / R 에서 trivial, Excel 은 변환 필요 |

→ **사용자 선택: JSON** (확장성 우선)

세션 별 1 file. 외부 분석 시 폴더 전체 read → pandas / R 로 dataframe.

## Privacy / Anonymous

- `session_id` 는 UUID v4 — 평가자 식별 불가
- 인적사항 입력 받지 않음
- `Saved/` 디렉토리는 `.gitignore` 에 의해 git 제외 → 평가 데이터 commit 사고 방지
- 평가 데이터 백업 책임은 본 프로젝트 범위 외 (운영자가 별도 위치로 복사)

## Consequences

**Positive**:
- 외부 분석 (그래프, 통계) 가능
- 평가자 간 집계 가능 (N 명 → 평균 / 분포)
- Schema versioning 으로 향후 데이터 확장 호환
- Anonymous + git 무시 → privacy 안전망

**Negative**:
- 세션 도중 quit / crash 시 미저장 (final ranking 후에만 write)
- 세션 timing 정보 부재 — Phase 3.x 또는 별도 Phase 진입 시 추가 ADR 필요
- 평가자가 본인 결과 확인 불가 (anonymous 의 trade-off)

**Neutral**:
- ADR-003 의 다른 항목 (CSV / file 출력 금지 → 본 결과 file 은 예외) 은 본 ADR 이 supersede
- Phase 1/2 의 calibration / liftgate 동작 데이터는 여전히 저장 안 함

## Implementation

- C++ helper: `FString UEvaluationSessionSubsystem::SerializeSessionToJson()` 의 결과를 `FFileHelper::SaveStringToFile()` 으로 디스크에 write
- `FPaths::ProjectSavedDir() / "EvaluationLogs" / FileName`
- 디렉토리 없으면 자동 생성 (`IFileManager::Get().MakeDirectory(..., true)`)
- 파일명 collision 회피: timestamp + short UUID

## Notes

- 평가자가 결과 보기 원하면 별도 phase 에서 in-VR result viewer 검토
- 데이터 schema 변경 시 `schema_version` bump + parser 측 호환 처리
- Phase 4 진입 시 시간 / detail 데이터 도입 검토 — 별도 ADR
