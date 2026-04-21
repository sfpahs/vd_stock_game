# 파일별 역할 및 API

## stockdata.h

**역할**: 하드코딩된 15개 종목 데이터

```cpp
struct Stock {
    QString symbol;    // "005930"
    QString name;      // "삼성전자"
    double price;      // 78000
    QString assetType; // "STOCK" | "COIN"
};

// API
QVector<Stock>& stockList();          // 15개 종목
double stockPrice(symbol);             // 현재가
QString stockName(symbol);             // 종목명
QString stockAssetType(symbol);        // assetType
QString stockImagePath(symbol);        // 이미지 경로
QVector<double> getPriceHistory(symbol); // 가격 히스토리
double getUsdKrwRate();               // 환율
```

---

## database.h / database.cpp

**패턴**: 싱글톤

### 초기화
- `initDb()` — DB 연결, 테이블 생성, 초기 데이터 설정

### 계좌
- `getCash()` → KRW 잔액
- `getUsdCash()` → USD 잔액
- `getHoldings()` → 보유 종목 vector
- `getTrades()` → 거래 이력 vector
- `getSchedules()` → 예약 주문 vector

### 매매
- `buyStock(symbol, qty, price, orderType)` → bool
  - assetType에 따라 통화 자동 차감
  - holdings 추가/갱신, trades 기록
- `sellStock(symbol, qty, price, orderType)` → bool
  - assetType에 따라 통화 자동 증가
  - holdings 감소, trades 기록

### 환전
- `exchangeToUsd(krwAmount)` → bool (KRW → USD)
- `exchangeToKrw(usdAmount)` → bool (USD → KRW)

### 예약 & LIMIT
- `addSchedule(date, symbol, type, qty, price, orderType)` → bool
- `checkAndExecuteSchedules()` — 타이머에서 1초마다 호출
  - LIMIT 조건 검사: BUY(현재가 ≤ 목표), SELL(현재가 ≥ 목표)
  - 체결 성공 → status = "DONE"
  - 체결 실패 → 재시도 (status 유지)
- `executeDueSchedules()` — 앱 시작 시 호출
  - 날짜 <= 오늘 AND status='PENDING' AND orderType='MARKET'만 실행

### 메모
- `addMemo(content)` → bool
- `getMemos()` → vector
- `toggleMemo(id)` → bool (완료 토글)
- `deleteMemo(id)` → bool

---

## mainwindow.h / mainwindow.cpp

**역할**: UI 메인 윈도우, 슬롯 구현

### 슬롯
- `onBuyClicked()` — 매수 버튼
- `onSellClicked()` — 매도 버튼
- `onExchangeClicked()` — 환전 버튼
- `onSymbolChanged()` — 종목 선택 변경
- `onAutoTimeUpdate()` — 1초 타이머 (LIMIT 체크)
- `onSchedAddClicked()` — 예약 등록
- `onMemoClicked()` — 메모 추가

### 헬퍼
- `refreshPortfolio()` — 포트폴리오 테이블
- `refreshHistory()` — 거래 내역
- `refreshSchedules()` — 예약 주문
- `refreshBank()` — 은행 정보
- `refreshTicker()` — 환율 표시

---

## portfoliomodel.h / portfoliomodel.cpp

**패턴**: QAbstractTableModel

**컬럼** (7개):
1. 종목코드
2. 종목명
3. 보유수량
4. 평균단가
5. 현재가
6. 평가액
7. 수익률 (색상: +빨강, -파랑)

### API
- `refresh()` — holdings에서 데이터 로드
- `rowCount()`, `columnCount()`, `data()` — Model API

---

## main.cpp

**역할**: 진입점

```cpp
int main() {
    QApplication app(...);
    MainWindow w;
    w.show();
    return app.exec();
}
```

---

## mainwindow.ui

**역할**: Qt Designer 레이아웃 파일 (6탭)
- 탭1: 포트폴리오
- 탭2: 매매
- 탭3: 거래내역
- 탭4: 예약
- 탭5: 은행
- 탭6: 메모
