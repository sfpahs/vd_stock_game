#pragma once

#include <QString>
#include <QDateTime>
#include <stdexcept>

// ── Account ───────────────────────────────────────────────────────────────────

class Account {
public:
    Account() = default;

    double krwCash() const { return m_krwCash; }
    double usdCash() const { return m_usdCash; }
    void   setKrwCash(double v) { m_krwCash = v; }
    void   setUsdCash(double v) { m_usdCash = v; }

    void addKrw(double amount)      { m_krwCash += amount; }
    void addUsd(double amount)      { m_usdCash += amount; }
    void subtractKrw(double amount) {
        if (m_krwCash < amount) throw std::runtime_error("KRW 잔액 부족");
        m_krwCash -= amount;
    }
    void subtractUsd(double amount) {
        if (m_usdCash < amount) throw std::runtime_error("USD 잔액 부족");
        m_usdCash -= amount;
    }

private:
    double m_krwCash = 50000000.0;
    double m_usdCash = 10000.0;
};

// ── Holding ───────────────────────────────────────────────────────────────────

class Holding {
public:
    Holding() = default;
    Holding(const QString& symbol, int qty, double avgPrice, const QString& assetType = "STOCK")
        : m_symbol(symbol), m_quantity(qty), m_avgPrice(avgPrice), m_assetType(assetType) {}

    QString symbol()    const { return m_symbol; }
    int     quantity()  const { return m_quantity; }
    double  avgPrice()  const { return m_avgPrice; }
    QString assetType() const { return m_assetType; }

    void addQuantity(int qty, double price) {
        double totalCost = m_avgPrice * m_quantity + price * qty;
        m_quantity += qty;
        m_avgPrice = (m_quantity > 0) ? totalCost / m_quantity : 0.0;
    }

    void subtractQuantity(int qty) {
        if (m_quantity < qty) throw std::runtime_error("수량 부족");
        m_quantity -= qty;
    }

    double evaluationAmount(double currentPrice) const {
        return (currentPrice - m_avgPrice) * m_quantity;
    }

    double profitRate(double currentPrice) const {
        if (m_avgPrice == 0.0) return 0.0;
        return (currentPrice - m_avgPrice) / m_avgPrice * 100.0;
    }

private:
    QString m_symbol;
    int     m_quantity  = 0;
    double  m_avgPrice  = 0.0;
    QString m_assetType = "STOCK";
};

// ── Trade ─────────────────────────────────────────────────────────────────────

class Trade {
public:
    enum class Type      { BUY, SELL };
    enum class OrderType { MARKET, LIMIT };

    Trade() = default;
    Trade(int id, const QString& symbol, Type type, int qty, double price,
          const QDateTime& ts, OrderType orderType)
        : m_id(id), m_symbol(symbol), m_type(type), m_quantity(qty),
          m_price(price), m_timestamp(ts), m_orderType(orderType) {}

    int       id()         const { return m_id; }
    QString   symbol()     const { return m_symbol; }
    Type      type()       const { return m_type; }
    int       quantity()   const { return m_quantity; }
    double    price()      const { return m_price; }
    QDateTime timestamp()  const { return m_timestamp; }
    OrderType orderType()  const { return m_orderType; }

    double  totalAmount() const { return m_price * m_quantity; }
    QString typeString()  const { return (m_type == Type::BUY) ? "BUY" : "SELL"; }

private:
    int       m_id        = 0;
    QString   m_symbol;
    Type      m_type      = Type::BUY;
    int       m_quantity  = 0;
    double    m_price     = 0.0;
    QDateTime m_timestamp;
    OrderType m_orderType = OrderType::MARKET;
};
