# 모의투자 Qt 앱 (stock_scheduler)

**📚 전체 문서는 `rules/` 폴더에 기능별로 분리되어 있습니다.**

---

## 🚀 빠른 시작

```bash
mkdir build && cd build
cmake -G "MinGW Makefiles" ..
mingw32-make -j4
./stock_scheduler.exe
```

**초기 자산**: KRW 50M원, USD $10,000 (종목 15개)

---

## 📖 문서 인덱스

| 문서 | 설명 |
|------|------|
| **[개요](rules/_overview.md)** | 프로젝트 소개, 기능, 기술 스택 |
| **[파일 역할](rules/_files-reference.md)** | 클래스, 함수, API 상세 |
| **[DB 스키마](rules/_database-schema.md)** | 테이블 정의, 쿼리 패턴 |
| **[UI 레이아웃](rules/_ui-layout.md)** | 6개 탭 구성, 위젯 배치 |
| **[이중통화](rules/_dual-currency.md)** | KRW/USD 관리, 환전 메커니즘 |
| **[LIMIT 주문](rules/_limit-order.md)** | 자동 체결, 재시도 로직 |
| **[동작 흐름](rules/_trading-flow.md)** | 매매, 환전, LIMIT 체결 시나리오 |
| **[빌드 & 테스트](rules/_build-test.md)** | 컴파일, 오류 해결, 테스트 체크 |
| **[리팩토링](rules/_refactoring-oop.md)** | OOP 설계 및 구현 계획 |
| **[향후 개선](rules/_future-improvements.md)** | 로드맵, 기술 개선 사항 |

---

## 🔍 핵심 클래스

| 클래스 | 역할 |
|-------|------|
| **DatabaseManager** | DB 관리 (CRUD, 환전, LIMIT 체결) |
| **MainWindow** | 메인 UI, 탭 관리 |
| **PortfolioModel** | 포트폴리오 테이블 모델 |

---

## 🛠️ 기술 스택

- **언어**: C++ 17
- **UI**: Qt 6.11.0 (Qt Designer)
- **DB**: SQLite
- **컴파일러**: MinGW 64-bit
- **빌드**: CMake 3.16+

---

## 📚 학습 순서

**처음 사용자**:
1. [개요](rules/_overview.md)
2. [빌드 & 테스트](rules/_build-test.md)
3. [UI 레이아웃](rules/_ui-layout.md)
4. [동작 흐름](rules/_trading-flow.md)

**개발자**:
1. [파일 역할](rules/_files-reference.md)
2. [DB 스키마](rules/_database-schema.md)
3. [리팩토링](rules/_refactoring-oop.md)

---

**마지막 업데이트**: 2026-04-21 | **버전**: 1.0.0-beta
