# 파일별 역할 및 API

## stockdata.h
```cpp
struct Stock {
    QString symbol;   // "005930"
    QString name;     // "삼성전자"
    double  price;    // 78000
};
```
**역할**: 하드코딩된 15개 종목 데이터
- `stockList()` — 15종목 static 벡터
- `stockPrice(symbol)` — 종목 현재가 조회
- `stockName(symbol)` — 종목명 조회
- `stockType(symbol)` → "STOCK" | "COIN" (자산 타입)

---

## database.h / database.cpp (DatabaseManager)

**패턴**: 싱글톤 (`DatabaseManager::instance()`)

### 초기화
- `initDb()` — DB 연결, 테이블 생성, 초기 현금(KRW 5000만원 + USD 10,000) 설정

### 계좌 / 포트폴리오
- `getCash()` → double  (KRW 잔액)
- `getUsdCash()` → double  (USD 잔액)
- `getHoldings()` → QVector<HoldingRow>  (보유 종목)
- `getTrades()` → QVector<TradeRow>  (거래 이력)
- `getUsdKrwRate()` → double  (실시간 환율, 기본값: 1471)

### 매매
- `buyStock(symbol, qty, price, orderType="MARKET")` → bool
  - assetType 자동 판단 (COIN → USD 차감 / STOCK → KRW 차감)
  - holding 추가/갱신 → trade 기록
  - 실패: 잔액 부족
- `sellStock(symbol, qty, price, orderType="MARKET")` → bool
  - assetType 자동 판단 (COIN → USD 증가 / STOCK → KRW 증가)
  - holding 수량 감소 → trade 기록
  - 실패: 보유 수량 부족

### 환전 (Exchange)
- `exchangeToUsd(krwAmount)` → bool
  - KRW → USD 환전 (환율 적용)
  - 실패: KRW 잔액 부족
- `exchangeToKrw(usdAmount)` → bool
  - USD → KRW 환전 (환율 적용)
  - 실패: USD 잔액 부족

### 주문 (Schedule & LIMIT)
- `addSchedule(date, symbol, type, qty, price, orderType="MARKET")` → bool
  - type: "BUY" | "SELL"
  - orderType: "MARKET" | "LIMIT"
  - LIMIT 주문: 가격 조건 만족 시 자동 체결
- `getSchedules()` → QVector<ScheduleRow>  (status = 'PENDING'만 조회)
- `checkAndExecuteSchedules()`
  - 타이머 기반 1초 단위 호출
  - LIMIT 주문 체결 조건 검사:
    - BUY LIMIT: 현재가 ≤ 목표가 → 체결
    - SELL LIMIT: 현재가 ≥ 목표가 → 체결
  - 체결 성공 시 status → "done"
  - 체결 실패 시 status 유지 (재시도)
- `executeDueSchedules()`
  - 앱 시작 시 호출
  - 오늘 날짜 이하 "pending" 예약 중 MARKET 주문만 즉시 실행

### 메모 (Memo)
- `addMemo(content)` → bool
- `getMemos()` → QVector<MemoRow>
- `toggleMemo(id)` → bool  (done 토글)
- `deleteMemo(id)` → bool

---

## portfoliomodel.h / portfoliomodel.cpp

**패턴**: `QAbstractTableModel` 서브클래스

**컬럼** (7개):
1. 종목코드
2. 종목명
3. 수량
4. 평균단가 (COIN: $ USD / STOCK: 원 KRW)
5. 현재가 (COIN: $ USD / STOCK: 원 KRW)
6. 평가손익 (계산: (현재가 - 평균단가) × 수량)
7. 수익률(%) (계산: (현재가 - 평균단가) / 평균단가 × 100)

**특징**:
- assetType 기반 통화 표시 (COIN → $ / STOCK → 원)
- 환율 실시간 적용 (USD 계산 시)
- 손익 양수: 빨강 (Qt::red)
- 손익 음수: 파랑 (Qt::blue)
- 우측 정렬 (숫자)

---

## mainwindow.h / mainwindow.cpp

**UI 위젯 접근**: `ui->위젯명` (ui_mainwindow.h 자동생성)

### 슬롯
- `onSymbolChanged(int)` — 종목 선택 → 현재가 + 총금액 갱신 (assetType별 통화 표시)
- `onPriceChanged(double)` — 가격 변경 → 총금액 갱신
- `onQtyChanged(int)` — 수량 변경 → 총금액 갱신
- `onBuyClicked()` — MARKET 매수 → 포트폴리오/거래내역 갱신
- `onSellClicked()` — MARKET 매도 → 포트폴리오/거래내역 갱신
- `onSchedAddClicked()` — MARKET/LIMIT 주문 등록
- `onExchangeClicked()` — 환전 다이얼로그 → KRW/USD 교환
- `onMemoAddClicked()` — 메모 추가
- `onMemoDeleteClicked()` — 메모 삭제
- `onMemoItemChanged(item)` — 메모 체크박스 → 완료 토글
- `onTabChanged(int)` — 탭 전환 시 전체 갱신
- `onAutoTimeUpdate()` — 타이머 1초 콜백 → LIMIT 주문 체결 검사

### 헬퍼 함수
- `setupStockCombo()` — trade/sched 콤보박스 초기화 (FX 타입 제외)
- `refreshPortfolio()` — 포트폴리오 테이블 + KRW/USD 잔액 표시
- `refreshBank()` — 은행탭: KRW/USD 잔액 표시
- `refreshHistory()` — 거래내역 테이블 갱신
- `refreshSchedules()` — 예약 테이블 갱신 (PENDING 주문만)
- `refreshMemos()` — 메모 목록 갱신
- `refreshTicker()` — 실시간 환율 표시 (상단 좌측)
- `updateTradeSummary()` — 매매탭: assetType별 통화 단위(KRW/USD) + 총금액 표시

---

## mainwindow.ui

**역할**: Qt Designer XML 레이아웃 (자세한 설명은 [UI 레이아웃](rules/_ui-layout.md) 참조)
