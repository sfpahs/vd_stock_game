# Phase 3: MainWindow & 예외처리 (Week 3)

UI 계층 리팩토링 및 예외 처리 강화.

---

## 3.1 MainWindow 의존성 주입

```cpp
class MainWindow : public QMainWindow {
private:
    Account* m_account;
    StockManager* m_stockManager;
    TradeService* m_tradeService;
    ScheduleService* m_scheduleService;
    ExchangeService* m_exchangeService;
    
public:
    MainWindow(Account* account, StockManager* stockManager,
               TradeService* tradeService, ScheduleService* scheduleService,
               ExchangeService* exchangeService, QWidget* parent = nullptr)
        : QMainWindow(parent), m_account(account),
          m_stockManager(stockManager), m_tradeService(tradeService),
          m_scheduleService(scheduleService),
          m_exchangeService(exchangeService) {
        // UI 초기화
    }
};
```

---

## 3.2 사용자 정의 예외

```cpp
class TradeException : public exception {
protected:
    string m_message;
    
public:
    explicit TradeException(const string& msg) : m_message(msg) {}
    const char* what() const noexcept override {
        return m_message.c_str();
    }
};

class InsufficientBalanceException : public TradeException {
public:
    InsufficientBalanceException(double required, double available)
        : TradeException("잔액 부족: " + to_string(required) +
                        "원 필요, " + to_string(available) + "원 보유") {}
};

class InsufficientQuantityException : public TradeException {
public:
    InsufficientQuantityException(int required, int available)
        : TradeException("수량 부족: " + to_string(required) +
                        "주 필요, " + to_string(available) + "주 보유") {}
};
```

---

## 3.3 슬롯 단순화 & 예외 처리

```cpp
void MainWindow::onBuyClicked() {
    try {
        QString symbol = ui->symbol_combo->currentText();
        int qty = ui->qty_spinbox->value();
        double price = ui->price_input->value();
        
        // TradeService 위임
        Trade trade = m_tradeService->buyStock(symbol, qty, price);
        
        // UI 갱신
        refreshPortfolio();
        refreshHistory();
        showToast("매수 완료", true);
        
    } catch (const InsufficientBalanceException& e) {
        QMessageBox::warning(this, "잔액 부족", QString::fromStdString(e.what()));
        
    } catch (const InsufficientQuantityException& e) {
        QMessageBox::warning(this, "수량 부족", QString::fromStdString(e.what()));
        
    } catch (const exception& e) {
        QMessageBox::critical(this, "오류", QString::fromStdString(e.what()));
    }
}
```

---

## 3.4 LIMIT 주문 타이머 통합

```cpp
// MainWindow 생성자 또는 setupUIElements()에서
void MainWindow::setupLimitOrderTimer() {
    QTimer* limitTimer = new QTimer(this);
    connect(limitTimer, &QTimer::timeout, this, [this]() {
        m_scheduleService->checkAndExecuteLimitOrders(*m_stockManager);
        refreshPendingOrders();
    });
    limitTimer->start(1000);  // 1초마다 체크
}
```

---

## 3.5 환전 슬롯 예시

```cpp
void MainWindow::onExchangeClicked() {
    try {
        double amount = QInputDialog::getDouble(this, "환전",
                                               "환전할 금액 (KRW):");
        
        if (amount <= 0) return;
        
        m_exchangeService->exchangeToUsd(amount);
        
        refreshBank();
        showToast("환전 완료", true);
        
    } catch (const InsufficientBalanceException& e) {
        QMessageBox::warning(this, "잔액 부족",
                           QString::fromStdString(e.what()));
    } catch (const exception& e) {
        QMessageBox::critical(this, "오류",
                            QString::fromStdString(e.what()));
    }
}
```

---

## 구현 순서 (5일)

- **Day 1**: 예외 클래스 정의
- **Day 2-3**: MainWindow DI 구현 (생성자 수정)
- **Day 4**: 각 슬롯에 try-catch 추가
- **Day 5**: 빌드 및 기능 테스트

---

## 객체 다이어그램

```
DatabaseManager (싱글톤)
  ├─ Account
  ├─ StockManager
  ├─ TradeService (Account, StockManager 의존)
  ├─ ScheduleService (TradeService 의존)
  └─ ExchangeService (Account 의존)
    ↓ (의존성 주입)
MainWindow
  ├─ onBuyClicked() → m_tradeService->buyStock()
  ├─ onExchangeClicked() → m_exchangeService->exchange*()
  └─ refreshPortfolio() (기존 헬퍼 유지)
```

---

**완료**: 3주 OOP 리팩토링 전체 계획 끝
