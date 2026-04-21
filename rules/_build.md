# 빌드 설정 및 오류 해결

## 빌드 환경

**요구사항**:
- Qt 6.11.0 이상
- CMake 3.16 이상
- MinGW 64-bit
- Windows 11 Pro

---

## CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.16)
project(stock_scheduler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS
    Core Gui Widgets Sql REQUIRED
)

add_executable(stock_scheduler
    main.cpp
    mainwindow.h mainwindow.cpp mainwindow.ui
    database.h database.cpp
    stockdata.h
    portfoliomodel.h portfoliomodel.cpp
)

target_link_libraries(stock_scheduler PRIVATE
    Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Sql
)
```

---

## 빌드 절차

### Qt Creator에서
1. **File** → **Open Project** → `CMakeLists.txt` 선택
2. **Kit**: MinGW 64-bit 선택
3. **Build** → **Build Project** (Ctrl+B)

### 명령행
```bash
mkdir -p build && cd build
cmake -G "MinGW Makefiles" ..
mingw32-make -j4
./Desktop_Qt_6_11_0_MinGW_64_bit-Debug/stock_scheduler.exe
```

---

## 컴파일 오류 해결

### "cannot find -lQt6Sql"
**원인**: CMakeLists.txt에 Qt6::Sql 누락

**해결**:
```cmake
find_package(Qt6 COMPONENTS ... Sql REQUIRED)
target_link_libraries(...Qt6::Sql)
```

### "error: no matching function for call to 'connect'"
**원인**: 신호/슬롯 타입 미스매치

**해결**:
```cpp
// ❌ 잘못된 예
connect(spinbox, SIGNAL(valueChanged(int)), 
        this, SLOT(onPriceChanged(int)));

// ✓ 올바른 예
connect(spinbox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        this, &MainWindow::onPriceChanged);
```

### "error: 'QInputDialog' has not been declared"
**원인**: 헤더 누락

**해결**: mainwindow.cpp에 추가
```cpp
#include <QInputDialog>
#include <QTimer>
#include <QLocale>
#include <QMessageBox>
```

### "undefined reference to 'DatabaseManager::instance()'"
**원인**: database.cpp에서 정의 누락

**해결**: database.cpp
```cpp
DatabaseManager& DatabaseManager::instance() {
    static DatabaseManager instance;
    return instance;
}
```

---

## Clazy 경고 해결 규칙

Qt Creator의 clazy 정적 분석기가 아래 경고를 발생시킬 경우 다음 규칙을 적용한다.

---

### clazy-range-loop-detach

**원인**: `for (const T& x : container)` 에서 `container`가 non-const이면 Qt 암시적 공유 컨테이너가 detach될 수 있음

**해결**: 컨테이너 변수를 `const auto`로 선언

```cpp
// ❌ 경고 발생
auto trades = DatabaseManager::instance().getTrades();
for (const TradeRow& t : trades) { ... }

// ✓ const auto로 선언
const auto trades = DatabaseManager::instance().getTrades();
for (const TradeRow& t : trades) { ... }
```

---

### clazy-qstring-arg

**원인**: `.arg(a).arg(b)` 처럼 단일 arg를 연속 체이닝하면 QString을 매번 재생성함

**해결**: multi-arg 형식 `.arg(a, b, c)` 사용

```cpp
// ❌ 경고 발생
QString("%1 %2 %3%").arg(a).arg(b).arg(c);

// ✓ multi-arg 형식
QString("%1 %2 %3%").arg(a, b, c);
```

---

### clazy-qcolor-from-literal

**원인**: `QColor("#E53935")` 처럼 문자열 리터럴로 QColor를 생성하면 런타임에 문자열 파싱 비용이 발생함

**해결**: 용도에 따라 두 가지 방법 적용

| 용도 | 해결 방법 |
|------|-----------|
| `setStyleSheet()` 에서만 사용 | `const char*` 문자열 직접 사용 (QColor 생성 자체 제거) |
| `setForeground()` / `setBackground()` 등 QColor 객체 필요 | `QColor(r, g, b)` int 생성자 사용 |

```cpp
// ── stylesheet 용도 ──────────────────────────────
// ❌ 경고 발생
QColor c = rate >= 0 ? QColor("#E53935") : QColor("#1E88E5");
label->setStyleSheet(QString("color: %1;").arg(c.name()));

// ✓ const char* 직접 사용
const char* colorStr = rate >= 0 ? "#E53935" : "#1E88E5";
label->setStyleSheet(QString("color: %1;").arg(colorStr));

// ── QColor 객체가 필요한 경우 ────────────────────
// ❌ 경고 발생
item->setForeground(QColor("#E53935"));

// ✓ int 생성자 사용
item->setForeground(QColor(229, 57, 53));
```

**프로젝트 사용 색상표**

| 색상 역할 | 문자열 | QColor(r, g, b) |
|-----------|--------|-----------------|
| 빨강 (상승/매수) | `"#E53935"` | `QColor(229, 57, 53)` |
| 파랑 (하락/매도) | `"#1E88E5"` | `QColor(30, 136, 229)` |

---

**다음**: [테스트 체크리스트](rules/_testing.md)
