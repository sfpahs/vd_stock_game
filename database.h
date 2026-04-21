#ifndef DATABASE_H
#define DATABASE_H

#include <QString>
#include <QVector>

struct HoldingRow {
    QString symbol;
    QString name;
    int     quantity;
    double  avgPrice;
    double  currentPrice;
    QString assetType;
};

struct TradeRow {
    int     id;
    QString symbol;
    QString name;
    QString type;
    int     quantity;
    double  price;
    double  fee;
    QString timestamp;
    QString assetType;
    QString orderType;
};

struct ScheduleRow {
    int     id;
    QString symbol;
    QString type;        // "BUY" | "SELL"
    int     quantity;
    double  targetPrice;
    QString status;      // "PENDING" | "DONE" | "CANCELLED"
    QString createdAt;
};

struct ProfitHistoryRow {
    int     id;
    QString nickname;
    double  profitRate;
    QString recordedAt;
};

class DatabaseManager
{
public:
    static DatabaseManager& instance();

    static constexpr double BUY_FEE_RATE  = 0.00015;   // 0.015%
    static constexpr double SELL_FEE_RATE = 0.00015;  // 0.015%
    static constexpr double SELL_TAX_RATE = 0.0015;   // 0.15%
    static constexpr double INITIAL_USD_CASH = 10000.0;

    bool initDb();

    double              getCash();
    double              getUsdCash();
    QVector<HoldingRow> getHoldings();
    QVector<TradeRow>   getTrades();
    int                 getTradeCount();
    QString             getStartDate();
    double              getInitialCash();

    bool buyStock (const QString& symbol, int qty, double price, const QString& orderType = "MARKET");
    bool sellStock(const QString& symbol, int qty, double price, const QString& orderType = "MARKET");

    bool exchangeToUsd(double krwAmount);
    bool exchangeToKrw(double usdAmount);

    double getTotalFees();
    double getTotalTax();

    bool addSchedule(const QString& symbol, const QString& type, int qty, double targetPrice);
    QVector<ScheduleRow> getSchedules(const QString& symbol = "");
    bool cancelSchedule(int id);
    bool modifySchedule(int id, int qty, double targetPrice);
    QString checkAndExecuteSchedules();

    QString getNickname();
    bool setNickname(const QString& nickname);
    bool resetCash();
    bool clearTrades();
    bool clearHoldings();
    bool resetAll();
    bool resetCashOnly();
    bool resetTrades();
    bool resetHoldings();

    // 대출 기능
    double getLoanAmount();
    bool borrow(double amount);
    bool repay(double amount);
    bool addLoanInterest();

    bool saveProfit(double profitRate);
    QVector<ProfitHistoryRow> getProfitHistory();

private:
    DatabaseManager() = default;
    bool createTables();
};

#endif // DATABASE_H
