# 이중통화 기능 (KRW/USD)

## 개요

**목표**: KRW(원) 기반 주식 거래와 USD($) 기반 암호화폐 거래를 동시에 지원

- **KRW 자산**: 주식 (STOCK) — 50,000,000원
- **USD 자산**: 암호화폐 (COIN) — $10,000.00
- **환전**: 실시간 환율 적용

---

## 핵심 설계

### 저장 규칙
```
모든 가격은 KRW 기준으로 holdings/trades에 저장
├─ STOCK 가격: 원 그대로 저장
├─ COIN 가격: USD × 환율 = KRW로 변환 후 저장
└─ 조회 시: assetType 기반 USD로 재계산 표시
```

### assetType 자동 판단
```cpp
QString assetType = StockData::stockType(symbol);
// → "STOCK" | "COIN"

// 매매 시
if (assetType == "COIN") {
    usd_cash -= (price / getUsdKrwRate()) * qty;  // USD 차감
} else {
    cash -= price * qty;  // KRW 차감
}
```

---

## 구현 상세

### Database 계층

#### account 테이블
```sql
CREATE TABLE account (
  id       INTEGER PRIMARY KEY,
  cash     REAL,              -- KRW 잔액
  usd_cash REAL DEFAULT 10000.0  -- USD 잔액
);
```

#### buyStock 함수
```cpp
bool DatabaseManager::buyStock(
    const QString& symbol, 
    int qty, 
    double price, 
    const QString& orderType = "MARKET"
) {
    // 1. assetType 자동 판단
    QString assetType = StockData::stockType(symbol);
    
    // 2. 통화별 차감
    if (assetType == "COIN") {
        double usdPrice = price / getUsdKrwRate();
        usd_cash -= usdPrice * qty;  // USD 차감
    } else {
        cash -= price * qty;  // KRW 차감
    }
    
    // 3. holdings 갱신 (KRW 기준)
    holdings.add(symbol, qty, price);
    
    // 4. trades 기록
    trades.insert(symbol, type="BUY", qty, price, assetType, orderType);
}
```

#### exchangeToUsd / exchangeToKrw
```cpp
bool DatabaseManager::exchangeToUsd(double krwAmount) {
    // KRW → USD 환전
    double usdAmount = krwAmount / getUsdKrwRate();
    
    // 트랜잭션
    cash -= krwAmount;        // KRW 감소
    usd_cash += usdAmount;    // USD 증가
    
    return commit();
}

bool DatabaseManager::exchangeToKrw(double usdAmount) {
    // USD → KRW 환전
    double krwAmount = usdAmount * getUsdKrwRate();
    
    // 트랜잭션
    usd_cash -= usdAmount;    // USD 감소
    cash += krwAmount;        // KRW 증가
    
    return commit();
}
```

#### getUsdKrwRate
```cpp
double DatabaseManager::getUsdKrwRate() {
    // 실시간 환율 (기본값: 1471)
    // 향후: 실제 API 연동 가능
    return 1471.0;
}
```

---

### UI 계층

#### updateTradeSummary (매매탭)
```cpp
void MainWindow::updateTradeSummary() {
    QString symbol = ui->symbol_combo->currentText();
    QString assetType = StockData::stockType(symbol);
    
    double qty = ui->qty_spinbox->value();
    double price = ui->price_input->value();
    double total = qty * price;
    
    // assetType별 통화 표시
    QString unit = (assetType == "COIN") ? "$" : "원";
    ui->total_amount_label->setText(
        QString("%1 %2").arg(total).arg(unit)
    );
    
    // 현재가 표시
    QString priceUnit = (assetType == "COIN") ? "$" : "원";
    ui->current_price_label->setText(
        QString("%1 %2").arg(price).arg(priceUnit)
    );
}
```

#### refreshPortfolio
```cpp
void MainWindow::refreshPortfolio() {
    double cash = db->getCash();        // KRW
    double usdCash = db->getUsdCash();  // USD
    
    // 이중 통화 잔액 표시
    ui->cash_label->setText(
        QString("현금(KRW): %1 원 | 현금(USD): $%2")
        .arg(formatNumber(cash))
        .arg(QString::number(usdCash, 'f', 2))
    );
    
    // 포트폴리오 모델 갱신
    m_portfolioModel->refresh();
}
```

#### refreshBank (은행탭)
```cpp
void MainWindow::refreshBank() {
    double cash = db->getCash();
    double usdCash = db->getUsdCash();
    double rate = db->getUsdKrwRate();
    
    ui->cash_label->setText(formatNumber(cash) + " 원");
    ui->usd_label->setText("$" + QString::number(usdCash, 'f', 2));
    ui->rate_label->setText(QString::number(rate) + " 원/$");
}
```

#### refreshTicker (환율 표시)
```cpp
void MainWindow::refreshTicker() {
    double rate = db->getUsdKrwRate();
    ui->ticker_label->setText(
        QString("💱 USD/KRW: %1 | 🥇 ...")
        .arg(formatNumber(rate))
    );
}
```

#### portfoliomodel.cpp (표시)
```cpp
QVariant PortfolioModel::data(const QModelIndex& index, int role) const {
    HoldingRow row = m_holdings[index.row()];
    QString assetType = row.assetType;
    
    // 평균단가 & 현재가 표시
    if (assetType == "COIN") {
        // USD로 표시
        double usdPrice = row.price / getUsdKrwRate();
        return QString("$%1").arg(usdPrice, 0, 'f', 2);
    } else {
        // KRW로 표시
        return QString("%1 원").arg(formatNumber(row.price));
    }
}
```

---

## 사용 예시

### 주식 매수 (KRW)
```
종목: 삼성전자 (005930)
assetType: STOCK
가격: 70,000원
수량: 100주
→ KRW 차감: 7,000,000원
```

### 암호화폐 매수 (USD → KRW 변환 저장)
```
종목: BTC (비트코인)
assetType: COIN
가격: $45,000
수량: 0.5
USD 차감: $22,500 = 22,500 × 1471 = 33,097,500 KRW
→ USD 차감: $22,500
→ holdings에 33,097,500 KRW로 저장
```

### 환전
```
사용자: "100만원을 달러로 환전해주세요"
→ KRW: 50,000,000 - 1,000,000 = 49,000,000
→ USD: 10,000 + (1,000,000 / 1471) ≈ 10,679.88
```

### 포트폴리오 표시
```
종목    | 자산유형 | 수량  | 평균단가    | 현재가      | 평가손익
───────┼────────┼──────┼──────────┼──────────┼──────────
삼성전자 | STOCK  | 100  | 70,000원 | 78,000원 | +800,000원
BTC    | COIN   | 0.5  | $45,000  | $41,000  | -$2,000 (-2%)
```

---

## 주의사항

### 소수점 처리
- KRW: 정수 (QSpinBox)
- USD: 소수점 2자리 (QDoubleSpinBox, decimals=2)
- 환율 계산 시 부동소수점 오차 고려

### 환율 갱신
- **주기**: refreshTicker() → 1초 단위
- **기본값**: 1471 (하드코딩)
- **향후**: 실제 API 연동 권장

### 거래 기록
- holdings/trades: 모든 가격 KRW로 저장
- 조회 시 assetType 기반 USD 재계산
- 보고서/분석 시 일관된 기준점 유지

### 마이그레이션
- 기존 DB: `ALTER TABLE account ADD COLUMN usd_cash REAL DEFAULT 10000.0`
- 자동 실행 (initDb에서 처리)
