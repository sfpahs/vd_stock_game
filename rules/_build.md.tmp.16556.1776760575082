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

**다음**: [테스트 체크리스트](rules/_testing.md)
