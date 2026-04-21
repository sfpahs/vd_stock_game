# LIMIT 주문 자동 체결 메커니즘

## 개요

**목표**: 가격 조건 기반 자동 매매 (조건 만족 시 1초 내 체결)

- **BUY LIMIT**: 현재가 ≤ 목표가 → 자동 매수
- **SELL LIMIT**: 현재가 ≥ 목표가 → 자동 매도
- **체크 주기**: 매 1초

---

## 아키텍처

### 타이머 기반 검사

```
MainWindow::MainWindow() {
    m_priceTimer = new QTimer(this);
    connect(m_priceTimer, SIGNAL(timeout()), 
            this, SLOT(onAutoTimeUpdate()));
    m_priceTimer->start(1000);  // 1초 주기
}

void MainWindow::onAutoTimeUpdate() {
    DatabaseManager::instance().checkAndExecuteSchedules();
    refreshSchedules();
    refreshPortfolio();
}
```

### 체결 조건 검사

```cpp
void DatabaseManager::checkAndExecuteSchedules() {
    QVector<ScheduleRow> pendingOrders = getSchedules();  // status='PENDING'
    
    foreach (const ScheduleRow& order : pendingOrders) {
        if (order.orderType != "LIMIT") continue;  // LIMIT만 검사
        
        double currentPrice = StockData::stockPrice(order.symbol);
        bool shouldExecute = false;
        
        // 체결 조건
        if (order.type == "BUY" && currentPrice <= order.price) {
            shouldExecute = true;  // 목표가 이하
        } else if (order.type == "SELL" && currentPrice >= order.price) {
            shouldExecute = true;  // 목표가 이상
        }
        
        if (shouldExecute) {
            // 현재가로 MARKET 주문 체결
            bool success = false;
            if (order.type == "BUY") {
                success = buyStock(order.symbol, order.quantity, 
                                 currentPrice, "MARKET");
            } else {
                success = sellStock(order.symbol, order.quantity, 
                                  currentPrice, "MARKET");
            }
            
            // 체결 성공 시 status 업데이트
            if (success) {
                updateScheduleStatus(order.id, "DONE");
            }
            // 실패 시: status 유지 (다음 타이밍 재시도)
        }
    }
}
```

---

## schedules 테이블 구조

```sql
CREATE TABLE schedules (
  id        INTEGER PRIMARY KEY AUTOINCREMENT,
  date      TEXT,      -- 예약 날짜
  symbol    TEXT,      -- 종목
  type      TEXT,      -- 'BUY' | 'SELL'
  quantity  INTEGER,   -- 주문 수량
  price     REAL,      -- LIMIT: 목표가 / MARKET: 참고가
  status    TEXT DEFAULT 'PENDING',  -- 'PENDING' | 'DONE'
  orderType TEXT DEFAULT 'MARKET',   -- 'MARKET' | 'LIMIT'
  assetType TEXT      -- 'STOCK' | 'COIN'
);
```

---

## 주문 타입별 동작

### MARKET 주문 (즉시 실행)

#### 등록 시
```cpp
void MainWindow::onSchedAddClicked() {
    QDate schedDate = ui->sched_date->date();
    QString symbol = ui->symbol_combo->currentText();
    QString type = ui->type_combo->currentText();  // BUY/SELL
    int qty = ui->qty_spinbox->value();
    double price = ui->price_input->value();
    QString orderType = ui->ordertype_combo->currentText();
    
    if (orderType == "MARKET") {
        // MARKET: 즉시 또는 앱 재시작 시 실행
        db->addSchedule(schedDate, symbol, type, qty, price, "MARKET");
    }
}
```

#### 실행 방식
- **앱 시작 시**: `executeDueSchedules()` → 오늘 날짜 이하 MARKET만 즉시 실행
- **가격 입력**: 매매탭에서 수동으로 MARKET 주문 (`onBuyClicked()` / `onSellClicked()`)

```cpp
// 앱 시작
void MainWindow::MainWindow() {
    DatabaseManager::instance().initDb();
    DatabaseManager::instance().executeDueSchedules();  // MARKET만
    // ...
}

// executeDueSchedules 구현
void DatabaseManager::executeDueSchedules() {
    QVector<ScheduleRow> orders = getSchedulesByCondition(
        "date <= TODAY AND status = 'PENDING' AND orderType = 'MARKET'"
    );
    
    foreach (const ScheduleRow& order : orders) {
        bool success = false;
        if (order.type == "BUY") {
            success = buyStock(order.symbol, order.quantity, order.price, "MARKET");
        } else {
            success = sellStock(order.symbol, order.quantity, order.price, "MARKET");
        }
        
        if (success) {
            updateScheduleStatus(order.id, "DONE");
        }
    }
}
```

### LIMIT 주문 (조건 체결)

#### 등록 시
```cpp
void MainWindow::onSchedAddClicked() {
    // ... (위와 동일)
    
    if (orderType == "LIMIT") {
        // LIMIT: 목표가 저장, 타이머에서 체크
        db->addSchedule(schedDate, symbol, type, qty, price, "LIMIT");
        // status: "PENDING" → 타이머가 체크
    }
}
```

#### 실행 방식 (매 1초 타이머 콜백)
```cpp
void DatabaseManager::checkAndExecuteSchedules() {
    // 1. PENDING 주문 조회
    QVector<ScheduleRow> pendingOrders = getSchedules();
    
    // 2. LIMIT만 필터링
    for (const auto& order : pendingOrders) {
        if (order.orderType != "LIMIT") continue;
        
        // 3. 체결 조건 검사
        double currentPrice = StockData::stockPrice(order.symbol);
        bool shouldExecute = false;
        
        if (order.type == "BUY" && currentPrice <= order.price) {
            shouldExecute = true;
        } else if (order.type == "SELL" && currentPrice >= order.price) {
            shouldExecute = true;
        }
        
        // 4. 체결 실행
        if (shouldExecute) {
            bool success = (order.type == "BUY")
                ? buyStock(order.symbol, order.quantity, currentPrice, "MARKET")
                : sellStock(order.symbol, order.quantity, currentPrice, "MARKET");
            
            if (success) {
                updateScheduleStatus(order.id, "DONE");
            }
        }
    }
}
```

---

## 체결 조건 상세

### BUY LIMIT
```
사용자: "삼성전자 70,000원에 100주 매수해줘"
등록: addSchedule(today, "005930", "BUY", 100, 70000, "LIMIT")

타이머 1초마다 검사:
  현재가 = 72,000 → 조건 불만족 (72,000 > 70,000)
  현재가 = 70,000 → 조건 만족! (70,000 <= 70,000) → 체결
  현재가 = 68,000 → 조건 만족! (68,000 <= 70,000) → 이미 체결됨
```

### SELL LIMIT
```
사용자: "비트코인 50,000달러에 0.5개 매도해줘"
등록: addSchedule(today, "BTC", "SELL", 0.5, 50000, "LIMIT")

타이머 1초마다 검사:
  현재가 = 45,000 → 조건 불만족 (45,000 < 50,000)
  현재가 = 50,000 → 조건 만족! (50,000 >= 50,000) → 체결
  현재가 = 52,000 → 조건 만족! (52,000 >= 50,000) → 이미 체결됨
```

---

## 체결 실패 및 재시도

### 실패 시나리오
```
LIMIT 주문: "삼성전자 70,000원 매수 100주"
조건 만족: 현재가 70,000 ≤ 70,000 ✓

buyStock() 실행
  ↓
  KRW 잔액 부족 (필요: 7,000,000원, 보유: 5,000,000원)
  ↓
  실패 반환

status: "PENDING" 유지 (DONE으로 변경 안됨)
다음 타이밍: 1초 후 재시도

잔액 충전 후:
  현재가 = 70,000 → 조건 여전히 만족
  buyStock() 성공 → status = "DONE"
```

### 재시도 메커니즘
```cpp
if (success) {
    updateScheduleStatus(order.id, "DONE");  // 성공
} else {
    // status 유지 (기본: PENDING)
    // 다음 타이밍에 자동 재시도
}
```

---

## 상태 전이 다이어그램

```
MARKET 주문:
  PENDING ---체결/앱재시작---> DONE
    ↑                          |
    └──────실패 시 유지────────┘

LIMIT 주문:
  PENDING ---타이머 1초마다 체크---> (조건 만족?) 
    ↑                                    |
    ├─ NO  ──유지────────────────────────┘
    │
    └─ YES ──buyStock/sellStock()
              │
              ├─ 성공 → status = "DONE"
              │
              └─ 실패 → status 유지 (재시도)
```

---

## 조회 범위

### getSchedules()
```cpp
QVector<ScheduleRow> DatabaseManager::getSchedules() {
    QSqlQuery query;
    query.prepare("SELECT * FROM schedules WHERE status = :status");
    query.addBindValue("PENDING");
    query.exec();
    // PENDING만 조회 → 이미 체결된 DONE은 표시 안함
}
```

### checkAndExecuteSchedules()
```cpp
// 시간 제약 없음 (매 1초 모든 PENDING LIMIT 검사)
// 단, 오늘 날짜 이하만 체크 (향후 개선 가능)
```

---

## 특이사항

### 이중통화 처리
```cpp
void DatabaseManager::checkAndExecuteSchedules() {
    for (const auto& order : pendingOrders) {
        // assetType 자동 판단
        QString assetType = StockData::stockType(order.symbol);
        
        // COIN: USD 잔액 차감 / STOCK: KRW 잔액 차감
        // buyStock/sellStock에서 자동 처리
    }
}
```

### 가격 정보 소스
- `StockData::stockPrice(symbol)` → 하드코딩 가격
- 실제 실시간 시세는 별도 API 연동 필요
- 현재: 매매탭에서 사용자가 입력한 가격 기준

### 타이밍 오차
- ±1000ms 범위 내 (QTimer 정확도)
- 극도로 높은 정확도 필요시 고정밀 타이머 사용 고려
