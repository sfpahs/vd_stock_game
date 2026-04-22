cmake_minimum_required(VERSION 3.19)
project(stock_game LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Sql Charts Multimedia)

qt_standard_project_setup()

qt_add_executable(stock_game
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    database.h
    database.cpp
    stockdata.h
    stockmanager.h
    stockmanager.cpp
    models.h
    exceptions.h
    tradeservice.h
    exchangeservice.h
    scheduleservice.h
    portfoliomodel.h
    portfoliomodel.cpp
    tickerwidget.h
    tickerwidget.cpp
)

target_include_directories(stock_game PRIVATE ${CMAKE_SOURCE_DIR})

target_link_libraries(stock_game
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Sql
        Qt::Charts
        Qt::Multimedia
)

install(TARGETS stock_game
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET stock_game
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
