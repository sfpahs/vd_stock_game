# OOP 리팩토링 (3주 계획)

## 현재 문제점

```cpp
// 정적 함수 기반 → 데이터/로직 분리
class DatabaseManager { static bool buyStock(...); };
class StockData { static double stockPrice(...); };
```

**문제**:
- 데이터와 행동 분리
- 상태 관리 어려움
- 확장성 제한

---

## 개선 목표

```
DatabaseManager (싱글톤)
  ├─ Account (계좌)
  ├─ StockManager (종목)
  ├─ TradeService (매매)
  ├─ ScheduleService (예약)
  ├─ ExchangeService (환전)
```

---

## 3주 리팩토링 계획

| 주차 | 작업 | 파일 |
|------|------|------|
| **Week 1** | 데이터 객체화 (Account, Stock, Holding, Trade) | [Phase 1](rules/_refactoring-phase1.md) |
| **Week 2** | 서비스 계층 (TradeService, ScheduleService, ExchangeService) | [Phase 2](rules/_refactoring-phase2.md) |
| **Week 3** | MainWindow 리팩토링 + 예외 처리 | [Phase 3](rules/_refactoring-phase3.md) |

---

## 이점

✅ **유지보수성**: 단일 책임, 변경 영향 제한  
✅ **재사용성**: 서비스를 다른 UI/API에서도 활용  
✅ **확장성**: 새 주문 타입, 자산 타입 추가 용이  
✅ **코드품질**: 응집도 ↑, 결합도 ↓, 가독성 ↑  

---

**참고**: 각 Phase는 별도 문서에서 상세 코드 예시와 구현 순서를 제공합니다.
