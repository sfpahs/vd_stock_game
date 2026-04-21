#pragma once

// StockManager 싱글톤이 모든 가격 상태를 관리합니다.
// 아래 inline 함수들은 하위 호환성을 위한 래퍼입니다.
#include "stockmanager.h"

// Stock = StockItem (기존 코드 호환)
using Stock = StockItem;

inline const QVector<StockItem>& stockList()
{
    return StockManager::instance().getStockList();
}

inline double stockPrice(const QString& symbol)
{
    return StockManager::instance().getPrice(symbol);
}

inline QString stockName(const QString& symbol)
{
    return StockManager::instance().getName(symbol);
}

inline QString stockAssetType(const QString& symbol)
{
    return StockManager::instance().getAssetType(symbol);
}

inline QString stockImagePath(const QString& symbol)
{
    return StockManager::instance().getImagePath(symbol);
}

inline double getUsdKrwRate()
{
    return StockManager::instance().getUsdKrwRate();
}

inline double coinPriceUsd(const QString& symbol)
{
    return StockManager::instance().getCoinUsdPrice(symbol);
}

inline void initGamePrices()
{
    StockManager::instance().init();
}

inline void updateGamePrices()
{
    StockManager::instance().update();
}

inline QVector<double> getPriceHistory(const QString& symbol)
{
    return StockManager::instance().getPriceHistory(symbol);
}

inline QVector<CandleData> getCandleData(const QString& symbol)
{
    return StockManager::instance().getCandleData(symbol);
}
