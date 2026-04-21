# Phase 1: 데이터 객체화 (Week 1)

데이터와 비즈니스 로직을 캡슐화하는 기본 클래스 구현.

---

## 1.1 Account 클래스

```cpp
class Account {
private:
    double m_krwCash = 50000000;
    double m_usdCash = 10000.0;
    
public:
    // Getter
    double getKrwCash() const { return m_krwCash; }
    double getUsdCash() const { return m_usdCash; }
    
    // 연산
    void addKrw(double amount) { m_krwCash += amount; }
    void subtractKrw(double amount) {
        if (m_krwCash < amount) throw runtime_error("KRW 잔액 부족");
        m_krwCash -= amount;
    }
    
    void addUsd(double amount) { m_usdCash += amount; }
    void subtractUsd(double amount) {
        if (m_usdCash < amount) throw runtime_error("USD 잔액 부족");
        m_usdCash -= amount;
    }
};
```

---

## 1.2 Stock 클래스

```cpp
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
};
```

---

## 1.3 Holding 클래스

```cpp
class Holding {
private:
    QString m_symbol;
    int m_quantity = 0;
    double m_avgPrice = 0;
    
public:
    Holding(const QString& symbol, int qty, double avgPrice)
        : m_symbol(symbol), m_quantity(qty), m_avgPrice(avgPrice) {}
    
    QString symbol() const { return m_symbol; }
    int quantity() const { return m_quantity; }
    double avgPrice() const { return m_avgPrice; }
    
    // 평균단가 재계산
    void addQuantity(int qty, double price) {
        double totalCost = m_avgPrice * m_quantity + price * qty;
        m_quantity += qty;
        m_avgPrice = totalCost / m_quantity;
    }
    
    void subtractQuantity(int qty) {
        if (m_quantity < qty) throw runtime_error("수량 부족");
        m_quantity -= qty;
    }
    
    double evaluationAmount(double currentPrice) const {
        return (currentPrice - m_avgPrice) * m_quantity;
    }
    
    double profitRate(double currentPrice) const {
        if (m_avgPrice == 0) return 0;
        return (currentPrice - m_avgPrice) / m_avgPrice * 100;
    }
};
```

---

## 1.4 Trade 클래스

```cpp
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
    OrderType m_orderType;
    
public:
    Trade(int id, const QString& symbol, TradeType type,
          int qty, double price, const QDateTime& ts, OrderType ot)
        : m_id(id), m_symbol(symbol), m_type(type),
          m_quantity(qty), m_price(price), m_timestamp(ts),
          m_orderType(ot) {}
    
    int id() const { return m_id; }
    QString symbol() const { return m_symbol; }
    TradeType type() const { return m_type; }
    int quantity() const { return m_quantity; }
    double price() const { return m_price; }
    
    double totalAmount() const { return m_price * m_quantity; }
    QString typeString() const {
        return (m_type == TradeType::BUY) ? "BUY" : "SELL";
    }
};
```

---

## 구현 순서 (5일)

- **Day 1-2**: Account, Stock, Holding, Trade 클래스 생성
- **Day 3**: header 파일들 (.h) 작성
- **Day 4-5**: 기존 코드에서 새 클래스로 전환
  - `DatabaseManager`에서 Account 생성
  - `stockList()` → `StockManager` 이동
  - `holdings` → `QMap<QString, Holding>` 변경

---

**다음**: [Phase 2 - 서비스 계층](rules/_refactoring-phase2.md)
