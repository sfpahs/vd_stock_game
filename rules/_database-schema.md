# 데이터베이스 스키마

## account 테이블

```sql
CREATE TABLE account (
    id       INTEGER PRIMARY KEY,
    cash     REAL,                    -- KRW 잔액
    usd_cash REAL DEFAULT 10000.0     -- USD 잔액
);
-- 초기값: (1, 50000000, 10000.0)
```

**역할**: 계좌 자산 관리

---

## holdings 테이블

```sql
CREATE TABLE holdings (
    symbol    TEXT PRIMARY KEY,       -- 종목 코드
    quantity  INTEGER,                -- 보유 수량
    avg_price REAL,                   -- 평균 단가 (KRW 기준)
    assetType TEXT                    -- 'STOCK' | 'COIN'
);
```

**역할**: 보유 종목 정보 (평가액 계산 기반)

---

## trades 테이블

```sql
CREATE TABLE trades (
    id        INTEGER PRIMARY KEY AUTOINCREMENT,
    symbol    TEXT,                   -- 종목 코드
    type      TEXT,                   -- 'BUY' | 'SELL'
    quantity  INTEGER,                -- 거래 수량
    price     REAL,                   -- 거래 가격 (KRW)
    timestamp TEXT,                   -- 'YYYY-MM-DD HH:MM:SS'
    assetType TEXT,                   -- 'STOCK' | 'COIN'
    orderType TEXT DEFAULT 'MARKET'   -- 'MARKET' | 'LIMIT'
);
```

**역할**: 거래 이력 (모든 매매 기록)

---

## schedules 테이블

```sql
CREATE TABLE schedules (
    id        INTEGER PRIMARY KEY AUTOINCREMENT,
    date      TEXT,                   -- 예약 날짜 'YYYY-MM-DD'
    symbol    TEXT,                   -- 주문 종목
    type      TEXT,                   -- 'BUY' | 'SELL'
    quantity  INTEGER,                -- 주문 수량
    price     REAL,                   -- 목표/주문 가격 (KRW)
    status    TEXT DEFAULT 'PENDING',  -- 'PENDING' | 'DONE' | 'ERROR'
    orderType TEXT DEFAULT 'MARKET',   -- 'MARKET' | 'LIMIT'
    assetType TEXT                    -- 'STOCK' | 'COIN'
);
```

**역할**: 예약 주문 관리
- MARKET: 앱 시작 시 자동 실행 (동일 날짜 이하)
- LIMIT: 매 1초 타이머에서 조건 검사 후 체결

---

## memos 테이블

```sql
CREATE TABLE memos (
    id      INTEGER PRIMARY KEY AUTOINCREMENT,
    content TEXT,                     -- 메모 내용
    done    INTEGER DEFAULT 0         -- 0=미완료, 1=완료
);
```

**역할**: 사용자 메모

---

## 주요 쿼리 패턴

### 보유 종목 조회
```sql
SELECT * FROM holdings WHERE quantity > 0;
```

### 예약 주문 조회
```sql
SELECT * FROM schedules 
WHERE status = 'PENDING' 
  AND (date <= '현재날짜' OR orderType = 'LIMIT');
```

### 거래 내역 조회
```sql
SELECT * FROM trades 
ORDER BY timestamp DESC 
LIMIT 100;
```

### 평가손익 계산
```
평가손익 = (현재가 - 평균단가) × 수량
수익률 = ((현재가 - 평균단가) / 평균단가) × 100%
```

---

## 마이그레이션

**기존 DB에 USD 추가**:
```sql
ALTER TABLE account ADD COLUMN usd_cash REAL DEFAULT 10000.0;
```

---

**생성**: DatabaseManager::initDb()
