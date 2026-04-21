#include "stockmanager.h"

#include <QRandomGenerator>
#include <algorithm>
#include <cmath>

// ── 싱글톤 ────────────────────────────────────────────────────────────────────

StockManager& StockManager::instance()
{
    static StockManager inst;
    return inst;
}

StockManager::StockManager()
{
    m_stockList = {
        {"000660", "엘티전자",    192000, "STOCK"},
        {"035720", "나론에너지",   42000, "STOCK"},
        {"066950", "광원전자",     15000, "STOCK"},
        {"039730", "원선물산",     28000, "STOCK"},
        {"DOGE",   "CVA코인",     50000, "COIN"},
        {"ETH",    "블록체인",     3000, "COIN"},
        {"XRP",    "물결코인",        3, "COIN"},
        {"MDS",    "MDS코인",      1000, "COIN"},
        {"GSC",    "GSC토큰",       500, "COIN"},
        {"USDKRW", "원/달러 환율", 1471, "FX"},
    };
}

// ── 초기화 ────────────────────────────────────────────────────────────────────

void StockManager::init()
{
    if (m_initialized) return;
    m_initialized = true;

    for (const StockItem& s : m_stockList) {
        m_gamePrices[s.symbol]     = s.basePrice;
        m_priceHistory[s.symbol].append(s.basePrice);
        m_lastChangeRate[s.symbol] = 0.0;
    }
}

// ── Box-Muller ────────────────────────────────────────────────────────────────

double StockManager::boxMullerGaussian(double stddev)
{
    if (m_bmHasSpare) {
        m_bmHasSpare = false;
        return m_bmSpare * stddev;
    }

    m_bmHasSpare = true;
    double u = QRandomGenerator::global()->generateDouble();
    double v = QRandomGenerator::global()->generateDouble();
    if (u < 1e-6) u = 1e-6;

    double mag  = std::sqrt(-2.0 * std::log(u));
    m_bmSpare   = mag * std::sin(2.0 * M_PI * v);
    return mag * std::cos(2.0 * M_PI * v) * stddev;
}

// ── 가격 업데이트 ─────────────────────────────────────────────────────────────

void StockManager::update()
{
    init();

    for (auto it = m_gamePrices.begin(); it != m_gamePrices.end(); ++it) {
        const QString& symbol = it.key();
        double currentPrice   = it.value();

        QString assetType = "STOCK";
        double  basePrice = 0.0;
        for (const StockItem& s : m_stockList) {
            if (s.symbol == symbol) {
                assetType = s.assetType;
                basePrice = s.basePrice;
                break;
            }
        }

        double logChangeRate;
        double probSameDirection;
        double lastRate = m_lastChangeRate[symbol];

        if (assetType == "FX") {
            double logRandomChange       = boxMullerGaussian(0.001);
            double deviation             = (currentPrice - basePrice) / basePrice;
            double logMeanReversionForce = -2.0 * deviation;
            logChangeRate = logRandomChange + logMeanReversionForce;
            if (logChangeRate >  0.002) logChangeRate =  0.002;
            if (logChangeRate < -0.002) logChangeRate = -0.002;
            probSameDirection = 0.50;

        } else if (assetType == "COIN") {
            double randRange    = QRandomGenerator::global()->generateDouble();
            double upProbability = (symbol == "MDS") ? 0.70 : 0.50;
            double logMagnitude;
            if (randRange < upProbability) {
                double rc = 0.10 + ((randRange / upProbability) * 0.15);
                logMagnitude = std::log(1.0 + rc);
            } else {
                double rc = -(0.10 + (((randRange - upProbability) / (1.0 - upProbability)) * 0.15));
                logMagnitude = std::log(1.0 + rc);
            }
            double reversionStrength     = (symbol == "MDS") ? 0.009 : 0.015;
            double logMeanReversionForce = reversionStrength * std::log(basePrice / currentPrice);
            logChangeRate     = logMagnitude + logMeanReversionForce;
            probSameDirection = (symbol == "MDS") ? 0.60 : 0.35;

        } else {
            logChangeRate     = boxMullerGaussian(0.07);
            probSameDirection = 0.65;
            if (symbol == "066950") {
                logChangeRate += 0.005;
                probSameDirection = 0.75;
            }
        }

        double direction;
        if (lastRate == 0.0) {
            direction = (QRandomGenerator::global()->generateDouble() < 0.5) ? +1.0 : -1.0;
        } else {
            bool sameDir       = (QRandomGenerator::global()->generateDouble() < probSameDirection);
            bool lastPositive  = (lastRate > 0.0);
            direction          = (sameDir == lastPositive) ? +1.0 : -1.0;
        }

        double finalLogChangeRate = direction * std::abs(logChangeRate);
        m_lastChangeRate[symbol]  = finalLogChangeRate;

        double newPrice = currentPrice * std::exp(finalLogChangeRate);
        if (newPrice <= 0.0)                 newPrice = currentPrice * 0.95;
        if (newPrice > currentPrice * 2.0)   newPrice = currentPrice * 2.0;

        it.value() = newPrice;
        m_priceHistory[symbol].append(newPrice);
    }
}

// ── Getter ────────────────────────────────────────────────────────────────────

double StockManager::getPrice(const QString& symbol) const
{
    const_cast<StockManager*>(this)->init();
    return m_gamePrices.value(symbol, 0.0);
}

double StockManager::getUsdKrwRate() const
{
    return getPrice("USDKRW");
}

double StockManager::getCoinUsdPrice(const QString& symbol) const
{
    double rate = getUsdKrwRate();
    return (rate > 0) ? getPrice(symbol) / rate : 0.0;
}

double StockManager::getLastChangeRate(const QString& symbol) const
{
    return m_lastChangeRate.value(symbol, 0.0);
}

QVector<double> StockManager::getPriceHistory(const QString& symbol) const
{
    const_cast<StockManager*>(this)->init();
    return m_priceHistory.value(symbol, QVector<double>());
}

QVector<CandleData> StockManager::getCandleData(const QString& symbol) const
{
    const_cast<StockManager*>(this)->init();
    const QVector<double>& history = m_priceHistory.value(symbol, QVector<double>());
    QVector<CandleData> candles;
    if (history.isEmpty()) return candles;

    for (int i = 0; i < history.size(); i += 10) {
        int end = qMin(i + 10, history.size());
        CandleData c;
        c.open  = history[i];
        c.close = history[end - 1];
        c.high  = *std::max_element(history.begin() + i, history.begin() + end);
        c.low   = *std::min_element(history.begin() + i, history.begin() + end);
        candles.append(c);
    }
    return candles;
}

QString StockManager::getName(const QString& symbol) const
{
    for (const StockItem& s : m_stockList)
        if (s.symbol == symbol) return s.name;
    return symbol;
}

QString StockManager::getAssetType(const QString& symbol) const
{
    for (const StockItem& s : m_stockList)
        if (s.symbol == symbol) return s.assetType;
    return "STOCK";
}

QString StockManager::getImagePath(const QString& symbol) const
{
    static const QMap<QString, QString> imageMap = {
        {"000660", "images/lt-electronics.png"},
        {"035720", "images/naron-energy.png"},
        {"066950", "images/gwangyeon-electronics.png"},
        {"039730", "images/wonseol-futures.png"},
        {"DOGE",   "images/cva-coin.png"},
        {"ETH",    "images/blockchain.png"},
        {"XRP",    "images/ripple-coin.png"},
        {"MDS",    "images/mds-coin.png"},
        {"GSC",    "images/gsc-token.png"},
        {"USDKRW", "images/usd-krw.png"},
    };
    return imageMap.value(symbol, "");
}

const QVector<StockItem>& StockManager::getStockList() const
{
    return m_stockList;
}
