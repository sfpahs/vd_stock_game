#pragma once

#include <QString>
#include <QVector>
#include <QMap>

struct StockItem {
    QString symbol;
    QString name;
    double  basePrice;
    QString assetType;  // "STOCK" | "COIN" | "FX"
};

struct CandleData {
    double open;
    double high;
    double low;
    double close;
};

class StockManager {
public:
    static StockManager& instance();

    void init();
    void update();

    double  getPrice(const QString& symbol) const;
    double  getUsdKrwRate() const;
    double  getCoinUsdPrice(const QString& symbol) const;
    double  getLastChangeRate(const QString& symbol) const;

    QVector<double>    getPriceHistory(const QString& symbol) const;
    QVector<CandleData> getCandleData(const QString& symbol) const;

    QString getName(const QString& symbol) const;
    QString getAssetType(const QString& symbol) const;
    QString getImagePath(const QString& symbol) const;
    const QVector<StockItem>& getStockList() const;

private:
    StockManager();
    double boxMullerGaussian(double stddev);

    bool   m_initialized  = false;
    bool   m_bmHasSpare   = false;
    double m_bmSpare      = 0.0;

    QVector<StockItem>         m_stockList;
    QMap<QString, double>      m_gamePrices;
    QMap<QString, QVector<double>> m_priceHistory;
    QMap<QString, double>      m_lastChangeRate;
};
