# 주요 동작 흐름

## 1. 앱 시작

```
main() → MainWindow
  └─ DatabaseManager::initDb()
      ├─ DB 생성 (stock_portfolio.db)
      ├─ 테이블 생성 (account, holdings, trades, schedules)
      ├─ 초기 현금: KRW 50M, USD $10K
      └─ MARKET 예약 주문 자동 실행
  └─ 1초 타이머 시작 (LIMIT 체크)
  └─ UI 갱신 (포트폴리오, 거래내역, 은행, 메모)
```

---

## 2. MARKET 매수 (주식)

```
매매탭: 종목 선택 → 수량 입력 → "매수" 클릭
  ├─ onBuyClicked()
  └─ DatabaseManager::buyStock(symbol, qty, price)
      ├─ 잔액 확인 (KRW >= price * qty)
      ├─ 차감: cash -= price * qty
      ├─ holdings 갱신 (평균단가 재계산)
      ├─ trades 기록
      └─ 성공 시 UI 갱신
```

---

## 3. MARKET 매도 (주식)

```
매매탭: "매도" 클릭
  ├─ onSellClicked()
  └─ DatabaseManager::sellStock(symbol, qty, price)
      ├─ 보유 수량 확인 (qty > 0)
      ├─ 증가: cash += price * qty
      ├─ holdings 수량 감소
      ├─ trades 기록
      └─ 성공 시 UI 갱신
```

---

## 4. LIMIT 주문 등록

```
예약탭: 종목 → BUY/SELL → 수량 → 가격 → "예약등록"
  ├─ onSchedAddClicked()
  └─ DatabaseManager::addSchedule(...)
      ├─ Schedule 테이블 추가
      ├─ status = 'PENDING'
      ├─ 날짜 = 오늘 또는 미래
      └─ UI 갱신 (PENDING 상태 표시)
```

---

## 5. LIMIT 주문 체결 (타이머)

```
매 1초마다:
  ├─ checkAndExecuteSchedules()
  │  ├─ SQL: date <= 오늘 AND status='PENDING' AND orderType='LIMIT'
  │  └─ 각 주문 루프:
  │      ├─ 현재가 조회
  │      ├─ 체결 조건 확인
  │      │  ├─ BUY: 현재가 <= 주문가
  │      │  └─ SELL: 현재가 >= 주문가
  │      ├─ 조건 만족 시 매매 실행
  │      ├─ 성공 → status = 'DONE'
  │      └─ 실패 (잔액부족) → status 유지 (재시도)
  └─ UI 갱신 (주문 상태, 포트폴리오)
```

---

## 6. 환전

```
은행탭: "환전" 버튼 클릭
  ├─ QInputDialog: 금액 입력
  └─ DatabaseManager::exchange(amount, direction)
      ├─ KRW → USD:
      │   ├─ cash -= amount
      │   ├─ usd_cash += amount / getUsdKrwRate()
      └─ USD → KRW:
          ├─ usd_cash -= amount
          └─ cash += amount * getUsdKrwRate()
      └─ UI 갱신 (은행 정보)
```

---

## 7. MARKET 예약 (앱 재시작 시)

```
MainWindow 생성 중:
  ├─ executeDueSchedules()
  │  ├─ SQL: date <= 오늘 AND status='PENDING' AND orderType='MARKET'
  │  ├─ 각 주문:
  │  │   ├─ 매매 실행 (buyStock or sellStock)
  │  │   ├─ 성공 → status = 'DONE'
  │  │   └─ 실패 → status = 'ERROR'
  │  └─ UI 갱신
  └─ (LIMIT 주문은 타이머에서만 체크)
```

---

## 8. 예외 처리

```
모든 매매/환전 시:
  ├─ 잔액 부족 → 메시지박스 + 실패 반환
  ├─ 수량 부족 → 메시지박스 + 실패 반환
  └─ DB 오류 → 롤백 + 메시지박스
```

---

## 9. UI 갱신 순서

```
매 매매 후:
1. refreshPortfolio() → 보유 종목, 자산 총액
2. refreshHistory() → 거래 내역 추가
3. refreshBank() → 현금 잔액
4. refreshPendingOrders() → 예약 주문 상태
5. updateTradeSummary() → 매매탭 정보 초기화
6. showToast() → "매수 완료" 메시지
```

---

**핵심**: 모든 매매는 트랜잭션 단위 처리 (원자성 보장)
