#pragma once

#include <stdexcept>
#include <string>

// ── 기반 예외 ─────────────────────────────────────────────────────────────────

class TradeException : public std::exception {
protected:
    std::string m_message;
public:
    explicit TradeException(const std::string& msg) : m_message(msg) {}
    const char* what() const noexcept override { return m_message.c_str(); }
};

// ── 잔액 부족 ─────────────────────────────────────────────────────────────────

class InsufficientBalanceException : public TradeException {
public:
    InsufficientBalanceException(double required, double available)
        : TradeException("잔액 부족: 필요 " +
                         std::to_string(static_cast<long long>(required)) +
                         "원, 보유 " +
                         std::to_string(static_cast<long long>(available)) + "원") {}
};

// ── 수량 부족 ─────────────────────────────────────────────────────────────────

class InsufficientQuantityException : public TradeException {
public:
    InsufficientQuantityException(int required, int available)
        : TradeException("수량 부족: 필요 " + std::to_string(required) +
                         "주, 보유 " + std::to_string(available) + "주") {}
};

// ── 주문 실패 (기타) ──────────────────────────────────────────────────────────

class OrderFailedException : public TradeException {
public:
    explicit OrderFailedException(const std::string& reason = "")
        : TradeException(reason.empty() ? "주문 실패" : "주문 실패: " + reason) {}
};
