# 데이터베이스 스키마 및 구조

## 테이블 정의

### account
```sql
CREATE TABLE account (
  id       INTEGER PRIMARY KEY,
  cash     REAL,
  usd_cash REAL DEFAULT 10000.0
);
-- 초기값: (1, 50000000, 10000.0)
-- 마이그레이션: 기존 DB에 ALTER TABLE account ADD COLUMN usd_cash REAL DEFAULT 10000.0
```
**역할**: 계좌 자산 관리
- `cash`: KRW 잔액
- `usd_cash`: USD 잔액

---

### holdings
```sql
CREATE TABLE holdings (
  symbol    TEXT PRIMARY KEY,
  quantity  INTEGER,
  avg_price REAL,
  assetType TEXT  -- 'STOCK' | 'COIN'
);
```
**역할**: 보유 종목 정보
- `symbol`: 종목 코드 (예: "005930", "BTC")
- `quantity`: 보유 수량
- `avg_price`: 평균 단가 (모든 자산 KRW 기준 저장)
- `assetType`: 자산 유형 (자동 판단)

---

### trades
```sql
CREATE TABLE trades (
  id        INTEGER PRIMARY KEY AUTOINCREMENT,
  symbol    TEXT,
  type      TEXT,      -- 'BUY' | 'SELL'
  quantity  INTEGER,
  price     REAL,
  timestamp TEXT,      -- 'YYYY-MM-DD HH:MM:SS'
  assetType TEXT,      -- 'STOCK' | 'COIN'
  orderType TEXT DEFAULT 'MARKET'  -- 'MARKET' | 'LIMIT'
);
```
**역할**: 거래 이력 기록
- `symbol`: 거래 종목
- `type`: 매매 유형
- `quantity`: 거래 수량
- `price`: 거래 가격 (KRW 기준)
- `timestamp`: 거래 시각
- `assetType`: 자산 유형
- `orderType`: 주문 방식 (MARKET/LIMIT)

---

### schedules
```sql
CREATE TABLE schedules (
  id        INTEGER PRIMARY KEY AUTOINCREMENT,
  date      TEXT,      -- 'YYYY-MM-DD'
  symbol    TEXT,
  type      TEXT,      -- 'BUY' | 'SELL'
  quantity  INTEGER,
  price     REAL,
  status    TEXT DEFAULT 'PENDING',  -- 'PENDING' | 'DONE'
  orderType TEXT DEFAULT 'MARKET',   -- 'MARKET' | 'LIMIT'
  assetType TEXT      -- 'STOCK' | 'COIN'
);
```
**역할**: 예약 주문 관리
- `date`: 예약 날짜
- `symbol`: 주문 종목
- `type`: 매매 유형
- `quantity`: 주문 수량
- `price`: 주문 목표가
- `status`: 주문 상태
  - `PENDING`: 대기 중 (미체결 또는 실행 전)
  - `DONE`: 완료 (체결 또는 실행됨)
- `orderType`: 주문 방식
  - `MARKET`: 즉시 실행 (가격 조건 무관)
  - `LIMIT`: 조건 체결 (가격 조건 만족 시 자동 체결)
- `assetType`: 자산 유형

---

### memos
```sql
CREATE TABLE memos (
  id         INTEGER PRIMARY KEY AUTOINCREMENT,
  content    TEXT,
  done       INTEGER DEFAULT 0,  -- 0=false, 1=true
  created_at TEXT               -- 'YYYY-MM-DD HH:MM:SS'
);
```
**역할**: 메모/할일 관리
- `content`: 메모 내용
- `done`: 완료 여부 (체크박스)
- `created_at`: 작성 시각

---

## 데이터 흐름

### 자산 저장 규칙
- **모든 가격은 KRW 기준**으로 holdings/trades에 저장
- USD 기반 거래: `USD 가격 × 환율` → KRW로 변환 후 저장
- 조회 시: 화면에서만 USD로 표시 (계산 기반)

### assetType 판단
- `StockData::stockType(symbol)` 함수로 자동 판단
- COIN: 암호화폐 (USD 기반 거래)
- STOCK: 주식 (KRW 기반 거래)
- FX: 특수 타입 (거래 불가)

### 거래 트랜잭션
- `buyStock()`, `sellStock()` → 원자성 보장
- `exchangeToUsd()`, `exchangeToKrw()` → 트랜잭션
- SQL 실패 시 자동 rollback

---

## 조회 쿼리 패턴

### PENDING 주문 조회
```sql
SELECT * FROM schedules WHERE status = 'PENDING'
```

### 오늘 이하 MARKET 주문 (앱 시작 시 실행)
```sql
SELECT * FROM schedules 
WHERE date <= '오늘' AND status = 'PENDING' AND orderType = 'MARKET'
```

### 거래 이력 (최신순)
```sql
SELECT * FROM trades ORDER BY timestamp DESC
```

### 포트폴리오 (holdings + 현재가)
```sql
SELECT symbol, quantity, avg_price, assetType FROM holdings
-- 포트폴리오 모델에서 현재가 동적 계산
```

---

## 환율 및 현재가

### 환율 (USD/KRW)
- **기본값**: 1471
- **관리함수**: `DatabaseManager::getUsdKrwRate()`
- **갱신 주기**: 1초 (refreshTicker)
- **사용 예**:
  ```cpp
  double usdAmount = price / getUsdKrwRate();  // KRW → USD
  double krwAmount = usdAmount * getUsdKrwRate();  // USD → KRW
  ```

### 현재가 (StockData)
- **저장 위치**: stockdata.h (하드코딩)
- **조회함수**: `StockData::stockPrice(symbol)`
- **업데이트**: 매매탭/예약탭에서 사용자 입력 기반
- **사용 예**:
  ```cpp
  double currentPrice = StockData::stockPrice("005930");  // 정적 가격
  double userPrice = ui->price_input->value();  // 동적 입력가
  ```
