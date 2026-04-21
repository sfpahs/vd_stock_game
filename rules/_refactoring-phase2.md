# Phase 2: 서비스 계층 (Week 2)

비즈니스 로직을 담당하는 서비스 클래스 구현.

---

## 2.1 StockManager 클래스

```cpp
class StockManager {
private:
    QMap<QString, Stock> m_stocks;
    
public:
    void initializeStocks() {
        m_stocks["005930"] = Stock("005930", "삼성전자", 78000, "STOCK");
        m_stocks["000660"] = Stock("000660", "SK하이닉스", 145000, "STOCK");
        m_stocks["BTC"] = Stock("BTC", "비트코인", 45000, "COIN");
        // ... 15개 종목
    }
    
    const Stock* getStock(const QString& symbol) const {
        auto it = m_stocks.find(symbol);
        return (it != m_stocks.end()) ? &it.value() : nullptr;
    }
    
    bool exists(const QString& symbol) const {
        return m_stocks.contains(symbol);
    }
    
    QString getAssetType(const QString& symbol) const {
        if (const Stock* stock = getStock(symbol)) {
            return stock->assetType();
        }
        return "";
    }
    
    void updatePrice(const QString& symbol, double price) {
        if (m_stocks.contains(symbol)) {
            m_stocks[symbol].setPrice(price);
        }
    }
};
```

---

## 2.2 TradeService 클래스

```cpp
class TradeService {
private:
    Account& m_account;
    StockManager& m_stockManager;
    QMap<QString, Holding>& m_holdings;
    double m_usdKrwRate;
    
public:
    TradeService(Account& acc, StockManager& sm,
                 QMap<QString, Holding>& holdings)
        : m_account(acc), m_stockManager(sm), m_holdings(holdings),
          m_usdKrwRate(1471.0) {}
    
    Trade buyStock(const QString& symbol, int qty, double price) {
        if (!canBuy(symbol, qty, price)) {
            throw runtime_error("매수 불가능");
        }
        
        QString assetType = m_stockManager.getAssetType(symbol);
        
        // 자산 차감
        if (assetType == "COIN") {
            double usdPrice = (price / m_usdKrwRate) * qty;
            m_account.subtractUsd(usdPrice);
        } else {
            m_account.subtractKrw(price * qty);
        }
        
        // 보유 종목 업데이트
        if (m_holdings.contains(symbol)) {
            m_holdings[symbol].addQuantity(qty, price);
        } else {
            m_holdings[symbol] = Holding(symbol, qty, price);
        }
        
        return Trade(0, symbol, Trade::TradeType::BUY, qty, price,
                    QDateTime::currentDateTime(), Trade::OrderType::MARKET);
    }
    
    bool canBuy(const QString& symbol, int qty, double price) const {
        QString assetType = m_stockManager.getAssetType(symbol);
        if (assetType == "COIN") {
            double usdPrice = (price / m_usdKrwRate) * qty;
            return m_account.getUsdCash() >= usdPrice;
        }
        return m_account.getKrwCash() >= (price * qty);
    }
};
```

---

## 2.3 ScheduleService 클래스

```cpp
class ScheduleService {
private:
    TradeService& m_tradeService;
    QVector<Schedule>& m_schedules;
    
public:
    ScheduleService(TradeService& ts, QVector<Schedule>& scheds)
        : m_tradeService(ts), m_schedules(scheds) {}
    
    void addLimitOrder(const QString& symbol, 
                       Schedule::OrderType type, int qty,
                       double price, const QDate& date) {
        Schedule s(m_schedules.size() + 1, date, symbol, type, qty,
                  price, Schedule::Status::PENDING,
                  Schedule::OrderType::LIMIT);
        m_schedules.append(s);
    }
    
    void checkAndExecuteLimitOrders(const StockManager& stockMgr) {
        for (Schedule& s : m_schedules) {
            if (s.status() != Schedule::Status::PENDING) continue;
            
            const Stock* stock = stockMgr.getStock(s.symbol());
            if (!stock) continue;
            
            if (shouldExecute(s, stock->price())) {
                try {
                    if (s.type() == Schedule::OrderType::BUY) {
                        m_tradeService.buyStock(s.symbol(),
                                               s.quantity(),
                                               stock->price());
                    }
                    s.setStatus(Schedule::Status::DONE);
                } catch (...) {
                    // 재시도 대기
                }
            }
        }
    }
    
private:
    bool shouldExecute(const Schedule& s, double currentPrice) const {
        if (s.type() == Schedule::OrderType::BUY) {
            return currentPrice <= s.price();
        } else {
            return currentPrice >= s.price();
        }
    }
};
```

---

## 2.4 ExchangeService 클래스

```cpp
class ExchangeService {
private:
    Account& m_account;
    double m_usdKrwRate;
    
public:
    ExchangeService(Account& acc)
        : m_account(acc), m_usdKrwRate(1471.0) {}
    
    void exchangeToUsd(double krwAmount) {
        m_account.subtractKrw(krwAmount);
        double usdAmount = krwAmount / m_usdKrwRate;
        m_account.addUsd(usdAmount);
    }
    
    void exchangeToKrw(double usdAmount) {
        m_account.subtractUsd(usdAmount);
        double krwAmount = usdAmount * m_usdKrwRate;
        m_account.addKrw(krwAmount);
    }
    
    void setUsdKrwRate(double rate) { m_usdKrwRate = rate; }
    double getUsdKrwRate() const { return m_usdKrwRate; }
};
```

---

## 구현 순서 (5일)

- **Day 1-2**: StockManager 구현 및 테스트
- **Day 3**: TradeService 구현 (buyStock, canBuy)
- **Day 4**: ScheduleService 구현
- **Day 5**: ExchangeService 구현 및 통합 테스트

---

**다음**: [Phase 3 - MainWindow & 예외처리](rules/_refactoring-phase3.md)
