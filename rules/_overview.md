# 프로젝트 개요 및 구조

## 프로젝트 개요
**stock_scheduler** — 로컬 SQLite 기반 모의투자 포트폴리오 추적 앱
- **기술스택**: Qt 6.11.0 (C++17) / CMake / SQLite
- **UI**: 6탭 인터페이스 (Qt Designer)
- **특징**: 로컬 저장, 하드코딩 종목, 자동 예약 실행
- **고급기능**: 이중통화(KRW/USD), LIMIT 주문 자동 체결

---

## 디렉토리 구조

```
stock_scheduler/
├── CMakeLists.txt          # Qt6 CMake 빌드 설정 (Qt::Sql 포함)
├── main.cpp                # 애플리케이션 진입점
├── mainwindow.h / .cpp     # 메인 윈도우 + 슬롯 구현 (이중통화 + LIMIT 주문)
├── mainwindow.ui           # 6탭 UI 레이아웃 (Qt Designer XML)
├── database.h / .cpp       # DatabaseManager: CRUD + 환전 + LIMIT 체결
├── stockdata.h             # 하드코딩 종목 15개 (구조체 + 헬퍼)
├── portfoliomodel.h / .cpp # QAbstractTableModel: 포트폴리오 (USD/KRW 구분)
├── PLAN.md                 # 구현 계획 문서
├── CLAUDE.md               # 메인 문서 (이 파일)
├── rules/                  # 기능별 상세 문서
│   ├── _overview.md               # 프로젝트 개요 (이 파일)
│   ├── _files-reference.md        # 파일별 역할
│   ├── _database-schema.md        # DB 스키마 및 API
│   ├── _ui-layout.md              # UI 탭 레이아웃
│   ├── _dual-currency.md          # 이중통화 기능
│   ├── _limit-order.md            # LIMIT 주문 메커니즘
│   ├── _trading-flow.md           # 주요 동작 흐름
│   ├── _build-test.md             # 빌드 및 테스트
│   └── _future-improvements.md    # 향후 개선사항
└── stock_portfolio.db      # SQLite DB (런타임 생성)
```

---

## 구현 상태 및 진행률

- **현재 진행률**: 75% 완료
- **완료된 기능**: 이중통화 (KRW/USD), 환전, LIMIT 주문 (프론트엔드/백엔드)
- **남은 작업**: 빌드 테스트 및 버그 수정 (25%)

---

## 빌드 및 실행

### 빌드
1. Qt Creator에서 프로젝트 열기
2. **Build** → **Build Project** 클릭
3. MinGW 64-bit 컴파일러 사용

### 실행
```bash
./build/Desktop_Qt_6_11_0_MinGW_64_bit-Debug/stock_scheduler.exe
```

### 출력 파일
- **stock_portfolio.db** — 현재 디렉토리에 자동 생성

---

## 문서 네비게이션

- **[파일별 역할](rules/_files-reference.md)** — 각 소스파일의 목적 및 API
- **[DB 스키마](rules/_database-schema.md)** — 데이터베이스 테이블 및 DatabaseManager 함수
- **[UI 레이아웃](rules/_ui-layout.md)** — 6개 탭 구성 및 위젯
- **[이중통화](rules/_dual-currency.md)** — KRW/USD 자산 관리
- **[LIMIT 주문](rules/_limit-order.md)** — 자동 체결 메커니즘
- **[주요 동작 흐름](rules/_trading-flow.md)** — 앱 실행, 매매, 환전, 예약 흐름
- **[빌드 및 테스트](rules/_build-test.md)** — 컴파일, 테스트 체크리스트
- **[향후 개선사항](rules/_future-improvements.md)** — 선택사항 및 확장 계획
