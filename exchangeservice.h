#pragma once

#include "database.h"
#include "exceptions.h"

class ExchangeService {
public:
    static ExchangeService& instance() {
        static ExchangeService inst;
        return inst;
    }

    // KRW → USD 환전 (netAmount: 수수료 차감 후 금액)
    void exchangeToUsd(double netKrwAmount) {
        if (!DatabaseManager::instance().exchangeToUsd(netKrwAmount)) {
            double available = DatabaseManager::instance().getCash();
            throw InsufficientBalanceException(netKrwAmount, available);
        }
    }

    // USD → KRW 환전 (netAmount: 수수료 차감 후 금액)
    void exchangeToKrw(double netUsdAmount) {
        if (!DatabaseManager::instance().exchangeToKrw(netUsdAmount)) {
            double available = DatabaseManager::instance().getUsdCash();
            throw InsufficientBalanceException(netUsdAmount * 1471.0, available * 1471.0);
        }
    }

private:
    ExchangeService() = default;
};
