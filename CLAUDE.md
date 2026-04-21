# 모의투자 Qt 앱 프로젝트 문서

## 프로젝트 개요
**stock_scheduler** — 로컬 SQLite 기반 모의투자 포트폴리오 추적 앱
- **기술스택**: Qt 6.11.0 (C++17) / CMake / SQLite
- **UI**: 5탭 인터페이스 (Qt Designer)
- **특징**: 로컬 저장, 하드코딩 종목, 자동 예약 실행

---

## 디렉토리 구조

```
stock_scheduler/
├── CMakeLists.txt          # Qt6 CMake 빌드 설정 (Qt::Sql 포함)
├── main.cpp                # 애플리케이션 진입점
├── mainwindow.h / .cpp     # 메인 윈도우 + 슬롯 구현
├── mainwindow.ui           # 5탭 UI 레이아웃 (Qt Designer XML)
├── database.h / .cpp       # DatabaseManager: CRUD 연산
├── stockdata.h             # 하드코딩 종목 15개 (구조체 + 헬퍼)
├── portfoliomodel.h / .cpp # QAbstractTableModel: 포트폴리오 테이블
├── PLAN.md                 # 구현 계획 문서
├── CLAUDE.md               # 이 파일
└── stock_portfolio.db      # SQLite DB (런타임 생성)
```

---

## 파일별 역할

### CMakeLists.txt
- Qt6 6.5+ 요구 (Core, Widgets, **Sql**)
- 소스파일 + 헤더 정의
- CMake 기반 빌드

### stockdata.h
```cpp
struct Stock {
    QString symbol;   // "005930"
    QString name;     // "삼성전자"
    double  price;    // 78000
};
```
- `stockList()` — 15종목 static 벡터
- `stockPrice(symbol)` — 종목 현재가 조회
- `stockName(symbol)` — 종목명 조회

### database.h / database.cpp (DatabaseManager)

**싱글톤 패턴**: `DatabaseManager::instance()`

#### 초기화
- `initDb()` — DB 연결, 테이블 생성, 초기 현금(5000만원) 설정

#### 계좌 / 포트폴리오
- `getCash()` → double
- `getHoldings()` → QVector<HoldingRow>  (보유 종목)
- `getTrades()` → QVector<TradeRow>  (거래 이력)

#### 매매
- `buyStock(symbol, qty, price)` → bool
  - 현금 차감 → holding 추가/갱신 → trade 기록
  - 실패: 잔액 부족
- `sellStock(symbol, qty, price)` → bool
  - 현금 추가 → holding 수량 감소 → trade 기록
  - 실패: 보유 수량 부족

#### 예약 (Schedule)
- `addSchedule(date, symbol, type, qty, price)` → bool
- `getSchedules()` → QVector<ScheduleRow>
- `executeDueSchedules()`
  - 앱 시작 시 호출
  - 오늘 날짜 이하 "pending" 예약 자동 실행
  - 성공 시 status → "done"

#### 메모 (Memo)
- `addMemo(content)` → bool
- `getMemos()` → QVector<MemoRow>
- `toggleMemo(id)` → bool  (done 토글)
- `deleteMemo(id)` → bool

### portfoliomodel.h / portfoliomodel.cpp

`QAbstractTableModel` 서브클래스

**컬럼** (7개):
1. 종목코드
2. 종목명
3. 수량
4. 평균단가 (한국 로케일 포맷)
5. 현재가
6. 평가손익 (계산: (현재가 - 평균단가) × 수량)
7. 수익률(%) (계산: (현재가 - 평균단가) / 평균단가 × 100)

**특징**:
- 손익 양수: 빨강 (Qt::red)
- 손익 음수: 파랑 (Qt::blue)
- 우측 정렬 (숫자)

### mainwindow.h / mainwindow.cpp

**UI 위젯 접근**: `ui->위젯명` (ui_mainwindow.h 자동생성)

**슬롯**:
- `onSymbolChanged(int)` — 종목 선택 → 현재가 + 총금액 갱신
- `onQtyChanged(int)` — 수량 변경 → 총금액 갱신
- `onBuyClicked()` — 매수 → 포트폴리오/거래내역 갱신
- `onSellClicked()` — 매도 → 포트폴리오/거래내역 갱신
- `onSchedAddClicked()` — 예약 등록
- `onMemoAddClicked()` — 메모 추가
- `onMemoDeleteClicked()` — 메모 삭제
- `onMemoItemChanged(item)` — 메모 체크박스 → 완료 토글
- `onTabChanged(int)` — 탭 전환 시 전체 갱신

**헬퍼**:
- `setupStockCombo()` — trade/sched 콤보박스 초기화
- `refreshPortfolio()` — 포트폴리오 테이블 + 요약 갱신
- `refreshHistory()` — 거래내역 테이블 갱신
- `refreshSchedules()` — 예약 테이블 갱신
- `refreshMemos()` — 메모 목록 갱신
- `updateTradeSummary()` — 매매탭 현재가/총금액 표시

### mainwindow.ui

**5탭 구성**:

#### Tab 1: 포트폴리오
```
[현금 잔액: ---] [총 평가액: ---] [총 손익: ---]
┌─────────────────────────────────┐
│ QTableView (PortfolioModel)      │
└─────────────────────────────────┘
```

#### Tab 2: 매매
```
종목: [QComboBox]
현재가: [라벨]
수량: [QSpinBox]
총 금액: [라벨]
[매수 버튼] [매도 버튼]
```

#### Tab 3: 거래내역
```
┌──────────────────────────────────────┐
│ QTableWidget: 시간/종목/유형/수량/가격 │
└──────────────────────────────────────┘
```

#### Tab 4: 매매 예약
```
예약 날짜: [QDateEdit]
종목: [QComboBox]
유형: [매매 타입 콤보]
수량: [QSpinBox]
[예약 등록 버튼]
┌──────────────────────────────────────┐
│ QTableWidget: 날짜/종목/유형/수량/상태 │
└──────────────────────────────────────┘
```

#### Tab 5: 메모/할일
```
[QLineEdit] [추가] [삭제]
┌──────────────────────┐
│ QListWidget (체크박스) │
└──────────────────────┘
```

---

## DB 스키마

### account
```sql
CREATE TABLE account (
  id   INTEGER PRIMARY KEY,
  cash REAL
);
-- 초기값: (1, 50000000)
```

### holdings
```sql
CREATE TABLE holdings (
  symbol    TEXT PRIMARY KEY,
  quantity  INTEGER,
  avg_price REAL
);
```

### trades
```sql
CREATE TABLE trades (
  id        INTEGER PRIMARY KEY AUTOINCREMENT,
  symbol    TEXT,
  type      TEXT,      -- 'BUY' | 'SELL'
  quantity  INTEGER,
  price     REAL,
  timestamp TEXT       -- 'YYYY-MM-DD HH:MM:SS'
);
```

### schedules
```sql
CREATE TABLE schedules (
  id       INTEGER PRIMARY KEY AUTOINCREMENT,
  date     TEXT,      -- 'YYYY-MM-DD'
  symbol   TEXT,
  type     TEXT,      -- 'BUY' | 'SELL'
  quantity INTEGER,
  price    REAL,
  status   TEXT DEFAULT 'pending'  -- 'pending' | 'done'
);
```

### memos
```sql
CREATE TABLE memos (
  id         INTEGER PRIMARY KEY AUTOINCREMENT,
  content    TEXT,
  done       INTEGER DEFAULT 0,  -- 0=false, 1=true
  created_at TEXT               -- 'YYYY-MM-DD HH:MM:SS'
);
```

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

## 주요 동작 흐름

### 앱 시작
1. `main()` → MainWindow 생성
2. `MainWindow::MainWindow()`
   - `DatabaseManager::instance().initDb()` — DB 초기화
   - `DatabaseManager::instance().executeDueSchedules()` — 대기 중인 예약 실행
   - 모든 UI 탭 갱신 (`refresh*` 함수들)

### 매수 버튼 클릭
1. `onBuyClicked()` 호출
2. `DatabaseManager::buyStock()` 실행
   - 트랜잭션 시작
   - account.cash 차감
   - holdings 추가 또는 평균단가 재계산
   - trades 기록
   - 트랜잭션 커밋
3. 성공 시: `refreshPortfolio()`, `refreshHistory()` 호출
4. 실패 시: 경고 메시지 (잔액 부족)

### 매매 예약 실행
- 앱 시작 시: `executeDueSchedules()`
- SQL 쿼리: `date <= 오늘 AND status = 'pending'`
- 각 예약에 대해 `buyStock()` 또는 `sellStock()` 실행
- 성공하면 status → "done"

### 메모 토글 (체크박스)
1. `onMemoItemChanged()` 호출
2. `DatabaseManager::toggleMemo(id)` 실행
3. SQL: `UPDATE memos SET done = NOT done WHERE id = :id`

---

## 주의사항

### 거래 트랜잭션
- `buyStock()`, `sellStock()`은 자동 트랜잭션 사용
- `QSqlDatabase::database().transaction()` / `.commit()`

### 로케일
- 숫자 포맷: `QLocale::Korean` 사용 (쉼표 구분)
- 날짜 포맷: "yyyy-MM-dd"

### 에러 처리
- DB 실패 → 메시지박스 경고
- SQL 에러는 `qWarning()` 출력

### 메모리 관리
- `MainWindow`: `ui` 포인터 동적할당 (소멸자에서 delete)
- `PortfolioModel`: MainWindow의 자식 위젯 (자동 정리)

---

## 테스트 체크리스트

- [ ] 앱 시작 → DB 생성 확인
- [ ] 매수 → 잔액 감소, 보유종목 추가
- [ ] 매도 → 잔액 증가, 수량 감소
- [ ] 보유 없을 때 매도 시도 → 실패 메시지
- [ ] 잔액 부족 시 매수 → 실패 메시지
- [ ] 예약 등록 → 테이블에 "pending" 상태로 표시
- [ ] 오늘 날짜 예약 → 앱 재시작 시 자동 실행
- [ ] 메모 추가 → 리스트에 표시
- [ ] 메모 체크박스 → 토글 동작
- [ ] 메모 삭제 → 리스트에서 제거
- [ ] 앱 재시작 후 모든 데이터 유지 확인

---

## 향후 개선 사항 (선택사항)

1. **API 연동**: 하드코딩 대신 실시간 시세 API (야후, 알파바 등)
2. **차트**: QCustomPlot 또는 Qt Charts 추가
3. **성과 분석**: 수익률 그래프, 자산 변화 차트
4. **분할 매수/매도**: 단계적 거래 기록
5. **다중 포트폴리오**: 여러 계좌 관리
6. **내보내기**: CSV/Excel 내보내기
