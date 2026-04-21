#ifndef STOCKDATA_H
#define STOCKDATA_H

#include <QString>
#include <QVector>
#include <QMap>
#include <QRandomGenerator>
#include <cmath>

struct Stock {
    QString symbol;
    QString name;
    double  basePrice;
    QString assetType;  // "STOCK", "COIN", or "FX"
};

inline const QVector<Stock>& stockList()
{
    static const QVector<Stock> list = {
        {"000660", "엘티전자", 192000, "STOCK"},
        {"035720", "나론에너지",      42000, "STOCK"},
        {"066950", "광원전자",       15000, "STOCK"},
        {"039730", "원선물산",       28000, "STOCK"},
        {"DOGE", "CVA코인",    50000, "COIN"},
        {"ETH", "블록체인",     3000, "COIN"},
        {"XRP", "물결코인",            3, "COIN"},
        {"MDS", "MDS코인",     1000, "COIN"},
        {"GSC", "GSC토큰",      500, "COIN"},
        {"USDKRW", "원/달러 환율",    1471, "FX"},
    };
    return list;
}

static QMap<QString, double> g_gamePrices;
static QMap<QString, QVector<double>> g_priceHistory;
static QMap<QString, double> g_lastChangeRate;

inline double getUsdKrwRate() {
    return g_gamePrices.value("USDKRW", 1471.0);
}

inline double coinPriceUsd(const QString& symbol) {
    return g_gamePrices.value(symbol, 0.0) / getUsdKrwRate();
}

inline QString stockAssetType(const QString& symbol) {
    for (const Stock& s : stockList())
        if (s.symbol == symbol) return s.assetType;
    return "STOCK";
}

// Box-Muller 변환으로 정규분포 난수 생성 (표준편차 기반)
inline double boxMullerGaussian(double stddev = 1.0)
{
    static bool hasSpare = false;
    static double spare;

    if (hasSpare) {
        hasSpare = false;
        return spare * stddev;
    }

    hasSpare = true;
    double u = QRandomGenerator::global()->generateDouble();
    double v = QRandomGenerator::global()->generateDouble();

    if (u < 1e-6) u = 1e-6;
    double mag = std::sqrt(-2.0 * std::log(u));
    spare = mag * std::sin(2.0 * M_PI * v);

    return mag * std::cos(2.0 * M_PI * v) * stddev;
}

inline void initGamePrices()
{
    if (g_gamePrices.isEmpty()) {
        for (const Stock& s : stockList()) {
            g_gamePrices[s.symbol] = s.basePrice;
            g_priceHistory[s.symbol].append(s.basePrice);
            g_lastChangeRate[s.symbol] = 0.0;
        }
    }
}

inline void updateGamePrices()
{
    initGamePrices();
    for (auto it = g_gamePrices.begin(); it != g_gamePrices.end(); ++it) {
        const QString& symbol = it.key();
        double currentPrice = it.value();

        // 자산 타입 확인 (STOCK, COIN, or FX)
        QString assetType = "STOCK";
        double basePrice = 0.0;
        for (const Stock& s : stockList()) {
            if (s.symbol == symbol) {
                assetType = s.assetType;
                basePrice = s.basePrice;
                break;
            }
        }

        double logChangeRate;
        double probSameDirection;
        double lastRate = g_lastChangeRate[symbol];

        if (assetType == "FX") {
            // === 환율 강제 횡보 모델 ===
            // 변동성 최소화 + 극강 평균회귀 (강제 범위 제한)
            double stddev = 0.001;  // 0.1% 변동성 (최소)
            double logRandomChange = boxMullerGaussian(stddev);

            // 강제 회귀: basePrice ±10원 범위 유지
            double deviation = (currentPrice - basePrice) / basePrice;
            double logMeanReversionForce = -2.0 * deviation;  // -200% 극강 회귀 (강제 회귀)
            logChangeRate = logRandomChange + logMeanReversionForce;

            // 극도로 엄격한 범위 제한: ±0.2% (약 ±3원)
            if (logChangeRate > 0.002) logChangeRate = 0.002;
            if (logChangeRate < -0.002) logChangeRate = -0.002;

            probSameDirection = 0.50;

        } else if (assetType == "COIN") {
            // === Phase 2 + Phase 3: 코인 가격 개선 ===
            double randRange = QRandomGenerator::global()->generateDouble();
            double upProbability = (symbol == "MDS") ? 0.70 : 0.50;
            double logMagnitude;

            if (randRange < upProbability) {
                // 상승: 10% ~ 25% → 로그 환산
                double randomChange = 0.10 + ((randRange / upProbability) * 0.15);
                logMagnitude = std::log(1.0 + randomChange);
            } else {
                // 하락: -10% ~ -25% → 로그 환산
                double randomChange = -(0.10 + (((randRange - upProbability) / (1.0 - upProbability)) * 0.15));
                logMagnitude = std::log(1.0 + randomChange);
            }

            // === Phase 3: 평균회귀 스케일링 개선 ===
            // basePrice 기준으로 정규화 (기존: currentPrice 기준 비대칭)
            double reversionStrength = (symbol == "MDS") ? 0.009 : 0.015;  // 3% → 0.9% (MDS)
            double logMeanReversionForce = reversionStrength * std::log(basePrice / currentPrice);
            logChangeRate = logMagnitude + logMeanReversionForce;

            // MDS: 추세 강화
            probSameDirection = (symbol == "MDS") ? 0.60 : 0.35;

        } else {
            // === 주식 ===
            // 기존 구조 유지하되, Box-Muller는 로그 스케일로 해석
            logChangeRate = boxMullerGaussian(0.07);
            probSameDirection = 0.65;

            // 광원전자: 상향 바이어스 + 추세 강화
            if (symbol == "066950") {
                logChangeRate += 0.005;
                probSameDirection = 0.75;
            }
        }

        // === Phase 1: 추세 로직 개선 ===
        // 기존: changeRate 계산 후 부호를 강제로 뒤집음 (확률 왜곡)
        // 개선: 방향을 먼저 정하고 크기는 별도 (확률 유지)

        double direction;  // +1 or -1
        if (lastRate == 0.0) {
            // 초기 방향: 50% 확률
            direction = (QRandomGenerator::global()->generateDouble() < 0.5) ? +1.0 : -1.0;
        } else {
            // 추세 확률 기반 방향 결정
            bool sameDirection = (QRandomGenerator::global()->generateDouble() < probSameDirection);
            bool lastWasPositive = (lastRate > 0.0);

            // sameDirection == lastWasPositive → +1
            // sameDirection != lastWasPositive → -1
            direction = (sameDirection == lastWasPositive) ? +1.0 : -1.0;
        }

        // 크기는 이미 계산된 logChangeRate의 절댓값
        double magnitude = std::abs(logChangeRate);
        double finalLogChangeRate = direction * magnitude;

        g_lastChangeRate[symbol] = finalLogChangeRate;

        // === Phase 2: 로그 수익률로 대칭성 확보 ===
        // 기존: newPrice = currentPrice * (1.0 + changeRate)
        // 개선: newPrice = currentPrice * exp(logChangeRate)
        double newPrice = currentPrice * std::exp(finalLogChangeRate);

        // 가격 범위 검증 (극단값 방지)
        if (newPrice <= 0.0) newPrice = currentPrice * 0.95;
        if (newPrice > currentPrice * 2.0) newPrice = currentPrice * 2.0;

        it.value() = newPrice;
        g_priceHistory[symbol].append(newPrice);
    }
}

inline QVector<double> getPriceHistory(const QString& symbol)
{
    initGamePrices();
    return g_priceHistory.value(symbol, QVector<double>());
}

// 1분 단위 캔들 데이터 구조
struct CandleData {
    double open;
    double high;
    double low;
    double close;
};

// 10초(10개 데이터)마다 캔들 데이터 생성
inline QVector<CandleData> getCandleData(const QString& symbol)
{
    initGamePrices();
    const QVector<double>& history = g_priceHistory.value(symbol, QVector<double>());
    QVector<CandleData> candles;

    if (history.isEmpty()) return candles;

    // 10개씩 묶어서 10초 캔들 생성
    for (int i = 0; i < history.size(); i += 10) {
        int end = qMin(i + 10, history.size());
        CandleData candle;

        candle.open = history[i];
        candle.close = history[end - 1];
        candle.high = *std::max_element(history.begin() + i, history.begin() + end);
        candle.low = *std::min_element(history.begin() + i, history.begin() + end);

        candles.append(candle);
    }

    return candles;
}

inline double stockPrice(const QString& symbol)
{
    initGamePrices();
    if (g_gamePrices.contains(symbol))
        return g_gamePrices[symbol];
    return 0.0;
}

inline QString stockName(const QString& symbol)
{
    for (const Stock& s : stockList())
        if (s.symbol == symbol) return s.name;
    return symbol;
}

inline QString stockImagePath(const QString& symbol)
{
    static const QMap<QString, QString> imageMap = {
        {"000660", "images/lt-electronics.png"},
        {"035720", "images/naron-energy.png"},
        {"066950", "images/gwangyeon-electronics.png"},
        {"039730", "images/wonseol-futures.png"},
        {"DOGE", "images/cva-coin.png"},
        {"ETH", "images/blockchain.png"},
        {"XRP", "images/ripple-coin.png"},
        {"MDS", "images/mds-coin.png"},
        {"GSC", "images/gsc-token.png"},
        {"USDKRW", "images/usd-krw.png"},
    };
    return imageMap.value(symbol, "");
}

#endif // STOCKDATA_H
