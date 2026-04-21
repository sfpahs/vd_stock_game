# UI 레이아웃 (mainwindow.ui)

## 6탭 구성 개요

```
┌─────────────────────────────────────┐
│ [1] [2] [3] [4] [5] [6]             │  탭 헤더
├─────────────────────────────────────┤
│                                     │
│     선택된 탭의 내용 표시            │
│                                     │
└─────────────────────────────────────┘
```

---

## Tab 1: 포트폴리오

```
┌─────────────────────────────────────────────────┐
│ 현금(KRW): 50,000,000 원  현금(USD): $10,000.00  │
│ 총 평가액: ---  총 손익: ---                    │
│ 💱 USD/KRW: 1,471 | 🥇 ...                     │
├─────────────────────────────────────────────────┤
│ 종목코드 | 종목명 | 수량 | 평균단가 | 현재가   │
│ ─────────┼───────┼──────┼────────┼────────  │
│ 005930  | 삼성전자| 100 | 70,000 | 78,000  │
│ BTC     | 비트코인| 0.5 |  $35k  | $41k    │
│ ...                                         │
└─────────────────────────────────────────────────┘
```

**위젯**:
- QLabel: "현금(KRW)", "현금(USD)", "총 평가액", "총 손익"
- QTableView (PortfolioModel): 7컬럼 테이블
  - COIN 행: USD 표시 ($)
  - STOCK 행: KRW 표시 (원)
  - 손익 색상: 양수(빨강) / 음수(파랑)

---

## Tab 2: 매매

```
┌───────────────────────────────┐
│ 종목: [005930 ▼]              │
│ 현재가: 78,000 원            │
│ 수량: [100     ]              │
│ 총 금액: 7,800,000 원         │
│ [매수]  [매도]               │
└───────────────────────────────┘
```

**위젯**:
- QComboBox (`symbol_combo`): 종목 선택 (FX 제외)
- QLabel (`current_price_label`): 현재가 ($ 또는 원)
- QSpinBox (`qty_spinbox`): 수량
- QDoubleSpinBox (`price_input`): 가격 (소수점 2자리)
- QLabel (`total_amount_label`): 총금액 (통화 표시)
- QPushButton: "매수", "매도"

**신호-슬롯**:
- `symbol_combo` changed → `onSymbolChanged()` → 현재가 갱신
- `price_input` / `qty_spinbox` changed → `onPriceChanged()` / `onQtyChanged()` → 총금액 갱신
- "매수" 클릭 → `onBuyClicked()` → MARKET 주문
- "매도" 클릭 → `onSellClicked()` → MARKET 주문

---

## Tab 3: 거래내역

```
┌──────────────────────────────────────────┐
│ 시간            | 종목 | 유형 | 수량 | 가격   │
├──────────────────────────────────────────┤
│ 2026-04-21 10:30 | 005930 | BUY  | 100 | 70,000 │
│ 2026-04-21 10:15 | BTC    | SELL | 0.5 | $41k   │
│ ...                                      │
└──────────────────────────────────────────┘
```

**위젯**:
- QTableWidget: 5컬럼 (시간, 종목, 유형, 수량, 가격)
  - 읽기 전용
  - 최신순 정렬

---

## Tab 4: 매매 예약 (MARKET/LIMIT)

```
┌────────────────────────────────────────────┐
│ 예약 날짜: [2026-04-21 ▼]                 │
│ 종목: [005930 ▼]                          │
│ 유형: [BUY ▼]                             │
│ 수량: [100]                               │
│ 가격: [70,000.00]                         │
│ 주문유형: [MARKET ▼]                      │
│ [예약 등록]                               │
├────────────────────────────────────────────┤
│ 날짜  | 종목 | 유형 | 수량 | 상태         │
├────────────────────────────────────────────┤
│ 2026-04-22 | 005930 | BUY  | 100 | PENDING │
│ 2026-04-22 | BTC    | SELL | 0.5 | DONE    │
│ ...                                       │
└────────────────────────────────────────────┘
```

**위젯**:
- QDateEdit (`sched_date`): 예약 날짜
- QComboBox: 종목, 유형(BUY/SELL), 주문유형(MARKET/LIMIT)
- QSpinBox: 수량
- QDoubleSpinBox: 가격
- QPushButton: "예약 등록"
- QTableWidget: 예약 목록 (날짜, 종목, 유형, 수량, 상태)

**신호-슬롯**:
- "예약 등록" 클릭 → `onSchedAddClicked()` → 주문 저장
  - orderType = "MARKET" → 앱 재시작 시 자동 실행
  - orderType = "LIMIT" → 매 1초 체결 조건 검사

---

## Tab 5: 은행 (환전)

```
┌─────────────────────────────┐
│ 현금(KRW): 50,000,000 원   │
│ 현금(USD): $10,000.00      │
│ 환율(USD/KRW): 1,471       │
│ [환전 버튼]                │
└─────────────────────────────┘
```

**위젯**:
- QLabel: 현금(KRW), 현금(USD), 환율
- QPushButton: "환전"

**신호-슬롯**:
- "환전" 클릭 → `onExchangeClicked()` → QInputDialog
  - KRW 금액 입력 → 달러 변환 → USD 증가, KRW 감소
  - USD 금액 입력 → 원 변환 → KRW 증가, USD 감소

---

## Tab 6: 메모/할일

```
┌────────────────────────────────┐
│ [메모 입력 ________] [추가] [삭제] │
├────────────────────────────────┤
│ ☐ 포트폴리오 리밸런싱           │
│ ☑ Apple 주식 매수 (완료)        │
│ ☐ 환전: 100만원 → 달러          │
│ ...                            │
└────────────────────────────────┘
```

**위젯**:
- QLineEdit: 메모 입력
- QPushButton: "추가", "삭제"
- QListWidget: 메모 목록 (체크박스)

**신호-슬롯**:
- "추가" 클릭 → `onMemoAddClicked()` → 메모 저장
- "삭제" 클릭 → `onMemoDeleteClicked()` → 선택 메모 삭제
- 메모 체크박스 → `onMemoItemChanged()` → done 토글

---

## 공통 요소

### 상단 공유 영역
```
┌────────────────────────────────────────┐
│ 💱 USD/KRW: 1,471 | 🥇 현재 종목 목록   │
└────────────────────────────────────────┘
```
- **refreshTicker()**: 1초 단위 환율 업데이트
- **refreshPortfolio()**: 포트폴리오 갱신
- **refreshBank()**: 은행 정보 갱신

### 신호-슬롯 연결 (mainwindow.cpp)
```cpp
connect(ui->tab_widget, SIGNAL(currentChanged(int)), 
        this, SLOT(onTabChanged(int)));
        
connect(m_priceTimer, SIGNAL(timeout()), 
        this, SLOT(onAutoTimeUpdate()));  // LIMIT 체결
```

---

## 레이아웃 구조 (Qt Designer)

```
MainWindow
├── m_centralWidget (QWidget)
│   ├── m_main_layout (QVBoxLayout)
│   │   ├── ticker_label (QLabel)
│   │   └── tab_widget (QTabWidget)
│   │       ├── Tab 1: portfolio_widget
│   │       ├── Tab 2: trading_widget
│   │       ├── Tab 3: history_widget
│   │       ├── Tab 4: schedule_widget
│   │       ├── Tab 5: bank_widget
│   │       └── Tab 6: memo_widget
```
