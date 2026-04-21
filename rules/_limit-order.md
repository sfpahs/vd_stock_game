# LIMIT 주문 자동 체결

## 개요

**목표**: 가격 조건 기반 자동 매매 (1초 내 체결)

- **BUY LIMIT**: 현재가 ≤ 목표가 → 자동 매수
- **SELL LIMIT**: 현재가 ≥ 목표가 → 자동 매도
- **체크 주기**: 매 1초

---

## 타이머 기반 검사

```cpp
// MainWindow 생성자
m_priceTimer = new QTimer(this);
connect(m_priceTimer, &QTimer::timeout, 
        this, &MainWindow::onAutoTimeUpdate);
m_priceTimer->start(1000);  // 1초마다

void MainWindow::onAutoTimeUpdate() {
    DatabaseManager::instance().checkAndExecuteSchedules();
    refreshSchedules();  // UI 갱신
    refreshPortfolio();
}
```

---

## 체결 조건 검사

```cpp
void DatabaseManager::checkAndExecuteSchedules() {
    // PENDING 상태의 LIMIT 주문만 조회
    QVector<ScheduleRow> pending = getSchedules("PENDING");
    
    for (const ScheduleRow& order : pending) {
        if (order.orderType != "LIMIT") continue;
        
        double currentPrice = StockData::stockPrice(order.symbol);
        bool shouldExecute = false;
        
        // 체결 조건
        if (order.type == "BUY" && currentPrice <= order.price) {
            shouldExecute = true;
        } else if (order.type == "SELL" && currentPrice >= order.price) {
            shouldExecute = true;
        }
        
        if (shouldExecute) {
            // 현재가로 MARKET 주문 실행
            bool success = (order.type == "BUY") ?
                buyStock(order.symbol, order.quantity, currentPrice, "MARKET") :
                sellStock(order.symbol, order.quantity, currentPrice, "MARKET");
            
            // 체결 성공 시만 status 변경
            if (success) {
                updateScheduleStatus(order.id, "DONE");
            }
            // 실패 (잔액부족 등) → PENDING 유지 (재시도)
        }
    }
}
```

---

## schedules 테이블

```sql
CREATE TABLE schedules (
    id        INTEGER PRIMARY KEY,
    date      TEXT,         -- 예약 날짜
    symbol    TEXT,         -- 종목명
    type      TEXT,         -- 'BUY' | 'SELL'
    quantity  INTEGER,      -- 수량
    price     REAL,         -- 목표 가격
    status    TEXT,         -- 'PENDING' | 'DONE' | 'ERROR'
    orderType TEXT          -- 'MARKET' | 'LIMIT'
);
```

---

## 재시도 로직

**문제**: 체결 시점에 잔액/수량이 부족하면?

**해결**: 
- 체결 실패 → status = 'PENDING' 유지
- 다음 1초 주기에 자동 재시도
- 예: KRW 부족 → 환전 후 ~ 체결 성공

**주의**: 무한 재시도 방지
- 타이머는 계속 실행
- 사용자가 "취소"하면 status = 'CANCELLED'

---

## 제약사항

❌ **미지원**:
- STOP_LOSS, TRAILING_STOP
- 시간 기반 체결 (특정 시간에만 체결)
- 부분 체결 (예: 100주 중 50주만)

✓ **지원**:
- 비동기 재시도
- 현재가 기반 즉시 체결
- DB에 영구 저장

---

**관련**: [이중통화 메커니즘](rules/_dual-currency.md)
