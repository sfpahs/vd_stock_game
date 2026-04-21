# 이중통화 (KRW/USD)

## 개요

**목표**: KRW 주식 + USD 암호화폐 동시 거래

- **KRW 자산**: 주식 (STOCK) — 50,000,000원
- **USD 자산**: 암호화폐 (COIN) — $10,000.00
- **환전**: 실시간 환율 적용

---

## 핵심 설계

### 저장 규칙

```
모든 가격은 KRW로 저장
├─ STOCK: 원 그대로 저장
├─ COIN: USD × 환율(1471) = KRW로 변환 후 저장
└─ 조회 시: assetType에 따라 USD로 재계산
```

### assetType 판단

```cpp
QString assetType = StockData::stockType(symbol);
// "STOCK" (주식) | "COIN" (암호화폐)

// 거래 시 자동 처리
if (assetType == "COIN") {
    usd_cash -= (price / getUsdKrwRate()) * qty;  // USD 차감
} else {
    cash -= price * qty;  // KRW 차감
}
```

---

## account 테이블

```sql
CREATE TABLE account (
    id       INTEGER PRIMARY KEY,
    cash     REAL,              -- KRW 잔액
    usd_cash REAL DEFAULT 10000.0  -- USD 잔액
);
```

---

## 매매 로직

### STOCK 매수 (KRW)

```cpp
bool DatabaseManager::buyStock(symbol, qty, price, "MARKET") {
    if (StockData::stockType(symbol) == "STOCK") {
        if (getKrwCash() < price * qty) return false;  // 잔액 부족
        
        // KRW 차감
        setKrwCash(getKrwCash() - price * qty);
        
        // holdings/trades 갱신
        // ...
        return true;
    }
}
```

### COIN 매수 (USD)

```cpp
bool DatabaseManager::buyStock(symbol, qty, price, "MARKET") {
    if (StockData::stockType(symbol) == "COIN") {
        double usdPrice = (price / getUsdKrwRate()) * qty;
        if (getUsdCash() < usdPrice) return false;  // USD 부족
        
        // USD 차감
        setUsdCash(getUsdCash() - usdPrice);
        
        // holdings: KRW로 변환 저장
        double krwPrice = price;  // 이미 환율 적용됨
        // ...
        return true;
    }
}
```

---

## 환전 메커니즘

```cpp
void DatabaseManager::exchangeKrwToUsd(double krwAmount) {
    if (getKrwCash() < krwAmount) return false;
    
    setKrwCash(getKrwCash() - krwAmount);
    double usdAmount = krwAmount / getUsdKrwRate();  // 1471
    setUsdCash(getUsdCash() + usdAmount);
}

void DatabaseManager::exchangeUsdToKrw(double usdAmount) {
    if (getUsdCash() < usdAmount) return false;
    
    setUsdCash(getUsdCash() - usdAmount);
    double krwAmount = usdAmount * getUsdKrwRate();
    setKrwCash(getKrwCash() + krwAmount);
}
```

---

## 포트폴리오 표시 규칙

```
| 종목명    | 수량  | 단가     | 평가액     | 손익  | 수익률 |
|----------|------|--------|----------|-------|-------|
| 삼성전자  | 100  | 70,000 | 7,000,000| 2M   | 40%   | (KRW)
| BTC      | 0.5  | $45K   | $22,500  | +$2K | 10%   | (USD)
```

---

## 환율 설정

```cpp
double getUsdKrwRate() {
    // DB에서 조회 또는 고정값
    return 1471.0;  // 1 USD = 1,471 KRW
}

void setUsdKrwRate(double rate) {
    // 환율 변경 (관리자 기능)
}
```

---

**관련**: [LIMIT 주문](rules/_limit-order.md)
