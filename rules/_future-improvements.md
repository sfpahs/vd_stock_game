# 향후 개선 사항

## 우선순위 로드맵

### Priority 1: 핵심 기능 완성 (분기 Q2 2026)

#### 1.1 실시간 시세 API 연동
**목표**: 하드코딩 대신 실제 데이터 사용

**구현**:
- Yahoo Finance API 또는 Alpha Vantage
- 환율 API (Open Exchange Rates)
- 1분 단위 데이터 갱신

```cpp
// 현재 (하드코딩)
double price = StockData::stockPrice("005930");  // 78000

// 개선 후 (API)
double price = APIManager::stockPrice("005930");  // 실시간
```

**영향 범위**: database.cpp, portfoliomodel.cpp, 매매탭

#### 1.2 차트 및 시각화
**목표**: 주가/환율 차트, 자산 변화 그래프

**도구**:
- QCustomPlot (경량, 무료)
- Qt Charts (Qt 기본 라이브러리)

**기능**:
- 일일/주간/월간 차트
- 포트폴리오 구성 파이차트
- 손익률 그래프

#### 1.3 성과 분석 대시보드
**목표**: 수익률, MDD, Sharpe Ratio 등

**추가 탭**: Tab 7 "분석" (선택사항)

```
┌─────────────────────────────────┐
│ 총수익률: +15.2%                │
│ 최대낙폭(MDD): -5.3%            │
│ Sharpe Ratio: 1.24              │
├─────────────────────────────────┤
│ [월별 수익률 차트]              │
│ [자산 변화 그래프]              │
└─────────────────────────────────┘
```

---

### Priority 2: 사용성 개선 (분기 Q3 2026)

#### 2.1 Stop-Loss / Take-Profit 주문
**목표**: LIMIT 주문 확장

**새로운 주문 타입**:
```sql
orderType: 'STOP_LOSS' | 'TAKE_PROFIT' | 'TRAILING_STOP'

예시:
- STOP_LOSS: 현재가 아래로 내려가면 매도 (손실 제한)
- TAKE_PROFIT: 현재가 위로 올라가면 매도 (수익 고정)
- TRAILING_STOP: 최고가 기준 -5% 하락 시 매도 (동적 손실방지)
```

#### 2.2 포트폴리오 리밸런싱
**목표**: 목표 배분 자동 조정

```
현재: 주식 80%, 코인 20%
목표: 주식 60%, 코인 40%
→ 자동 매도/매수로 리밸런싱
```

#### 2.3 거래 수수료 및 세금 시뮬레이션
**목표**: 실제 비용 반영

```cpp
struct Fee {
    double commission;     // 0.1% (주식)
    double spread;        // $2 (코인)
    double tax;          // 15% (거래소득세)
};

// buyStock() 수정
totalCost = price * qty + totalCost * Fee.commission;
```

#### 2.4 다중 포트폴리오
**목표**: 여러 계좌 관리

```
계좌 목록:
├─ "안전 자산" (현금+채권)
├─ "공격 자산" (성장주+코인)
└─ "글로벌" (달러, 유로)

각 계좌별 독립적 거래, 분석
```

---

### Priority 3: 데이터 관리 (분기 Q3-Q4 2026)

#### 3.1 CSV/Excel 내보내기
**목표**: 거래내역, 포트폴리오 기록

```cpp
void MainWindow::onExportClicked() {
    QString filename = QFileDialog::getSaveFileName(
        this, "Export", "", "CSV Files (*.csv);;Excel Files (*.xlsx)"
    );
    
    if (filename.endsWith(".csv")) {
        exportToCSV(filename);
    } else if (filename.endsWith(".xlsx")) {
        exportToXLSX(filename);  // xlsxwriter 라이브러리
    }
}
```

#### 3.2 데이터 백업 및 복원
**목표**: 자동 백업, 버전 관리

```
백업 위치: {AppData}/stock_scheduler_backups/
파일명: stock_portfolio_2026-04-21_10-30-45.db

자동 백업: 매일 자정, 거래 후
복원: 메뉴 → 도구 → 복원
```

#### 3.3 클라우드 동기화
**목표**: 여러 기기에서 접근

**선택사항**:
- Google Drive / OneDrive (기본)
- 자체 서버 (고급)

---

### Priority 4: 고급 기능 (2027년 이후)

#### 4.1 옵션/선물 거래
**목표**: 파생상품 지원

```
새로운 assetType: 'OPTION' | 'FUTURES'

옵션:
- Call / Put
- 행사가, 만기일
- Greeks (Delta, Gamma, Theta)

선물:
- 레버리지 거래
- 증거금 관리
```

#### 4.2 분할 매수/매도 (DCA, DCS)
**목표**: 평균 단가 최적화

```
DCA (Dollar Cost Averaging):
- 매월 1일에 100만원 자동 매수

DCS (Dollar Cost Selling):
- 목표가 도달 후 나눠서 매도
```

#### 4.3 알고리즘 거래 (Bot)
**목표**: 자동 거래 전략

```
제공 전략:
1. Moving Average Crossover (MA 20/50 교차 매매)
2. RSI Oversold/Overbought
3. Bollinger Bands
4. Custom Python Script

예시:
if (MA50 > MA200) {
    buySignal = true;
}
```

#### 4.4 소셜 기능
**목표**: 포트폴리오 공유, 벤치마킹

- 공개 포트폴리오 (익명)
- 리더보드 (수익률 랭킹)
- 전략 공유
- 댓글 / 평가

---

## 기술 개선사항

### 데이터베이스

#### 쿼리 최적화
```sql
-- 현재 (느림)
SELECT * FROM trades WHERE symbol = 'BTC'
SELECT * FROM holdings WHERE symbol = 'BTC'

-- 개선 (인덱스 추가)
CREATE INDEX idx_trades_symbol ON trades(symbol);
CREATE INDEX idx_holdings_symbol ON holdings(symbol);
CREATE INDEX idx_schedules_status ON schedules(status);
```

#### 마이그레이션 자동화
```cpp
class DatabaseMigration {
    v1_0_0() {  // 초기 스키마
        CREATE TABLE ...
    }
    
    v1_1_0() {  // 이중통화 추가
        ALTER TABLE account ADD COLUMN usd_cash ...
    }
    
    v1_2_0() {  // LIMIT 주문 추가
        ALTER TABLE schedules ADD COLUMN orderType ...
    }
}
```

### UI/UX

#### 다크 모드
```cpp
void MainWindow::applyDarkTheme() {
    QPalette palette;
    palette.setColor(QPalette::Window, QColor(53, 53, 53));
    palette.setColor(QPalette::WindowText, Qt::white);
    setPalette(palette);
}
```

#### 반응형 레이아웃
- 윈도우 리사이징 시 자동 조정
- 태블릿 지원 (QTabletEvent)

#### 온보딩 튜토리얼
```
첫 실행:
1. 환영 화면
2. 기본 개념 (종목, 매매, 환율)
3. 첫 번째 매매 가이드
4. 닫기
```

### 성능

#### 캐싱
```cpp
struct PriceCache {
    QMap<QString, CachedPrice> m_cache;
    QTimer m_refreshTimer;  // 1분마다 갱신
};

double getPrice(const QString& symbol) {
    if (m_cache.contains(symbol) && !m_cache[symbol].isExpired()) {
        return m_cache[symbol].price;  // 캐시 사용
    }
    return fetchFromAPI(symbol);  // API 호출
}
```

#### 비동기 처리
```cpp
// 현재 (블로킹)
buyStock(...);  // UI 프리징

// 개선 (비동기)
QtConcurrent::run([this]() {
    buyStock(...);
});
```

---

## 확장성 계획

### 플러그인 아키텍처
```
plugins/
├── strategy_ma_crossover/
├── api_yahoo_finance/
├── export_excel/
└── broker_integration/
```

### REST API 제공
```cpp
class APIServer : public QHttpServer {
    // GET /api/portfolio
    // GET /api/trades
    // POST /api/order
    // GET /api/analysis
};
```

### 웹 버전 (Qt for WebAssembly)
```
stock_scheduler (C++ Desktop)
     ↓
wasmify (WebAssembly 변환)
     ↓
Web App (WASM + JavaScript)
```

---

## 사용자 피드백 우선순위

### 자주 요청되는 기능
1. **실시간 시세** (80% 요청)
   → Priority 1.1로 이동

2. **차트** (65% 요청)
   → Priority 1.2로 이동

3. **CSV 내보내기** (45% 요청)
   → Priority 3.1로 이동

4. **더 많은 암호화폐** (30% 요청)
   → 간단 (stockdata.h에 추가)

5. **모바일 앱** (20% 요청)
   → Priority 4 (장기 계획)

---

## 릴리스 계획

```
v1.0.0 (2026-04-30)
  └─ 핵심 기능 완성 (현재)

v1.1.0 (2026-06-30)
  ├─ 실시간 시세 API
  ├─ 차트 기능
  └─ 성과 분석

v1.2.0 (2026-09-30)
  ├─ Stop-Loss / Take-Profit
  ├─ CSV 내보내기
  └─ 다중 포트폴리오

v2.0.0 (2027-Q1)
  ├─ 옵션/선물
  ├─ 알고리즘 거래
  └─ REST API
```

---

## 커뮤니티 기여 환영

**오픈소스**: MIT License
- **Issue 제보**: GitHub Issues
- **PR 기여**: Feature, Bugfix 환영
- **토론**: Discussions

**기여 가이드**:
1. Fork
2. Feature 브랜치 생성
3. 커밋
4. PR 생성
5. Code Review

**좋은 제1 PR**:
- [ ] 추가 종목 (stockdata.h)
- [ ] 번역 (i18n)
- [ ] 문서 개선
- [ ] 버그 픽스
