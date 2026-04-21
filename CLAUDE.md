# 모의투자 Qt 앱 (stock_scheduler) — 메인 문서

> **📚 전체 문서는 `rules/` 폴더에 기능별로 분리되어 있습니다.**

---

## 🚀 빠른 시작

```bash
# 빌드
mkdir build && cd build
cmake -G "MinGW Makefiles" ..
mingw32-make -j4

# 실행
./stock_scheduler.exe
```

**초기 데이터**:
- KRW 현금: 50,000,000원
- USD 현금: $10,000.00
- 제공 종목: 15개 (주식, 암호화폐)

---

## 📖 문서 구조

### 핵심 문서 (먼저 읽기)
| 문서 | 설명 |
|------|------|
| **[프로젝트 개요](rules/_overview.md)** | 프로젝트 소개, 디렉토리 구조, 빌드 방법 |
| **[파일별 역할](rules/_files-reference.md)** | 각 소스파일의 클래스, 함수, API 상세 |

### 기능별 상세 문서
| 문서 | 설명 |
|------|------|
| **[DB 스키마](rules/_database-schema.md)** | 테이블 정의, 쿼리 패턴, 데이터 흐름 |
| **[UI 레이아웃](rules/_ui-layout.md)** | 6개 탭 구성, 위젯, 신호-슬롯 |
| **[이중통화](rules/_dual-currency.md)** | KRW/USD 자산 관리, 환전 메커니즘 |
| **[LIMIT 주문](rules/_limit-order.md)** | 자동 체결 조건, 타이머, 재시도 로직 |

### 동작 및 테스트 문서
| 문서 | 설명 |
|------|------|
| **[주요 동작 흐름](rules/_trading-flow.md)** | 앱 시작, 매매, 환전, LIMIT 체결 시나리오 |
| **[빌드 & 테스트](rules/_build-test.md)** | 컴파일 오류 해결, 테스트 체크리스트 |

### 개발 및 리팩토링 문서
| 문서 | 설명 |
|------|------|
| **[객체화 리팩토링](rules/_refactoring-oop.md)** | OOP 설계, 데이터/서비스 계층 분리, 예외 처리 |

### 향후 계획
| 문서 | 설명 |
|------|------|
| **[향후 개선사항](rules/_future-improvements.md)** | Priority별 로드맵, 기술 개선, 확장 계획 |

---

## 🎯 주요 기능

✅ **MARKET 매매**
- 주식 (KRW) 및 암호화폐 (USD) 거래
- 실시간 자산 갱신

✅ **LIMIT 주문**
- 가격 기반 자동 체결 (1초 주기)
- 재시도 메커니즘

✅ **이중통화**
- KRW (한국원) 및 USD (미국달러) 동시 관리
- 실시간 환율 적용 환전

✅ **자동 예약**
- MARKET 주문: 앱 재시작 시 자동 실행
- LIMIT 주문: 매 1초 체결 조건 검사

✅ **포트폴리오 추적**
- 보유 종목, 평가손익, 수익률
- 통화별 색상 구분 (USD/KRW)

✅ **메모 & 할일**
- 거래 메모 추가/삭제
- 체크박스로 완료 토글

---

## 📊 프로젝트 상태

- **진행률**: 75% 완료 ✅
- **남은 작업**: 빌드 테스트 및 버그 수정 (25%)

### 완료된 항목
- ✅ 이중통화 (KRW/USD) 구현
- ✅ 환전 기능
- ✅ LIMIT 주문 자동 체결
- ✅ 타이머 기반 주기 검사

### 예정 항목
- ⏳ Qt Creator 빌드 (1-2시간)
- ⏳ 기능별 테스트 (30-45분)
- ⏳ 버그 수정 (30-60분)

---

## 🛠️ 기술 스택

```
언어: C++ 17
UI: Qt 6.11.0 (Qt Designer)
데이터베이스: SQLite
빌드: CMake 3.16+
컴파일러: MinGW 64-bit
OS: Windows 11
```

---

## 📋 의존성

```cmake
find_package(Qt6 COMPONENTS
    Core
    Gui
    Widgets
    Sql       # SQLite 지원
    REQUIRED
)
```

---

## 🔍 핵심 클래스

| 클래스 | 역할 |
|-------|------|
| **DatabaseManager** | 싱글톤 DB 관리 (CRUD, 환전, LIMIT 체결) |
| **MainWindow** | 메인 UI, 탭 관리, 슬롯 구현 |
| **PortfolioModel** | QAbstractTableModel (포트폴리오 테이블) |
| **StockData** | 하드코딩 종목 (15개) |

---

## 🚦 빠른 참조

### 자주 찾는 정보

**Q: LIMIT 주문은 어떻게 작동하나요?**  
→ [LIMIT 주문 메커니즘](rules/_limit-order.md) 참조

**Q: 이중통화는 어떻게 구현되었나요?**  
→ [이중통화 기능](rules/_dual-currency.md) 참조

**Q: 빌드 오류가 발생했어요**  
→ [빌드 & 테스트](rules/_build-test.md#컴파일-오류-해결) 참조

**Q: 매매 흐름을 알고 싶어요**  
→ [주요 동작 흐름](rules/_trading-flow.md) 참조

**Q: DB 스키마를 확인하고 싶어요**  
→ [DB 스키마](rules/_database-schema.md) 참조

---

## 📁 파일 구조

```
stock_scheduler/
├── CMakeLists.txt                  # 빌드 설정
├── main.cpp                        # 진입점
├── mainwindow.h / .cpp             # 메인 UI
├── mainwindow.ui                   # UI 레이아웃
├── database.h / .cpp               # DB 관리
├── stockdata.h                     # 종목 데이터
├── portfoliomodel.h / .cpp         # 포트폴리오 모델
├── CLAUDE.md                       # 이 파일 (메인 인덱스)
├── rules/                          # 상세 문서
│   ├── _overview.md                # 프로젝트 개요
│   ├── _files-reference.md         # 파일별 역할
│   ├── _database-schema.md         # DB 스키마
│   ├── _ui-layout.md               # UI 레이아웃
│   ├── _dual-currency.md           # 이중통화
│   ├── _limit-order.md             # LIMIT 주문
│   ├── _trading-flow.md            # 동작 흐름
│   ├── _build-test.md              # 빌드 & 테스트
│   └── _future-improvements.md     # 향후 개선
└── stock_portfolio.db              # SQLite DB (런타임 생성)
```

---

## 🎓 학습 순서 (처음 사용자)

1. **[프로젝트 개요](rules/_overview.md)** — 전체 그림 파악
2. **[빌드 & 테스트](rules/_build-test.md)** — 빌드 및 실행
3. **[UI 레이아웃](rules/_ui-layout.md)** — 6개 탭 이해
4. **[주요 동작 흐름](rules/_trading-flow.md)** — 매매, 환전, LIMIT 동작
5. **[파일별 역할](rules/_files-reference.md)** — 코드 구조
6. **[DB 스키마](rules/_database-schema.md)** — 데이터 관리
7. **[이중통화](rules/_dual-currency.md)** & **[LIMIT 주문](rules/_limit-order.md)** — 고급 기능

## 💻 개발자 순서 (코드 수정 시)

1. **[객체화 리팩토링](rules/_refactoring-oop.md)** — OOP 설계 및 구현 순서
2. **[파일별 역할](rules/_files-reference.md)** — API 상세 정보
3. **[주요 동작 흐름](rules/_trading-flow.md)** — 통합 방식 이해

---

## 🔗 외부 링크

- **Qt Documentation**: https://doc.qt.io/qt-6/
- **SQLite Documentation**: https://www.sqlite.org/docs.html
- **CMake Documentation**: https://cmake.org/documentation/
- **MinGW**: https://www.mingw-w64.org/

---

## 📝 라이선스

MIT License — 자유롭게 사용, 수정, 배포 가능

---

## ❓ 문제 해결

### 빌드 실패
→ [빌드 & 테스트 - 컴파일 오류 해결](rules/_build-test.md#컴파일-오류-해결)

### 기능 작동 안됨
→ [빌드 & 테스트 - 테스트 체크리스트](rules/_build-test.md#테스트-체크리스트)

### 버그 보고
→ Issues 섹션에서 상세 설명 및 재현 단계 포함

---

**마지막 업데이트**: 2026-04-21  
**작성자**: Claude Code  
**버전**: 1.0.0-beta
