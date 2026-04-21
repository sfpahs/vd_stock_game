# 객체지향 리팩토링 (OOP 설계)

## 현재 상태 분석

### 문제점
```cpp
// 현재: 정적 함수 기반
class DatabaseManager {
    static bool buyStock(...);
    static QString getAssetType(...);
};

class StockData {
    static QVector<Stock> stockList();
    static double stockPrice(...);
};

// 문제
// 1. 데이터와 행동이 분리됨
// 2. 상태 관리 어려움
// 3. 확장성 제한
// 4. 테스트 불편
```

### 개선 방향
```
DatabaseManager (싱글톤)
  ├─ Account (계좌 데이터 객체)
  ├─ StockManager (종목 관리)
  ├─ TradeService (매매 비즈니스 로직)
  ├─ ScheduleService (예약 주문)
  ├─ ExchangeService (환전)
  └─ PriceService (시세 관리)
```

---

## 리팩토링 계획

### Phase 1: 데이터 객체화 (기본 구조)

#### 1.1 Account 클래스
```cpp
// 현재
double cash;
double usd_cash;

// 개선
class Account {
private:
    double m_krwCash;
    double m_usdCash;
    const double INITIAL_KRW = 50000000;
    const double INITIAL_USD = 10000.0;
    
public:
    Account();
    
    // Getter
    double getKrwCash() const { return m_krwCash; }
    double getUsdCash() const { return m_usdCash; }
    
    // Setter
    void setKrwCash(double amount) { m_krwCash = amount; }
    void setUsdCash(double amount) { m_usdCash = amount; }
    
    // 연산
    void addKrw(double amount) { m_krwCash += amount; }
    void subtractKrw(double amount) { 
        if (m_krwCash < amount) throw std::runtime_error("KRW 잔액 부족");
        m_krwCash -= amount; 
    }
    
    void addUsd(double amount) { m_usdCash += amount; }
    void subtractUsd(double amount) {
        if (m_usdCash < amount) throw std::runtime_error("USD 잔액 부족");
        m_usdCash -= amount;
    }
    
    void reset() {
        m_krwCash = INITIAL_KRW;
        m_usdCash = INITIAL_USD;
    }
};
```

#### 1.2 Stock 클래스 (구조체 → 클래스)
```cpp
// 현재 (구조체)
struct Stock {
    QString symbol;
    QString name;
    double price;
};

// 개선 (클래스)
class Stock {
private:
    QString m_symbol;
    QString m_name;
    double m_price;
    QString m_assetType;  // "STOCK" | "COIN"
    
public:
    Stock(const QString& symbol, const QString& name, 
          double price, const QString& assetType)
        : m_symbol(symbol), m_name(name), m_price(price), 
          m_assetType(assetType) {}
    
    // Getter
    QString symbol() const { return m_symbol; }
    QString name() const { return m_name; }
    double price() const { return m_price; }
    QString assetType() const { return m_assetType; }
    
    // Setter
    void setPrice(double price) { m_price = price; }
    
    // 비즈니스 로직
    bool isCoin() const { return m_assetType == "COIN"; }
    bool isStock() const { return m_assetType == "STOCK"; }
    QString displayPrice(double usdKrwRate = 1471.0) const {
        if (isCoin()) {
            return QString("$%1").arg(m_price / usdKrwRate, 0, 'f', 2);
        } else {
            return QString("%1원").arg(formatNumber(m_price));
        }
    }
};
```

#### 1.3 Holding 클래스
```cpp
// 현재
struct HoldingRow {
    QString symbol;
    int quantity;
    double avg_price;
};

// 개선
class Holding {
private:
    QString m_symbol;
    int m_quantity;
    double m_avgPrice;
    QString m_assetType;
    
public:
    Holding(const QString& symbol, int qty, double avgPrice, 
            const QString& assetType)
        : m_symbol(symbol), m_quantity(qty), m_avgPrice(avgPrice),
          m_assetType(assetType) {}
    
    // Getter
    QString symbol() const { return m_symbol; }
    int quantity() const { return m_quantity; }
    double avgPrice() const { return m_avgPrice; }
    QString assetType() const { return m_assetType; }
    
    // 연산
    void addQuantity(int qty, double price) {
        // 평균단가 재계산
        double totalCost = m_avgPrice * m_quantity + price * qty;
        m_quantity += qty;
        m_avgPrice = totalCost / m_quantity;
    }
    
    void subtractQuantity(int qty) {
        if (m_quantity < qty) throw std::runtime_error("수량 부족");
        m_quantity -= qty;
    }
    
    double evaluationAmount(double currentPrice) const {
        return (currentPrice - m_avgPrice) * m_quantity;
    }
    
    double profitRate(double currentPrice) const {
        if (m_avgPrice == 0) return 0;
        return (currentPrice - m_avgPrice) / m_avgPrice * 100;
    }
    
    bool isEmpty() const { return m_quantity == 0; }
};
```

#### 1.4 Trade 클래스
```cpp
// 현재
struct TradeRow {
    int id;
    QString symbol;
    QString type;  // "BUY" | "SELL"
    int quantity;
    double price;
    QString timestamp;
};

// 개선
class Trade {
public:
    enum class TradeType { BUY, SELL };
    enum class OrderType { MARKET, LIMIT };
    
private:
    int m_id;
    QString m_symbol;
    TradeType m_type;
    int m_quantity;
    double m_price;
    QDateTime m_timestamp;
    QString m_assetType;
    OrderType m_orderType;
    
public:
    Trade(int id, const QString& symbol, TradeType type, 
          int qty, double price, const QDateTime& timestamp,
          const QString& assetType, OrderType orderType = OrderType::MARKET)
        : m_id(id), m_symbol(symbol), m_type(type), m_quantity(qty),
          m_price(price), m_timestamp(timestamp), m_assetType(assetType),
          m_orderType(orderType) {}
    
    // Getter
    int id() const { return m_id; }
    QString symbol() const { return m_symbol; }
    TradeType type() const { return m_type; }
    int quantity() const { return m_quantity; }
    double price() const { return m_price; }
    QDateTime timestamp() const { return m_timestamp; }
    QString assetType() const { return m_assetType; }
    OrderType orderType() const { return m_orderType; }
    
    // 비즈니스 로직
    QString typeString() const {
        return (m_type == TradeType::BUY) ? "BUY" : "SELL";
    }
    
    double totalAmount() const { return m_price * m_quantity; }
    
    bool isCoin() const { return m_assetType == "COIN"; }
};
```

---

### Phase 2: 서비스 계층 (비즈니스 로직)

#### 2.1 StockManager 클래스
```cpp
class StockManager {
private:
    QMap<QString, Stock> m_stocks;
    
public:
    StockManager();
    
    // 초기화
    void initializeStocks();
    
    // 조회
    const Stock* getStock(const QString& symbol) const;
    QVector<Stock> getAllStocks() const;
    QVector<Stock> getStocksByType(const QString& assetType) const;
    
    // 관리
    void addStock(const Stock& stock);
    void updatePrice(const QString& symbol, double price);
    
    // 검증
    bool exists(const QString& symbol) const;
    QString getAssetType(const QString& symbol) const;
};

// 구현
StockManager::StockManager() {
    initializeStocks();
}

void StockManager::initializeStocks() {
    m_stocks["005930"] = Stock("005930", "삼성전자", 78000, "STOCK");
    m_stocks["000660"] = Stock("000660", "SK하이닉스", 145000, "STOCK");
    m_stocks["BTC"] = Stock("BTC", "비트코인", 45000, "COIN");
    m_stocks["ETH"] = Stock("ETH", "이더리움", 2500, "COIN");
    // ... 15개 종목
}

const Stock* StockManager::getStock(const QString& symbol) const {
    if (m_stocks.contains(symbol)) {
        return &m_stocks[symbol];  // 위험 (포인터)
    }
    return nullptr;
}
```

#### 2.2 TradeService 클래스
```cpp
class TradeService {
private:
    Account& m_account;
    StockManager& m_stockManager;
    QMap<QString, Holding>& m_holdings;
    QVector<Trade>& m_trades;
    double m_usdKrwRate;
    
public:
    TradeService(Account& account, StockManager& stockManager,
                 QMap<QString, Holding>& holdings, QVector<Trade>& trades)
        : m_account(account), m_stockManager(stockManager),
          m_holdings(holdings), m_trades(trades), m_usdKrwRate(1471.0) {}
    
    // MARKET 매매
    Trade buyStock(const QString& symbol, int qty, double price);
    Trade sellStock(const QString& symbol, int qty, double price);
    
    // 검증
    bool canBuy(const QString& symbol, int qty, double price) const;
    bool canSell(const QString& symbol, int qty) const;
    
    // 환율
    void setUsdKrwRate(double rate) { m_usdKrwRate = rate; }
    double getUsdKrwRate() const { return m_usdKrwRate; }
    
private:
    void processTradeAmount(Trade::TradeType type, double totalAmount);
};

// 구현
Trade TradeService::buyStock(const QString& symbol, int qty, double price) {
    // 1. 검증
    if (!canBuy(symbol, qty, price)) {
        throw std::runtime_error("매수 불가능");
    }
    
    // 2. 계좌 업데이트
    QString assetType = m_stockManager.getAssetType(symbol);
    if (assetType == "COIN") {
        double usdPrice = (price / m_usdKrwRate) * qty;
        m_account.subtractUsd(usdPrice);
    } else {
        m_account.subtractKrw(price * qty);
    }
    
    // 3. 보유 종목 업데이트
    if (m_holdings.contains(symbol)) {
        m_holdings[symbol].addQuantity(qty, price);
    } else {
        m_holdings[symbol] = Holding(symbol, qty, price, assetType);
    }
    
    // 4. 거래 기록
    Trade trade(m_trades.size() + 1, symbol, Trade::TradeType::BUY,
                qty, price, QDateTime::currentDateTime(), assetType,
                Trade::OrderType::MARKET);
    m_trades.append(trade);
    
    return trade;
}

bool TradeService::canBuy(const QString& symbol, int qty, double price) const {
    QString assetType = m_stockManager.getAssetType(symbol);
    
    if (assetType == "COIN") {
        double usdPrice = (price / m_usdKrwRate) * qty;
        return m_account.getUsdCash() >= usdPrice;
    } else {
        return m_account.getKrwCash() >= (price * qty);
    }
}
```

#### 2.3 ScheduleService 클래스
```cpp
class ScheduleService {
private:
    TradeService& m_tradeService;
    QVector<Schedule>& m_schedules;
    QTimer m_limitOrderTimer;
    
public:
    ScheduleService(TradeService& tradeService, QVector<Schedule>& schedules);
    
    // 주문 등록
    Schedule addMarketOrder(const QString& symbol, Schedule::OrderType type,
                           int qty, double price, const QDate& date);
    Schedule addLimitOrder(const QString& symbol, Schedule::OrderType type,
                          int qty, double price, const QDate& date);
    
    // 실행
    void executeMarketOrders();  // 앱 시작 시
    void checkAndExecuteLimitOrders();  // 매 1초
    
    // 상태 조회
    QVector<Schedule> getPendingOrders() const;
    QVector<Schedule> getCompletedOrders() const;
    
private:
    void startLimitOrderTimer();
    void onTimerTimeout();
    bool shouldExecute(const Schedule& order, double currentPrice) const;
};

// 구현
Schedule ScheduleService::addLimitOrder(
    const QString& symbol, Schedule::OrderType type,
    int qty, double price, const QDate& date) {
    
    Schedule schedule(m_schedules.size() + 1, date, symbol, type,
                     qty, price, Schedule::Status::PENDING,
                     Schedule::OrderType::LIMIT);
    
    m_schedules.append(schedule);
    return schedule;
}

void ScheduleService::checkAndExecuteLimitOrders() {
    for (Schedule& schedule : m_schedules) {
        if (schedule.status() != Schedule::Status::PENDING) continue;
        if (schedule.orderType() != Schedule::OrderType::LIMIT) continue;
        
        // 현재가 조회
        double currentPrice = m_tradeService.getStockPrice(schedule.symbol());
        
        // 체결 조건 검사
        if (shouldExecute(schedule, currentPrice)) {
            try {
                if (schedule.type() == Schedule::OrderType::BUY) {
                    m_tradeService.buyStock(schedule.symbol(),
                                           schedule.quantity(),
                                           currentPrice);
                } else {
                    m_tradeService.sellStock(schedule.symbol(),
                                            schedule.quantity(),
                                            currentPrice);
                }
                schedule.setStatus(Schedule::Status::DONE);
            } catch (const std::exception& e) {
                // 실패 시 status 유지 (재시도)
            }
        }
    }
}

bool ScheduleService::shouldExecute(
    const Schedule& order, double currentPrice) const {
    
    if (order.type() == Schedule::OrderType::BUY) {
        return currentPrice <= order.price();  // BUY LIMIT
    } else {
        return currentPrice >= order.price();  // SELL LIMIT
    }
}
```

#### 2.4 ExchangeService 클래스
```cpp
class ExchangeService {
private:
    Account& m_account;
    double m_usdKrwRate;
    
public:
    ExchangeService(Account& account)
        : m_account(account), m_usdKrwRate(1471.0) {}
    
    // 환전
    void exchangeToUsd(double krwAmount);
    void exchangeToKrw(double usdAmount);
    
    // 환율
    void setUsdKrwRate(double rate) { m_usdKrwRate = rate; }
    double getUsdKrwRate() const { return m_usdKrwRate; }
    
    // 계산
    double krwToUsd(double krw) const { return krw / m_usdKrwRate; }
    double usdToKrw(double usd) const { return usd * m_usdKrwRate; }
};

// 구현
void ExchangeService::exchangeToUsd(double krwAmount) {
    m_account.subtractKrw(krwAmount);  // 예외 발생 가능
    
    double usdAmount = krwToUsd(krwAmount);
    m_account.addUsd(usdAmount);
}

void ExchangeService::exchangeToKrw(double usdAmount) {
    m_account.subtractUsd(usdAmount);
    
    double krwAmount = usdToKrw(usdAmount);
    m_account.addKrw(krwAmount);
}
```

---

### Phase 3: MainWindow 리팩토링

#### 3.1 의존성 주입 (DI)
```cpp
// 현재
class MainWindow : public QMainWindow {
    DatabaseManager db;  // 강결합
    PortfolioModel model;
};

// 개선
class MainWindow : public QMainWindow {
private:
    // 서비스 계층 (느슨한 결합)
    Account* m_account;
    StockManager* m_stockManager;
    TradeService* m_tradeService;
    ScheduleService* m_scheduleService;
    ExchangeService* m_exchangeService;
    
public:
    MainWindow(Account* account, StockManager* stockManager,
               TradeService* tradeService, ScheduleService* scheduleService,
               ExchangeService* exchangeService);
    
    // ... slots, helpers
};
```

#### 3.2 슬롯 단순화
```cpp
// 현재
void MainWindow::onBuyClicked() {
    // DB 접근, 검증, 갱신 모두 혼재
    DatabaseManager::buyStock(...);
    refreshPortfolio();
}

// 개선
void MainWindow::onBuyClicked() {
    try {
        QString symbol = ui->symbol_combo->currentText();
        int qty = ui->qty_spinbox->value();
        double price = ui->price_input->value();
        
        // TradeService에 위임
        Trade trade = m_tradeService->buyStock(symbol, qty, price);
        
        // UI 갱신
        refreshPortfolio();
        refreshHistory();
        refreshTicker();
        
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "오류", e.what());
    }
}
```

#### 3.3 구현 순서
- [ ] DatabaseManager 분해 (Account, StockManager 추출)
- [ ] MainWindow 생성자 수정 (서비스 객체 주입)
- [ ] 각 슬롯에 try-catch 추가
- [ ] 헬퍼 함수 그대로 유지 (refreshPortfolio 등)

---


## 예외 처리 추가

#### 4.1 사용자 정의 예외
```cpp
class TradeException : public std::exception {
private:
    std::string m_message;
    
public:
    explicit TradeException(const std::string& msg) : m_message(msg) {}
    
    const char* what() const noexcept override {
        return m_message.c_str();
    }
};

class InsufficientBalanceException : public TradeException {
public:
    InsufficientBalanceException(double required, double available)
        : TradeException(
            "잔액 부족: " + std::to_string(required) + "원 필요, " +
            std::to_string(available) + "원 보유"
        ) {}
};

class InsufficientQuantityException : public TradeException {
public:
    InsufficientQuantityException(int required, int available)
        : TradeException(
            "수량 부족: " + std::to_string(required) + "주 필요, " +
            std::to_string(available) + "주 보유"
        ) {}
};
```

#### 4.2 예외 처리 사용
```cpp
void MainWindow::onBuyClicked() {
    try {
        Trade trade = m_tradeService->buyStock(symbol, qty, price);
        
    } catch (const InsufficientBalanceException& e) {
        QMessageBox::warning(this, "잔액 부족", e.what());
        
    } catch (const InsufficientQuantityException& e) {
        QMessageBox::warning(this, "수량 부족", e.what());
        
    } catch (const TradeException& e) {
        QMessageBox::warning(this, "거래 오류", e.what());
        
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "시스템 오류", e.what());
    }
}
```

---

## 리팩토링 실행 순서

### Week 1: 데이터 객체화
- [ ] Day 1-2: Account, Stock, Holding, Trade 클래스 구현
- [ ] Day 3: StockManager 구현
- [ ] Day 4-5: 기존 코드에서 새 클래스로 전환

### Week 2: 서비스 계층
- [ ] Day 1-2: TradeService 구현
- [ ] Day 3: ScheduleService 구현
- [ ] Day 4: ExchangeService 구현
- [ ] Day 5: 전체 통합

### Week 3: UI 리팩토링
- [ ] Day 1-2: MainWindow 의존성 주입
- [ ] Day 3-4: 슬롯 단순화 및 예외 처리 추가
- [ ] Day 5: 빌드 및 기능 동작 확인

---

## 객체 다이어그램

```
┌─ DatabaseManager (싱글톤)
│  │
│  ├─ Account
│  │  └─ getKrwCash(), getUsdCash()
│  │
│  ├─ StockManager
│  │  └─ Stock[] (15개)
│  │     └─ symbol, name, price, assetType
│  │
│  ├─ TradeService
│  │  ├─ buyStock()
│  │  ├─ sellStock()
│  │  └─ canBuy(), canSell()
│  │
│  ├─ ScheduleService
│  │  ├─ addMarketOrder()
│  │  ├─ addLimitOrder()
│  │  ├─ executeMarketOrders()
│  │  └─ checkAndExecuteLimitOrders()
│  │
│  └─ ExchangeService
│     ├─ exchangeToUsd()
│     ├─ exchangeToKrw()
│     └─ getUsdKrwRate()
│
└─ MainWindow
   ├─ onBuyClicked()
   ├─ onSellClicked()
   ├─ onSchedAddClicked()
   ├─ onExchangeClicked()
   └─ refresh*() 헬퍼들
```

---

## 주요 이점

### 유지보수성 ↑
- 각 클래스가 단일 책임
- 변경 영향 범위 제한
- 버그 수정 시 해당 클래스만 수정

### 재사용성 ↑
- TradeService를 다른 UI에서도 사용 가능
- API 서버에서도 바로 활용
- 새 기능 추가 시 기존 클래스 재조합

### 확장성 ↑
- 새 주문 타입 추가 (STOP_LOSS, TRAILING_STOP)
- 새 자산 타입 추가 (옵션, 선물)
- 새 서비스 클래스 추가 용이

### 코드 품질 ↑
- 응집도 높음 (관련 기능이 한 클래스에)
- 결합도 낮음 (클래스 간 의존성 최소)
- 가독성 향상 (명확한 책임 분담)
