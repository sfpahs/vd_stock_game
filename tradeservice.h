#pragma once

#include <QString>
#include "database.h"
#include "stockdata.h"
#include "exceptions.h"

class TradeService {
public:
    static TradeService& instance() {
        static TradeService inst;
        return inst;
    }

    // MARKET 매수 — 실패 시 예외 발생
    void buy(const QString& symbol, int qty, double price,
             const QString& orderType = "MARKET")
    {
        if (!DatabaseManager::instance().buyStock(symbol, qty, price, orderType)) {
            if (orderType == "LIMIT") {
                throw OrderFailedException("LIMIT 주문 등록 실패");
            }
            QString assetType = stockAssetType(symbol);
            if (assetType == "COIN") {
                double cost      = (price / getUsdKrwRate()) * qty *
                                   (1.0 + DatabaseManager::BUY_FEE_RATE);
                double available = DatabaseManager::instance().getUsdCash();
                throw InsufficientBalanceException(
                    cost * getUsdKrwRate(), available * getUsdKrwRate());
            } else {
                double cost      = price * qty * (1.0 + DatabaseManager::BUY_FEE_RATE);
                double available = DatabaseManager::instance().getCash();
                throw InsufficientBalanceException(cost, available);
            }
        }
    }

    // MARKET 매도 — 실패 시 예외 발생
    void sell(const QString& symbol, int qty, double price,
              const QString& orderType = "MARKET")
    {
        if (!DatabaseManager::instance().sellStock(symbol, qty, price, orderType)) {
            if (orderType == "LIMIT") {
                throw OrderFailedException("LIMIT 주문 등록 실패");
            }
            throw InsufficientQuantityException(qty, 0);
        }
    }

    // 예약 주문 체결 — 체결된 주문 타입("BUY"/"SELL") 반환, 없으면 ""
    QString checkAndExecuteSchedules() {
        return DatabaseManager::instance().checkAndExecuteSchedules();
    }

private:
    TradeService() = default;
};
