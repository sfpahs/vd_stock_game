# 주요 동작 흐름

## 앱 시작 흐름

```
main()
  ↓
MainWindow 생성
  ├─ DatabaseManager::instance().initDb()
  │  ├─ DB 연결 (stock_portfolio.db)
  │  ├─ 테이블 생성 (account, holdings, trades, schedules, memos)
  │  ├─ 초기 현금: KRW 50,000,000, USD $10,000
  │  └─ 마이그레이션: account에 usd_cash 컬럼 추가 (기존 DB)
  │
  ├─ DatabaseManager::instance().executeDueSchedules()
  │  ├─ SQL: date <= 오늘 AND status = 'PENDING' AND orderType = 'MARKET'
  │  ├─ 각 MARKET 주문 실행: buyStock() or sellStock()
  │  └─ 성공 시 status = "DONE"
  │
  ├─ m_priceTimer 시작 (1000ms 주기)
  │  └─ onAutoTimeUpdate() → checkAndExecuteSchedules() (LIMIT 체크)
  │
  └─ 모든 UI 탭 갱신
     ├─ refreshPortfolio() → KRW/USD 잔액, 포트폴리오 테이블
     ├─ refreshBank() → 은행 탭 정보
     ├─ refreshHistory() → 거래 내역
     ├─ refreshSchedules() → PENDING 주문
     ├─ refreshMemos() → 메모 목록
     └─ refreshTicker() → 환율 표시 (💱 USD/KRW: 1,471)
```

---

## MARKET 매수 흐름

```
매매탭 - "매수" 버튼 클릭
  ↓
onBuyClicked()
  ├─ symbol = ui->symbol_combo->currentText()
  ├─ qty = ui->qty_spinbox->value()
  ├─ price = ui->price_input->value() (사용자 입력)
  ├─ assetType = StockData::stockType(symbol)
  │
  └─ DatabaseManager::buyStock(symbol, qty, price, "MARKET")
      ├─ 트랜잭션 시작
      │
      ├─ assetType에 따라 통화 차감
      │  ├─ COIN: usd_cash -= (price / getUsdKrwRate()) * qty
      │  └─ STOCK: cash -= price * qty
      │
      ├─ holdings 갱신 (또는 새로 추가)
      │  ├─ 기존 보유 → 평균단가 재계산
      │  └─ 신규 보유 → 새 행 추가
      │
      ├─ trades 테이블 기록
      │  ├─ symbol, type='BUY', qty, price
      │  ├─ assetType, orderType='MARKET'
      │  └─ timestamp = 현재시각
      │
      ├─ 트랜잭션 커밋
      └─ true 반환

성공 시:
  ├─ refreshPortfolio() → 포트폴리오 테이블 갱신
  ├─ refreshHistory() → 거래 내역 추가
  ├─ updateTradeSummary() → 매매탭 정보 초기화
  └─ 메시지박스: "매수 성공"

실패 시 (잔액부족):
  └─ 메시지박스: "KRW 잔액이 부족합니다"
```

---

## MARKET 매도 흐름

```
매매탭 - "매도" 버튼 클릭
  ↓
onSellClicked()
  ├─ symbol, qty, price (위와 동일)
  │
  └─ DatabaseManager::sellStock(symbol, qty, price, "MARKET")
      ├─ 트랜잭션 시작
      │
      ├─ 보유 수량 확인
      │  └─ qty > holdings[symbol].quantity → 실패
      │
      ├─ assetType에 따라 통화 증가
      │  ├─ COIN: usd_cash += (price / getUsdKrwRate()) * qty
      │  └─ STOCK: cash += price * qty
      │
      ├─ holdings 수량 감소
      │  └─ quantity -= qty (0이면 행 삭제)
      │
      ├─ trades 테이블 기록
      │  └─ type='SELL'
      │
      └─ 트랜잭션 커밋

성공 시:
  ├─ refreshPortfolio()
  ├─ refreshHistory()
  └─ updateTradeSummary()

실패 시 (수량부족):
  └─ 메시지박스: "보유 수량이 부족합니다"
```

---

## LIMIT 주문 등록 흐름

```
예약탭 - "예약 등록" 버튼 클릭 (orderType = "LIMIT")
  ↓
onSchedAddClicked()
  ├─ date = ui->sched_date->date()
  ├─ symbol, type, qty, price (사용자 입력)
  ├─ orderType = "LIMIT" (선택)
  │
  └─ DatabaseManager::addSchedule(date, symbol, type, qty, price, "LIMIT")
      ├─ schedules 테이블에 INSERT
      │  ├─ status = 'PENDING'
      │  ├─ orderType = 'LIMIT'
      │  └─ assetType 자동 판단
      │
      └─ true 반환

성공 시:
  ├─ refreshSchedules() → 예약 테이블에 "PENDING" 상태로 표시
  └─ 메시지박스: "LIMIT 주문이 등록되었습니다"

실패 시:
  └─ 메시지박스: "주문 등록 실패"
```

---

## LIMIT 주문 자동 체결 흐름

```
타이머 1초 주기로 실행
  ↓
onAutoTimeUpdate()
  │
  └─ DatabaseManager::checkAndExecuteSchedules()
      │
      ├─ getSchedules() → status = 'PENDING'만 조회
      │
      ├─ 각 주문에 대해
      │  │
      │  ├─ orderType = 'LIMIT'만 처리
      │  │
      │  ├─ currentPrice = StockData::stockPrice(symbol)
      │  │
      │  ├─ 체결 조건 검사
      │  │  ├─ BUY LIMIT: currentPrice <= order.price
      │  │  └─ SELL LIMIT: currentPrice >= order.price
      │  │
      │  └─ 조건 만족 시
      │     │
      │     ├─ buyStock(symbol, qty, currentPrice, "MARKET")
      │     │   또는 sellStock() 실행
      │     │
      │     ├─ 성공 시
      │     │  └─ UPDATE schedules SET status = 'DONE' WHERE id = order.id
      │     │
      │     └─ 실패 시 (잔액부족)
      │        └─ status 유지 (PENDING) → 다음 타이밍 재시도
      │
      └─ refreshSchedules() → PENDING 주문 목록 갱신
```

**예시**:
```
10:00:00 - LIMIT 주문 등록: BTC 매수 $50,000 목표
10:00:01 - 현재가 $52,000 → 조건 불만족
10:00:02 - 현재가 $50,500 → 조건 불만족
...
10:05:30 - 현재가 $49,500 → 조건 만족 ($49,500 <= $50,000)
         → buyStock() 실행
         → status = 'DONE'
         → 포트폴리오 갱신
```

---

## 환전 흐름

```
은행탭 - "환전" 버튼 클릭
  ↓
onExchangeClicked()
  │
  ├─ QInputDialog 팝업
  │  ├─ "원(KRW) 금액을 입력하세요"
  │  │   또는 "달러(USD) 금액을 입력하세요"
  │  └─ amount 입력
  │
  └─ 입력 방향에 따라
     │
     ├─ KRW → USD 환전
     │  └─ DatabaseManager::exchangeToUsd(krwAmount)
     │     ├─ usdAmount = krwAmount / getUsdKrwRate()
     │     ├─ 트랜잭션
     │     │  ├─ cash -= krwAmount
     │     │  └─ usd_cash += usdAmount
     │     └─ 커밋
     │
     └─ USD → KRW 환전
        └─ DatabaseManager::exchangeToKrw(usdAmount)
           ├─ krwAmount = usdAmount * getUsdKrwRate()
           ├─ 트랜잭션
           │  ├─ usd_cash -= usdAmount
           │  └─ cash += krwAmount
           └─ 커밋

성공 시:
  ├─ refreshPortfolio() → 포트폴리오 잔액 갱신
  ├─ refreshBank() → 은행 정보 갱신
  └─ 메시지박스: "환전 완료"

실패 시 (잔액부족):
  └─ 메시지박스: "잔액이 부족합니다"
```

**예시**:
```
사용자: "1,000,000원을 달러로 환전"
→ exchangeToUsd(1000000)
→ usdAmount = 1,000,000 / 1471 ≈ 679.88
→ KRW: 50,000,000 - 1,000,000 = 49,000,000
→ USD: 10,000 + 679.88 = 10,679.88
```

---

## 탭 전환 흐름

```
사용자: 탭 변경
  ↓
onTabChanged(int tabIndex)
  │
  ├─ 모든 UI 갱신
  │
  ├─ refreshPortfolio()
  ├─ refreshBank()
  ├─ refreshHistory()
  ├─ refreshSchedules()
  ├─ refreshMemos()
  └─ refreshTicker()
  │
  └─ 새 탭 표시
```

---

## 메모 관리 흐름

### 메모 추가
```
메모탭 - "추가" 버튼 클릭
  ↓
onMemoAddClicked()
  ├─ content = ui->memo_input->text()
  ├─ DatabaseManager::addMemo(content)
  │  ├─ INSERT INTO memos (content, done, created_at)
  │  └─ return true
  └─ refreshMemos()
```

### 메모 토글 (체크박스)
```
메모 리스트에서 체크박스 클릭
  ↓
onMemoItemChanged(item)
  ├─ id = item->id
  ├─ DatabaseManager::toggleMemo(id)
  │  ├─ UPDATE memos SET done = NOT done WHERE id = :id
  │  └─ return true
  └─ refreshMemos() → 리스트 갱신
```

### 메모 삭제
```
메모탭 - "삭제" 버튼 클릭
  ↓
onMemoDeleteClicked()
  ├─ id = selectedMemoId()
  ├─ DatabaseManager::deleteMemo(id)
  │  ├─ DELETE FROM memos WHERE id = :id
  │  └─ return true
  └─ refreshMemos()
```

---

## 통합 시퀀스 다이어그램

```
앱 시작
  ↓ initDb() ─────────────┐
  ↓ executeDueSchedules() │
  ↓ startTimer()          │
  ↓ refresh*()            │
  │                       │
UI 렌더링 ◄──────────────┘
  │
  ├─ 매매탭 (onBuyClicked / onSellClicked)
  │  └─ MARKET 매도/매수
  │
  ├─ 예약탭 (onSchedAddClicked)
  │  └─ MARKET / LIMIT 주문 등록
  │
  ├─ 은행탭 (onExchangeClicked)
  │  └─ 환전 (KRW ↔ USD)
  │
  ├─ 메모탭 (메모 추가/삭제/토글)
  │  └─ 메모 관리
  │
  └─ [1초마다 타이머]
     └─ onAutoTimeUpdate()
        └─ LIMIT 주문 체결 검사
           └─ refresh*() (갱신)
```
