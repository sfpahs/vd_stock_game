#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "database.h"
#include "stockdata.h"

#include <QDebug>
#include <QBrush>
#include <QPalette>
#include <QIcon>
#include <QTableWidgetItem>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QDate>
#include <QLocale>
#include <QTimer>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QChart>
#include <QChartView>
#include <QValueAxis>
#include <QCategoryAxis>
#include <QCandlestickSeries>
#include <QCandlestickSet>
#include <QLineSeries>
#include <QDialog>
#include <QLabel>
#include <QFormLayout>
#include <QSpinBox>
#include <QPushButton>
#include <QApplication>
#include <QScreen>
#include <QLineEdit>
#include <QInputDialog>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QCoreApplication>
#include <algorithm>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_portModel(new PortfolioModel(this))
    , m_interestCounter(0)
{
    ui->setupUi(this);
    setWindowTitle("Stock Game");

    QTimer::singleShot(100, this, &MainWindow::initializeUI);
}

void MainWindow::initializeUI()
{
    // 배경색 설정
    QColor bgColor(240, 240, 240);  // 밝은 회색

    // MainWindow 배경
    QPalette mainPalette;
    mainPalette.setColor(QPalette::Window, bgColor);
    this->setAutoFillBackground(true);
    this->setPalette(mainPalette);

    // centralwidget 배경
    QPalette centralPalette;
    centralPalette.setColor(QPalette::Window, bgColor);
    ui->centralwidget->setAutoFillBackground(true);
    ui->centralwidget->setPalette(centralPalette);

    // tabWidget 배경
    QPalette tabPalette;
    tabPalette.setColor(QPalette::Window, bgColor);
    ui->tabWidget->setAutoFillBackground(true);
    ui->tabWidget->setPalette(tabPalette);

    setupUIElements();
}

void MainWindow::setupUIElements()
{
    // 수익률 정보 레이블
    m_profitRateLabel = new QLabel(this);
    m_profitRateLabel->setStyleSheet("QLabel { padding: 5px 10px; font-weight: bold; }");
    m_profitRateLabel->setMinimumWidth(120);

    // 환율 정보 레이블
    m_exchangeRateLabel = new QLabel(this);
    m_exchangeRateLabel->setStyleSheet("QLabel { padding: 5px 10px; font-weight: bold; }");
    m_exchangeRateLabel->setMinimumWidth(150);

    // 오른쪽 코너: 수익률 + 환율 + ticker(순위) + 설정 버튼
    QWidget* cornerContainer = new QWidget(this);
    QHBoxLayout* cornerLayout = new QHBoxLayout(cornerContainer);
    cornerLayout->setContentsMargins(0, 0, 4, 0);
    cornerLayout->setSpacing(4);

    cornerLayout->addWidget(m_profitRateLabel);
    cornerLayout->addWidget(m_exchangeRateLabel);

    m_ticker = new TickerWidget(cornerContainer);
    cornerLayout->addWidget(m_ticker);

    QPushButton* settingsBtn = new QPushButton("⚙️", cornerContainer);
    settingsBtn->setFixedSize(40, 40);
    settingsBtn->setStyleSheet("QPushButton { border: none; font-size: 20px; }");
    cornerLayout->addWidget(settingsBtn);

    ui->tabWidget->setCornerWidget(cornerContainer, Qt::TopRightCorner);
    connect(settingsBtn, &QPushButton::clicked, this, &MainWindow::onSettingsClicked);

    DatabaseManager::instance().initDb();

    // 최초 실행시 100초분 데이터 생성
    for (int i = 0; i < 100; ++i) {
        updateGamePrices();
    }

    m_portModel->refresh();
    ui->port_table_view->setModel(m_portModel);
    ui->port_table_view->horizontalHeader()->setVisible(true);
    ui->port_table_view->horizontalHeader()->setStretchLastSection(false);
    for (int i = 0; i < m_portModel->columnCount(); ++i) {
        ui->port_table_view->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);
    }

    ui->trade_price_input->setMinimum(1);

    setupStockCombo();

    // 종목 정보 패널 생성 (아이콘, 이름, 현재가, 등락율)
    qDebug() << "Creating stock info panel...";
    QWidget* stockInfoPanel = new QWidget(this);
    QHBoxLayout* stockInfoLayout = new QHBoxLayout(stockInfoPanel);
    stockInfoLayout->setContentsMargins(5, 5, 5, 5);
    stockInfoLayout->setSpacing(15);

    // 아이콘
    m_stockIconLabel = new QLabel(this);
    m_stockIconLabel->setFixedSize(80, 80);
    m_stockIconLabel->setScaledContents(true);
    m_stockIconLabel->setAlignment(Qt::AlignCenter);
    m_stockIconLabel->setStyleSheet("QLabel { background-color: white; }");
    stockInfoLayout->addWidget(m_stockIconLabel);
    qDebug() << "Stock icon label created";

    // 텍스트 정보 (이름, 현재가, 등락율)
    QVBoxLayout* stockTextLayout = new QVBoxLayout();

    m_stockNameLabel = new QLabel("", this);
    m_stockNameLabel->setStyleSheet("QLabel { font-size: 14px; font-weight: bold; }");
    stockTextLayout->addWidget(m_stockNameLabel);

    m_stockPriceLabel = new QLabel("현재가: --", this);
    m_stockPriceLabel->setStyleSheet("QLabel { font-size: 12px; }");
    stockTextLayout->addWidget(m_stockPriceLabel);

    m_stockChangeRateLabel = new QLabel("등락율: --", this);
    m_stockChangeRateLabel->setStyleSheet("QLabel { font-size: 12px; }");
    stockTextLayout->addWidget(m_stockChangeRateLabel);

    stockTextLayout->addStretch();
    stockInfoLayout->addLayout(stockTextLayout);
    stockInfoLayout->addStretch();

    // trade_left_vlayout에 최상단에 삽입
    QVBoxLayout* tradeLeftLayout = qobject_cast<QVBoxLayout*>(ui->trade_left_vlayout);
    if (tradeLeftLayout) {
        qDebug() << "trade_left_vlayout found, inserting stock info panel";
        tradeLeftLayout->insertWidget(0, stockInfoPanel);
        // 구분선 추가
        QFrame* separator = new QFrame(this);
        separator->setFrameShape(QFrame::HLine);
        separator->setFrameShadow(QFrame::Sunken);
        tradeLeftLayout->insertWidget(1, separator);
        qDebug() << "Stock info panel inserted successfully";
    } else {
        qDebug() << "ERROR: trade_left_vlayout not found!";
    }

    ui->trade_buy_btn->setCheckable(true);
    ui->trade_sell_btn->setCheckable(true);
    ui->trade_buy_btn->setChecked(true);

    ui->trade_buy_btn->setStyleSheet(
        "QPushButton { color: white; background-color: #E53935; border: none; padding: 5px; border-radius: 3px; font-weight: bold; opacity: 0.6; }"
        "QPushButton:hover { background-color: #D32F2F; opacity: 0.8; }"
        "QPushButton:checked { background-color: #C62828; opacity: 1.0; font-weight: bold; }"
    );
    ui->trade_sell_btn->setStyleSheet(
        "QPushButton { color: white; background-color: #1E88E5; border: none; padding: 5px; border-radius: 3px; font-weight: bold; opacity: 0.6; }"
        "QPushButton:hover { background-color: #1565C0; opacity: 0.8; }"
        "QPushButton:checked { background-color: #0D47A1; opacity: 1.0; font-weight: bold; }"
    );

    connect(ui->trade_symbol_combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onSymbolChanged);
    connect(ui->trade_order_type_combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onOrderTypeChanged);
    connect(ui->trade_buy_btn, &QPushButton::toggled,
            this, &MainWindow::onBuyClicked);
    connect(ui->trade_sell_btn, &QPushButton::toggled,
            this, &MainWindow::onSellClicked);
    connect(ui->trade_qty_spin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::onQtyChanged);
    connect(ui->trade_price_input, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onPriceChanged);
    connect(ui->trade_execute_btn, &QPushButton::clicked,
            this, &MainWindow::onExecuteClicked);
    connect(ui->trade_next_time_btn, &QPushButton::clicked,
            this, &MainWindow::onNextTimeClicked);

    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);

    connect(ui->trade_modify_btn, &QPushButton::clicked, this, &MainWindow::onModifyScheduleClicked);
    connect(ui->trade_cancel_btn, &QPushButton::clicked, this, &MainWindow::onCancelScheduleClicked);
    ui->trade_modify_btn->setVisible(false);
    ui->trade_cancel_btn->setVisible(false);

    ui->trade_pending_table->setSelectionMode(QAbstractItemView::NoSelection);
    ui->trade_pending_table->setStyleSheet(
        "QTableWidget::item:hover { background-color: transparent; }"
    );

    m_priceTimer = new QTimer(this);
    connect(m_priceTimer, &QTimer::timeout, this, &MainWindow::onAutoTimeUpdate);
    m_priceTimer->start(1000);

    m_audioOutput = new QAudioOutput(this);
    m_audioPlayer = new QMediaPlayer(this);
    m_audioPlayer->setAudioOutput(m_audioOutput);

    // 환전/대출/상환 버튼 추가
    QHBoxLayout* buttonLayout = new QHBoxLayout();

    QPushButton* exchangeBtn = new QPushButton("환전 (원 ↔ 달러)", this);
    exchangeBtn->setStyleSheet(
        "QPushButton { background-color: #FFA726; color: white; border: none; padding: 8px; border-radius: 4px; font-weight: bold; }"
        "QPushButton:hover { background-color: #FB8C00; }"
        "QPushButton:pressed { background-color: #E65100; }"
    );

    QPushButton* borrowBtn = new QPushButton("대출받기", this);
    borrowBtn->setStyleSheet(
        "QPushButton { background-color: #7B1FA2; color: white; border: none; padding: 8px; border-radius: 4px; font-weight: bold; }"
        "QPushButton:hover { background-color: #6A1B9A; }"
        "QPushButton:pressed { background-color: #4A148C; }"
    );

    QPushButton* repayBtn = new QPushButton("상환하기", this);
    repayBtn->setStyleSheet(
        "QPushButton { background-color: #C2185B; color: white; border: none; padding: 8px; border-radius: 4px; font-weight: bold; }"
        "QPushButton:hover { background-color: #AD1457; }"
        "QPushButton:pressed { background-color: #880E4F; }"
    );

    buttonLayout->addWidget(exchangeBtn);
    buttonLayout->addWidget(borrowBtn);
    buttonLayout->addWidget(repayBtn);

    // 환율 정보 패널 (이미지 + 환율)
    QWidget* exchangeRatePanel = new QWidget(this);
    QHBoxLayout* exchangeRateLayout = new QHBoxLayout(exchangeRatePanel);
    exchangeRateLayout->setContentsMargins(10, 10, 10, 10);
    exchangeRateLayout->setSpacing(15);

    // 환율 이미지
    QLabel* exchangeImageLabel = new QLabel(this);
    exchangeImageLabel->setFixedSize(100, 100);
    exchangeImageLabel->setScaledContents(true);
    exchangeImageLabel->setAlignment(Qt::AlignCenter);
    exchangeImageLabel->setStyleSheet("QLabel { background-color: white; border-radius: 8px; }");

    QString usdKrwImagePath = "images/usd-krw.png";
    QStringList usdKrwPathsToTry = {
        usdKrwImagePath,
        QDir::currentPath() + "/" + usdKrwImagePath,
        QCoreApplication::applicationDirPath() + "/" + usdKrwImagePath,
        QCoreApplication::applicationDirPath() + "/../../" + usdKrwImagePath,
        QCoreApplication::applicationDirPath() + "/../../../" + usdKrwImagePath,
    };
    for (const QString& tryPath : usdKrwPathsToTry) {
        if (QFile::exists(tryPath)) {
            QPixmap pixmap(tryPath);
            exchangeImageLabel->setPixmap(pixmap);
            break;
        }
    }
    exchangeRateLayout->addWidget(exchangeImageLabel);

    // 환율 정보
    QVBoxLayout* exchangeInfoLayout = new QVBoxLayout();

    QLabel* exchangeRateTitleLabel = new QLabel("USD/KRW 환율", this);
    exchangeRateTitleLabel->setStyleSheet("QLabel { font-size: 14px; font-weight: bold; }");
    exchangeInfoLayout->addWidget(exchangeRateTitleLabel);

    m_bankExchangeRateLabel = new QLabel("1 USD = --- KRW", this);
    m_bankExchangeRateLabel->setStyleSheet("QLabel { font-size: 18px; font-weight: bold; color: #1E88E5; }");
    exchangeInfoLayout->addWidget(m_bankExchangeRateLabel);

    m_bankExchangeRateChangeLabel = new QLabel("변동률: ---%", this);
    m_bankExchangeRateChangeLabel->setStyleSheet("QLabel { font-size: 12px; color: #666; }");
    exchangeInfoLayout->addWidget(m_bankExchangeRateChangeLabel);

    exchangeInfoLayout->addStretch();
    exchangeRateLayout->addLayout(exchangeInfoLayout);
    exchangeRateLayout->addStretch();

    // bank_vlayout에 패널들 추가
    QVBoxLayout* bankLayout = qobject_cast<QVBoxLayout*>(ui->tab_bank->layout());
    if (bankLayout) {
        bankLayout->insertWidget(1, exchangeRatePanel);  // form 다음에 삽입
        // 구분선 추가
        QFrame* separator = new QFrame(this);
        separator->setFrameShape(QFrame::HLine);
        separator->setFrameShadow(QFrame::Sunken);
        bankLayout->insertWidget(2, separator);
        bankLayout->insertLayout(3, buttonLayout);  // 구분선 다음에 버튼 삽입
    }
    connect(exchangeBtn, &QPushButton::clicked, this, &MainWindow::onExchangeClicked);
    connect(borrowBtn, &QPushButton::clicked, this, &MainWindow::onBorrowClicked);
    connect(repayBtn, &QPushButton::clicked, this, &MainWindow::onRepayClicked);

    QTimer::singleShot(150, this, [this]() {
        refreshPortfolio();
        refreshHistory();
        refreshBank();
        updateExchangeRate();
        refreshTicker();
    });
    QTimer::singleShot(300, this, &MainWindow::setupChart);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupStockCombo()
{
    auto& list = stockList();
    for (const Stock& s : list) {
        if (s.assetType != "FX") {
            QString itemText = s.name + " (" + s.symbol + ")";
            QString imagePath = stockImagePath(s.symbol);

            if (!imagePath.isEmpty()) {
                QString fullPath;
                QStringList pathsToTry = {
                    imagePath,
                    QDir::currentPath() + "/" + imagePath,
                    QCoreApplication::applicationDirPath() + "/" + imagePath,
                    QCoreApplication::applicationDirPath() + "/../../" + imagePath,
                    QCoreApplication::applicationDirPath() + "/../../../" + imagePath,
                };

                for (const QString& tryPath : pathsToTry) {
                    if (QFile::exists(tryPath)) {
                        fullPath = tryPath;
                        break;
                    }
                }

                if (!fullPath.isEmpty()) {
                    QIcon icon(fullPath);
                    ui->trade_symbol_combo->addItem(icon, itemText, s.symbol);
                } else {
                    ui->trade_symbol_combo->addItem(itemText, s.symbol);
                }
            } else {
                ui->trade_symbol_combo->addItem(itemText, s.symbol);
            }
        }
    }
}

void MainWindow::onSymbolChanged(int)
{
    QString symbol = ui->trade_symbol_combo->currentData().toString();
    double price = stockPrice(symbol);
    QString name = stockName(symbol);
    QString assetType = stockAssetType(symbol);

    // 종목 정보 패널 업데이트
    if (m_stockNameLabel && m_stockPriceLabel && m_stockChangeRateLabel && m_stockIconLabel) {
        // 종목명
        m_stockNameLabel->setText(name);

        // 현재가 (자산 타입에 따라 표시 형식 다름)
        QLocale locale(QLocale::Korean);
        QLocale enLocale(QLocale::English);

        if (assetType == "COIN") {
            double usdPrice = price / getUsdKrwRate();
            m_stockPriceLabel->setText(QString("현재가: $%1").arg(enLocale.toString(usdPrice, 'f', 2)));
        } else {
            m_stockPriceLabel->setText(QString("현재가: %1 원").arg(locale.toString((long long)price)));
        }

        // 등락율 계산 (가격 히스토리에서 가장 오래된 가격과 현재 가격 비교)
        QVector<double> history = getPriceHistory(symbol);
        double changeRate = 0.0;
        if (history.size() >= 2) {
            double openPrice = history.first();
            changeRate = ((price - openPrice) / openPrice) * 100.0;
        }

        QColor changeColor = changeRate >= 0 ? QColor("#E53935") : QColor("#1E88E5");  // 빨강/파랑
        m_stockChangeRateLabel->setText(QString("등락율: %1%").arg(QString::number(changeRate, 'f', 2)));
        m_stockChangeRateLabel->setStyleSheet(QString("QLabel { font-size: 12px; color: %1; }").arg(changeColor.name()));

        // 아이콘 업데이트
        QString imagePath = stockImagePath(symbol);
        if (!imagePath.isEmpty()) {
            QString fullPath;

            QStringList pathsToTry = {
                imagePath,
                QDir::currentPath() + "/" + imagePath,
                QCoreApplication::applicationDirPath() + "/" + imagePath,
                QCoreApplication::applicationDirPath() + "/../../" + imagePath,
                QCoreApplication::applicationDirPath() + "/../../../" + imagePath,
            };

            for (const QString& tryPath : pathsToTry) {
                if (QFile::exists(tryPath)) {
                    fullPath = tryPath;
                    break;
                }
            }

            QPixmap pixmap(fullPath);
            qDebug() << "Symbol:" << symbol << "Image path:" << imagePath << "Full path:" << fullPath << "Loaded:" << !pixmap.isNull();
            if (!pixmap.isNull()) {
                QPixmap scaledPixmap = pixmap.scaledToHeight(100, Qt::SmoothTransformation);
                m_stockIconLabel->setPixmap(scaledPixmap);
            } else {
                m_stockIconLabel->setText("❌ 이미지 미로드");
                qWarning() << "Failed to load image, tried:" << pathsToTry;
            }
        } else {
            m_stockIconLabel->setText("⚠️ 경로 없음");
        }
    }

    if (ui->trade_order_type_combo->currentIndex() == 0) {
        ui->trade_price_input->setValue(price);
    }

    if (ui->trade_buy_btn->isChecked()) {
        ui->label_price->setText("매수가격");
    } else {
        ui->label_price->setText("매도가격");
    }

    updateTradeSummary();
    refreshHistory(symbol);
    refreshPendingOrders(symbol);
    QTimer::singleShot(0, this, &MainWindow::setupChart);
}

void MainWindow::onOrderTypeChanged(int index)
{
    if (index == 0) {
        QString symbol = ui->trade_symbol_combo->currentData().toString();
        double price = stockPrice(symbol);
        ui->trade_price_input->setEnabled(false);
        ui->trade_price_input->setValue(price);
    } else {
        ui->trade_price_input->setEnabled(true);
    }

    updateTradeSummary();
}

void MainWindow::onBuyClicked(bool checked)
{
    if (checked) {
        ui->trade_sell_btn->blockSignals(true);
        ui->trade_sell_btn->setChecked(false);
        ui->trade_sell_btn->blockSignals(false);
        ui->label_price->setText("매수가격");
    } else {
        ui->trade_buy_btn->blockSignals(true);
        ui->trade_buy_btn->setChecked(true);
        ui->trade_buy_btn->blockSignals(false);
    }
    updateTradeSummary();
}

void MainWindow::onSellClicked(bool checked)
{
    if (checked) {
        ui->trade_buy_btn->blockSignals(true);
        ui->trade_buy_btn->setChecked(false);
        ui->trade_buy_btn->blockSignals(false);
        ui->label_price->setText("매도가격");
    } else {
        ui->trade_sell_btn->blockSignals(true);
        ui->trade_sell_btn->setChecked(true);
        ui->trade_sell_btn->blockSignals(false);
    }
    updateTradeSummary();
}

void MainWindow::onExecuteClicked()
{
    QString symbol = ui->trade_symbol_combo->currentData().toString();
    bool isBuy = ui->trade_buy_btn->isChecked();
    int qty = ui->trade_qty_spin->value();

    double price;
    QString orderType;

    if (ui->trade_order_type_combo->currentIndex() == 0) {
        orderType = "MARKET";
        price = stockPrice(symbol);
    } else {
        orderType = "LIMIT";
        price = ui->trade_price_input->value();
    }

    bool success = false;
    QString message;

    if (isBuy) {
        success = DatabaseManager::instance().buyStock(symbol, qty, price, orderType);
        message = success ? "주문접수" : "잔액 부족 또는 오류 발생!";
    } else {
        success = DatabaseManager::instance().sellStock(symbol, qty, price, orderType);
        message = success ? "주문접수" : "보유 수량 부족 또는 오류 발생!";
    }

    if (success && orderType == "MARKET") {
        playExecutionSound(isBuy ? "BUY" : "SELL");
    }

    showToast(message, success, isBuy);
    refreshPortfolio();
    refreshHistory(symbol);
    refreshBank();
    refreshPendingOrders(symbol);
}

void MainWindow::onQtyChanged(int)
{
    updateTradeSummary();
}

void MainWindow::onPriceChanged(double)
{
    updateTradeSummary();
}

void MainWindow::updateTradeSummary()
{
    QString symbol = ui->trade_symbol_combo->currentData().toString();
    int qty = ui->trade_qty_spin->value();
    bool isBuy = ui->trade_buy_btn->isChecked();
    QString assetType = stockAssetType(symbol);

    // 시장가면 현재 시장가, 지정가면 사용자 입력 가격 사용
    double price;
    if (ui->trade_order_type_combo->currentIndex() == 0) {
        price = stockPrice(symbol);
    } else {
        price = ui->trade_price_input->value();
    }

    QLocale locale(QLocale::Korean);
    QLocale enLocale(QLocale::English);

    // COIN: USD 표시, STOCK/FX: KRW 표시
    if (assetType == "COIN") {
        double usdPrice = price / getUsdKrwRate();
        double principal = usdPrice * qty;
        double fee = 0.0;
        if (isBuy) {
            fee = principal * DatabaseManager::BUY_FEE_RATE;
        } else {
            fee = principal * (DatabaseManager::SELL_FEE_RATE + DatabaseManager::SELL_TAX_RATE);
        }
        ui->trade_total_label->setText("$" + enLocale.toString(principal, 'f', 2));
        QString feeType = isBuy ? " (수수료)" : " (수수료+세금)";
        ui->trade_fee_label->setText("$" + enLocale.toString(fee, 'f', 2) + feeType);
    } else {
        double principal = price * qty;
        double fee = 0.0;
        if (isBuy) {
            fee = principal * DatabaseManager::BUY_FEE_RATE;
        } else {
            fee = principal * (DatabaseManager::SELL_FEE_RATE + DatabaseManager::SELL_TAX_RATE);
        }
        ui->trade_total_label->setText(locale.toString((long long)principal) + " 원");
        QString feeType = isBuy ? " (수수료)" : " (수수료+세금)";
        ui->trade_fee_label->setText(locale.toString(fee, 'f', 0) + " 원" + feeType);
    }
}

void MainWindow::onTabChanged(int)
{
    refreshPortfolio();
    refreshHistory("");
    refreshBank();
    QString symbol = ui->trade_symbol_combo->currentData().toString();
    refreshPendingOrders(symbol);
}

void MainWindow::refreshPortfolio()
{
    m_portModel->refresh();
    ui->port_table_view->horizontalHeader()->setVisible(true);
    ui->port_table_view->horizontalHeader()->setStretchLastSection(false);
    for (int i = 0; i < m_portModel->columnCount(); ++i) {
        ui->port_table_view->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);
    }
    ui->port_table_view->horizontalHeader()->setStretchLastSection(true);
    for (int i = 0; i < m_portModel->columnCount(); ++i) {
        ui->port_table_view->horizontalHeader()->setSectionResizeMode(i, QHeaderView::ResizeToContents);
    }

    double cash = DatabaseManager::instance().getCash();
    double usdCash = DatabaseManager::instance().getUsdCash();
    auto holdings = DatabaseManager::instance().getHoldings();
    double initialCash = DatabaseManager::instance().getInitialCash();
    int tradeCount = DatabaseManager::instance().getTradeCount();
    QString startDate = DatabaseManager::instance().getStartDate();

    double totalEval = cash;
    double totalCost = 0;
    for (const HoldingRow& h : holdings) {
        totalEval += h.currentPrice * h.quantity;
        totalCost += h.avgPrice * h.quantity;
    }
    double pnl = totalEval - totalCost;
    double profitRate = (totalEval - initialCash) / initialCash * 100.0;

    // 기간 계산
    QDateTime startDateTime = QDateTime::fromString(startDate, "yyyy-MM-dd hh:mm:ss");
    QDateTime now = QDateTime::currentDateTime();
    qint64 elapsed = startDateTime.daysTo(now);
    QString durationStr = QString::number(elapsed) + "일";

    QLocale locale(QLocale::Korean);
    QLocale enLocale(QLocale::English);

    // 수익률 표시 (양수: 빨강, 음수: 파랑)
    QColor profitColor = profitRate >= 0 ? QColor("#E53935") : QColor("#1E88E5");
    QString profitText = QString::number(profitRate, 'f', 2) + "%";
    m_profitRateLabel->setText("수익률: " + profitText);
    m_profitRateLabel->setStyleSheet(QString("QLabel { padding: 5px 10px; font-weight: bold; color: %1; }").arg(profitColor.name()));

    ui->port_cash_label->setText("시작금액: " + locale.toString(initialCash, 'f', 0) + " 원  |  현금(KRW): " + locale.toString(cash, 'f', 0) + " 원 | 현금(USD): $" + enLocale.toString(usdCash, 'f', 2));
    ui->port_total_label->setText("총 평가액: " + locale.toString(totalEval, 'f', 0) + " 원  |  수익률: " + QString::number(profitRate, 'f', 2) + "%");
    ui->port_pnl_label->setText("총 손익: " + locale.toString(pnl, 'f', 0) + " 원  |  거래횟수: " + QString::number(tradeCount) + "회  |  기간: " + durationStr);
}

void MainWindow::refreshHistory(const QString& filterSymbol)
{
    ui->history_table_widget->setRowCount(0);
    ui->history_table_widget->setColumnCount(8);
    ui->history_table_widget->setHorizontalHeaderLabels({"시간", "종목코드", "종목명", "유형", "주문유형", "수량", "가격", "수수료"});

    auto trades = DatabaseManager::instance().getTrades();
    QLocale locale(QLocale::Korean);

    ui->history_table_widget->verticalHeader()->setVisible(false);

    for (const TradeRow& t : trades) {
        if (!filterSymbol.isEmpty() && t.symbol != filterSymbol) continue;

        int row = ui->history_table_widget->rowCount();
        ui->history_table_widget->insertRow(row);

        QColor textColor, bgColor;
        if (t.type == "BUY") {
            textColor = QColor("#E53935");
            bgColor = QColor(229, 57, 53, 31);
        } else {
            textColor = QColor("#1E88E5");
            bgColor = QColor(30, 136, 229, 31);
        }

        QString orderTypeText = (t.orderType == "MARKET") ? "시장가" : "지정가";
        QTableWidgetItem* items[] = {
            new QTableWidgetItem(t.timestamp),
            new QTableWidgetItem(t.symbol),
            new QTableWidgetItem(t.name),
            new QTableWidgetItem(t.type),
            new QTableWidgetItem(orderTypeText),
            new QTableWidgetItem(QString::number(t.quantity)),
            new QTableWidgetItem(locale.toString(t.price, 'f', 0)),
            new QTableWidgetItem(locale.toString(t.fee, 'f', 2))
        };

        for (int col = 0; col < 8; ++col) {
            items[col]->setForeground(textColor);
            items[col]->setBackground(bgColor);
            ui->history_table_widget->setItem(row, col, items[col]);
        }
    }
    ui->history_table_widget->horizontalHeader()->setStretchLastSection(false);
    for (int i = 0; i < 8; ++i) {
        ui->history_table_widget->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);
    }
}

void MainWindow::refreshBank()
{
    QLocale locale(QLocale::Korean);
    QLocale enLocale(QLocale::English);
    auto& db = DatabaseManager::instance();

    double totalFees    = db.getTotalFees();
    double totalTax     = db.getTotalTax();
    double feeOnly      = totalFees - totalTax;

    ui->bank_fee_label->setText(locale.toString(feeOnly, 'f', 0) + " 원");
    ui->bank_tax_label->setText(locale.toString(totalTax, 'f', 0) + " 원");
    ui->bank_total_cost_label->setText(locale.toString(totalFees, 'f', 0) + " 원");

    // 환율 정보 업데이트
    double rate = getUsdKrwRate();
    if (m_bankExchangeRateLabel) {
        m_bankExchangeRateLabel->setText(QString("1 USD = %1 KRW").arg(locale.toString((long long)rate)));
    }

    // 환율 변동률 계산 및 표시
    QVector<double> rateHistory = getPriceHistory("USDKRW");
    if (m_bankExchangeRateChangeLabel && rateHistory.size() >= 2) {
        double openRate = rateHistory.first();
        double changeRate = ((rate - openRate) / openRate) * 100.0;
        QColor changeColor = changeRate >= 0 ? QColor("#E53935") : QColor("#1E88E5");
        m_bankExchangeRateChangeLabel->setText(QString("변동률: %1%").arg(QString::number(changeRate, 'f', 2)));
        m_bankExchangeRateChangeLabel->setStyleSheet(QString("QLabel { font-size: 12px; color: %1; }").arg(changeColor.name()));
    }

    if (!isVisible()) return;
    QTimer::singleShot(0, this, &MainWindow::setupExchangeChart);
}

void MainWindow::setupExchangeChart()
{
    if (!ui->bank_exchange_chart) return;

    QVector<double> history = getPriceHistory("USDKRW");
    if (history.isEmpty()) return;

    // 이전 차트 삭제
    if (ui->bank_exchange_chart->chart()) {
        ui->bank_exchange_chart->chart()->deleteLater();
    }

    QChart* chart = new QChart();
    QLineSeries* series = new QLineSeries();
    series->setColor(Qt::blue);

    // 최근 150개 데이터 사용
    int startIdx = qMax(0, (int)history.size() - 150);
    QVector<double> recentHistory(history.begin() + startIdx, history.end());

    for (int i = 0; i < recentHistory.size(); ++i) {
        series->append(startIdx + i, recentHistory[i]);
    }

    chart->addSeries(series);
    chart->legend()->hide();

    QValueAxis* axisY = new QValueAxis();
    double minRate = *std::min_element(recentHistory.begin(), recentHistory.end());
    double maxRate = *std::max_element(recentHistory.begin(), recentHistory.end());
    double range = (maxRate - minRate) * 0.1;
    axisY->setRange(minRate - range, maxRate + range);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    QCategoryAxis* axisX = new QCategoryAxis();
    for (int i = 0; i < recentHistory.size(); i += 10) {
        axisX->append(QString::number(startIdx + i), startIdx + i);
    }
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    ui->bank_exchange_chart->setChart(chart);
}

void MainWindow::onNextTimeClicked()
{
    for (int i = 0; i < 10; ++i) {
        updateGamePrices();
    }

    DatabaseManager::instance().checkAndExecuteSchedules();

    QString symbol = ui->trade_symbol_combo->currentData().toString();

    // Only update price for market orders, not limit orders
    if (ui->trade_order_type_combo->currentIndex() == 0) {
        double price = stockPrice(symbol);
        ui->trade_price_input->setValue(price);
    }

    refreshPortfolio();
    refreshHistory(symbol);
    refreshPendingOrders(symbol);
    setupChart();
}

void MainWindow::onAutoTimeUpdate()
{
    updateGamePrices();
    QString executedType = DatabaseManager::instance().checkAndExecuteSchedules();

    if (!executedType.isEmpty()) {
        playExecutionSound(executedType);
    }

    // 300초(300 * 1000ms)마다 대출 이자 차감 (8% 연이율)
    m_interestCounter++;
    if (m_interestCounter >= 300) {
        m_interestCounter = 0;
        DatabaseManager::instance().addLoanInterest();
    }

    QString symbol = ui->trade_symbol_combo->currentData().toString();

    // 종목 정보 패널 업데이트 (현재가, 등락율)
    if (m_stockPriceLabel && m_stockChangeRateLabel) {
        double price = stockPrice(symbol);
        QString assetType = stockAssetType(symbol);

        // 현재가 업데이트
        QLocale locale(QLocale::Korean);
        QLocale enLocale(QLocale::English);

        if (assetType == "COIN") {
            double usdPrice = price / getUsdKrwRate();
            m_stockPriceLabel->setText(QString("현재가: $%1").arg(enLocale.toString(usdPrice, 'f', 2)));
        } else {
            m_stockPriceLabel->setText(QString("현재가: %1 원").arg(locale.toString((long long)price)));
        }

        // 등락율 업데이트
        QVector<double> history = getPriceHistory(symbol);
        double changeRate = 0.0;
        if (history.size() >= 2) {
            double openPrice = history.first();
            changeRate = ((price - openPrice) / openPrice) * 100.0;
        }

        QColor changeColor = changeRate >= 0 ? QColor("#E53935") : QColor("#1E88E5");
        m_stockChangeRateLabel->setText(QString("등락율: %1%").arg(QString::number(changeRate, 'f', 2)));
        m_stockChangeRateLabel->setStyleSheet(QString("QLabel { font-size: 12px; color: %1; }").arg(changeColor.name()));
    }

    // Only update price for market orders, not limit orders
    if (ui->trade_order_type_combo->currentIndex() == 0) {
        double price = stockPrice(symbol);
        ui->trade_price_input->setValue(price);
    }

    updateExchangeRate();
    refreshPortfolio();
    refreshHistory(symbol);
    refreshPendingOrders(symbol);
}

void MainWindow::setupChart()
{
    if (!ui->trade_chart_view) return;

    QString symbol = ui->trade_symbol_combo->currentData().toString();
    QVector<CandleData> candles = getCandleData(symbol);

    if (candles.isEmpty()) return;

    // 이전 차트 삭제
    if (ui->trade_chart_view->chart()) {
        ui->trade_chart_view->chart()->deleteLater();
    }

    QChart* chart = new QChart();

    // 최근 15개 봉만 사용
    int startIdx = qMax(0, candles.size() - 15);
    QVector<CandleData> recentCandles(candles.begin() + startIdx, candles.end());

    QCandlestickSeries* series = new QCandlestickSeries();
    series->setIncreasingColor(Qt::red);
    series->setDecreasingColor(Qt::blue);

    for (int i = 0; i < recentCandles.size(); ++i) {
        const CandleData& candle = recentCandles[i];
        QCandlestickSet* set = new QCandlestickSet(candle.open, candle.high, candle.low, candle.close);
        set->setTimestamp(startIdx + i);
        series->append(set);
    }

    chart->addSeries(series);
    chart->legend()->hide();
    QValueAxis* axisY = new QValueAxis();
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    QValueAxis* axisX = new QValueAxis();
    axisX->setRange(startIdx - 0.5, candles.size() - 0.5);
    axisX->setTickCount(recentCandles.size());
    axisX->setLabelFormat("%i");

    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    ui->trade_chart_view->setChart(chart);
}

void MainWindow::showToast(const QString& message, bool isSuccess, bool isBuy)
{
    QWidget* toast = new QWidget(nullptr);
    toast->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    toast->setAttribute(Qt::WA_TranslucentBackground);

    QVBoxLayout* layout = new QVBoxLayout(toast);
    layout->setContentsMargins(16, 12, 16, 12);

    QLabel* label = new QLabel(message, toast);

    QString bgColor;
    if (isSuccess) {
        bgColor = isBuy ? "#E53935" : "#1E88E5";
    } else {
        bgColor = "#FF6F00";
    }

    label->setStyleSheet(
        QString("color: white; background-color: %1; padding: 8px 12px; border-radius: 4px; font-weight: bold;").arg(bgColor)
    );
    layout->addWidget(label);
    toast->setLayout(layout);
    toast->adjustSize();

    QRect windowGeom = this->geometry();
    int x = windowGeom.right() - toast->width() - 20;
    int y = windowGeom.bottom() - toast->height() - 20;
    toast->move(x, y);
    toast->show();

    QTimer* timer = new QTimer(toast);
    connect(timer, &QTimer::timeout, toast, [toast]() {
        toast->close();
        toast->deleteLater();
    });
    timer->setSingleShot(true);
    timer->start(3000);
}

void MainWindow::refreshPendingOrders(const QString& symbol)
{
    ui->trade_pending_table->setRowCount(0);
    ui->trade_pending_table->setColumnCount(7);
    ui->trade_pending_table->setHorizontalHeaderLabels({"번호", "구분", "수량", "지정가", "등록시간", "정정", "취소"});
    ui->trade_pending_table->verticalHeader()->setVisible(false);

    auto schedules = DatabaseManager::instance().getSchedules(symbol);
    QLocale locale(QLocale::Korean);

    for (const ScheduleRow& s : schedules) {
        int row = ui->trade_pending_table->rowCount();
        ui->trade_pending_table->insertRow(row);

        QColor textColor = (s.type == "BUY") ? QColor("#E53935") : QColor("#1E88E5");
        QColor bgColor = (s.type == "BUY") ? QColor(229, 57, 53, 31) : QColor(30, 136, 229, 31);

        auto makeItem = [&](const QString& text) {
            auto* item = new QTableWidgetItem(text);
            item->setForeground(textColor);
            item->setBackground(bgColor);
            item->setData(Qt::UserRole, s.id);
            return item;
        };

        ui->trade_pending_table->setItem(row, 0, makeItem(QString::number(s.id)));
        ui->trade_pending_table->setItem(row, 1, makeItem(s.type));
        ui->trade_pending_table->setItem(row, 2, makeItem(QString::number(s.quantity)));
        ui->trade_pending_table->setItem(row, 3, makeItem(locale.toString(s.targetPrice, 'f', 0)));
        ui->trade_pending_table->setItem(row, 4, makeItem(s.createdAt));

        int scheduleId = s.id;

        QPushButton* modifyBtn = new QPushButton("✎");
        modifyBtn->setStyleSheet("color: #1976D2; border: 1px solid #1976D2; border-radius:3px; padding: 2px;");
        modifyBtn->setMaximumWidth(50);
        connect(modifyBtn, &QPushButton::clicked, this, [this, scheduleId]() {
            modifyScheduleById(scheduleId);
        });
        ui->trade_pending_table->setCellWidget(row, 5, modifyBtn);

        QPushButton* cancelBtn = new QPushButton("✕");
        cancelBtn->setStyleSheet("color: #D32F2F; border: 1px solid #D32F2F; border-radius:3px; padding: 2px;");
        cancelBtn->setMaximumWidth(50);
        connect(cancelBtn, &QPushButton::clicked, this, [this, scheduleId]() {
            cancelScheduleById(scheduleId);
        });
        ui->trade_pending_table->setCellWidget(row, 6, cancelBtn);
    }
    ui->trade_pending_table->horizontalHeader()->setStretchLastSection(false);
    for (int i = 0; i < 7; ++i) {
        ui->trade_pending_table->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);
    }
}

void MainWindow::onModifyScheduleClicked()
{
    auto selectedItems = ui->trade_pending_table->selectedItems();
    if (selectedItems.isEmpty()) {
        return;
    }

    int id = selectedItems.at(0)->data(Qt::UserRole).toInt();
    modifyScheduleById(id);
}

void MainWindow::modifyScheduleById(int id)
{
    auto schedules = DatabaseManager::instance().getSchedules();

    ScheduleRow selectedSchedule;
    bool found = false;
    for (const ScheduleRow& s : schedules) {
        if (s.id == id) {
            selectedSchedule = s;
            found = true;
            break;
        }
    }

    if (!found) {
        QMessageBox::warning(this, "오류", "주문을 찾을 수 없습니다!");
        return;
    }

    QDialog dlg(this);
    dlg.setWindowTitle("주문 정정");
    dlg.setMinimumWidth(300);

    QVBoxLayout vlayout(&dlg);
    QFormLayout flayout;

    QSpinBox qtyInput;
    qtyInput.setMinimum(1);
    qtyInput.setMaximum(100000);
    qtyInput.setValue(selectedSchedule.quantity);
    flayout.addRow("수량:", &qtyInput);

    QSpinBox priceInput;
    priceInput.setMinimum(0);
    priceInput.setMaximum(9999999);
    priceInput.setSingleStep(100);
    priceInput.setValue(selectedSchedule.targetPrice);
    flayout.addRow("지정가:", &priceInput);

    vlayout.addLayout(&flayout);

    QHBoxLayout btnLayout;
    QPushButton okBtn("확인"), cancelBtn("취소");
    btnLayout.addWidget(&okBtn);
    btnLayout.addWidget(&cancelBtn);
    vlayout.addLayout(&btnLayout);

    connect(&okBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
    connect(&cancelBtn, &QPushButton::clicked, &dlg, &QDialog::reject);

    if (dlg.exec() == QDialog::Accepted) {
        if (DatabaseManager::instance().modifySchedule(id, qtyInput.value(), priceInput.value())) {
            refreshPendingOrders(ui->trade_symbol_combo->currentData().toString());
        }
    }
}

void MainWindow::onCancelScheduleClicked()
{
    auto selectedItems = ui->trade_pending_table->selectedItems();
    if (selectedItems.isEmpty()) {
        return;
    }

    int id = selectedItems.at(0)->data(Qt::UserRole).toInt();
    cancelScheduleById(id);
}

void MainWindow::cancelScheduleById(int id)
{
    int ret = QMessageBox::question(this, "확인", "선택한 주문을 취소하시겠습니까?",
                                    QMessageBox::Yes | QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        if (DatabaseManager::instance().cancelSchedule(id)) {
            refreshPendingOrders(ui->trade_symbol_combo->currentData().toString());
        }
    }
}

void MainWindow::onSettingsClicked()
{
    QDialog dlg(this);
    dlg.setWindowTitle("세팅");
    dlg.setMinimumWidth(400);

    QVBoxLayout* layout = new QVBoxLayout(&dlg);

    QFormLayout* formLayout = new QFormLayout();

    QLineEdit* nicknameEdit = new QLineEdit();
    nicknameEdit->setText(DatabaseManager::instance().getNickname());
    QPushButton* saveBtn = new QPushButton("저장");
    QHBoxLayout* nicknameLayout = new QHBoxLayout();
    nicknameLayout->addWidget(nicknameEdit);
    nicknameLayout->addWidget(saveBtn);
    formLayout->addRow("닉네임:", nicknameLayout);

    QLabel* infoLabel = new QLabel();
    QString startDate = DatabaseManager::instance().getStartDate();
    int tradeCount = DatabaseManager::instance().getTradeCount();
    double cash = DatabaseManager::instance().getCash();
    QLocale locale(QLocale::Korean);
    QString infoText = QString("시작: %1 | 거래: %2회 | 현금: %3원")
        .arg(QDateTime::fromString(startDate, "yyyy-MM-dd hh:mm:ss").toString("yyyy-MM-dd"))
        .arg(tradeCount)
        .arg(locale.toString(cash, 'f', 0));
    infoLabel->setText(infoText);
    formLayout->addRow("계정 정보:", infoLabel);

    layout->addLayout(formLayout);

    QFrame* separator1 = new QFrame();
    separator1->setFrameShape(QFrame::HLine);
    layout->addWidget(separator1);

    QPushButton* resetAllBtn = new QPushButton("전체 초기화 (되돌릴 수 없음)");
    resetAllBtn->setStyleSheet("color: white; background-color: #E53935; font-weight: bold;");
    layout->addWidget(resetAllBtn);

    layout->addStretch();

    connect(saveBtn, &QPushButton::clicked, [this, &dlg, nicknameEdit]() {
        QString nickname = nicknameEdit->text().trimmed();
        if (nickname.isEmpty()) {
            QMessageBox::warning(&dlg, "입력 오류", "닉네임을 입력하세요!");
            return;
        }
        if (DatabaseManager::instance().setNickname(nickname)) {
            QMessageBox::information(&dlg, "성공", "닉네임이 저장되었습니다!");
        } else {
            QMessageBox::warning(&dlg, "실패", "닉네임 저장에 실패했습니다!");
        }
    });

    connect(resetAllBtn, &QPushButton::clicked, [this, &dlg]() {
        int ret = QMessageBox::warning(&dlg, "주의",
            "⚠️ 모든 데이터를 초기화합니다!\n\n"
            "거래 기록, 보유 종목, 현금이 모두 삭제되고\n"
            "현금은 5000만원으로 리셋됩니다.\n\n"
            "이 작업은 되돌릴 수 없습니다!",
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);
        if (ret == QMessageBox::Yes) {
            auto& db = DatabaseManager::instance();
            double cash = db.getCash();
            auto holdings = db.getHoldings();
            double initialCash = db.getInitialCash();

            double totalEval = cash;
            for (const HoldingRow& h : holdings)
                totalEval += h.currentPrice * h.quantity;
            double profitRate = (totalEval - initialCash) / initialCash * 100.0;
            db.saveProfit(profitRate);

            if (db.resetAll()) {
                QMessageBox::information(&dlg, "완료", "모든 데이터가 초기화되었습니다!");
                refreshPortfolio();
                refreshHistory("");
                refreshBank();
                updateExchangeRate();
                refreshTicker();
            }
        }
    });

    dlg.exec();
}

void MainWindow::updateExchangeRate()
{
    QLocale locale(QLocale::Korean);
    double rate = getUsdKrwRate();
    QString exchangeText = "💱 USD/KRW: " + locale.toString((long long)rate);
    m_exchangeRateLabel->setText(exchangeText);
}

void MainWindow::refreshTicker()
{
    // 순위 정보만 표시 (환율은 별도 라벨에서 처리)
    auto history = DatabaseManager::instance().getProfitHistory();

    if (history.isEmpty()) {
        m_ticker->setText("🥇 순위 기록 없음 — 전체 초기화 후 기록됩니다");
        return;
    }

    static const QStringList medals = {"🥇", "🥈", "🥉", "4위", "5위"};
    QString rankText;
    int count = qMin(history.size(), 5);
    for (int i = 0; i < count; ++i) {
        const auto& r = history[i];
        QString sign = (r.profitRate >= 0) ? "+" : "";
        QString name = r.nickname.isEmpty() ? "익명" : r.nickname;
        rankText += QString("%1 %2 %3%  ")
                    .arg(medals[i])
                    .arg(name)
                    .arg(sign + QString::number(r.profitRate, 'f', 2));
    }
    m_ticker->setText(rankText.trimmed());
}

void MainWindow::onBorrowClicked()
{
    double currentLoan = DatabaseManager::instance().getLoanAmount();
    QLocale locale(QLocale::Korean);

    // 대출 전용 다이얼로그
    QDialog borrowDialog(this);
    borrowDialog.setWindowTitle("대출받기");
    borrowDialog.setStyleSheet("QDialog { background-color: #f5f5f5; }");

    QVBoxLayout* layout = new QVBoxLayout(&borrowDialog);

    // 현재 대출 금액 표시
    QLabel* currentLoanLabel = new QLabel(QString("현재 대출금: %1원").arg(locale.toString((long long)currentLoan)));
    currentLoanLabel->setStyleSheet("font-weight: bold; color: #7B1FA2;");
    layout->addWidget(currentLoanLabel);

    // 금액 입력 (버튼으로만 조작 가능)
    layout->addWidget(new QLabel("대출할 금액 (최소: 1000만원, 500만원 단위):"));

    int initialAmount = 10000000;  // 1000만원
    QLabel* amountLabel = new QLabel(locale.toString((long long)initialAmount));
    amountLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #7B1FA2; text-align: center;");
    amountLabel->setAlignment(Qt::AlignCenter);

    // +/- 버튼
    QHBoxLayout* amountBtnLayout = new QHBoxLayout();

    QPushButton* decreaseBtn = new QPushButton("-");
    decreaseBtn->setMaximumWidth(60);
    decreaseBtn->setStyleSheet(
        "QPushButton { background-color: #E53935; color: white; font-weight: bold; border-radius: 3px; }"
        "QPushButton:hover { background-color: #D32F2F; }"
    );

    amountBtnLayout->addWidget(decreaseBtn);
    amountBtnLayout->addWidget(amountLabel);

    QPushButton* increaseBtn = new QPushButton("+");
    increaseBtn->setMaximumWidth(60);
    increaseBtn->setStyleSheet(
        "QPushButton { background-color: #1E88E5; color: white; font-weight: bold; border-radius: 3px; }"
        "QPushButton:hover { background-color: #1565C0; }"
    );
    amountBtnLayout->addWidget(increaseBtn);

    layout->addLayout(amountBtnLayout);

    QLabel* unitLabel = new QLabel("(+/- 버튼으로 500만원 단위 조정)");
    unitLabel->setStyleSheet("color: #999; font-size: 9px;");
    layout->addWidget(unitLabel);

    // 금액 저장용 포인터
    int* amount = new int(initialAmount);

    // +/- 버튼 로직
    connect(increaseBtn, &QPushButton::clicked, [=]() {
        *amount += 5000000;
        if (*amount > 500000000) *amount = 500000000;
        amountLabel->setText(locale.toString((long long)*amount));
    });

    connect(decreaseBtn, &QPushButton::clicked, [=]() {
        *amount -= 5000000;
        if (*amount < 10000000) *amount = 10000000;
        amountLabel->setText(locale.toString((long long)*amount));
    });

    // 이자율 표시
    QLabel* interestLabel = new QLabel("이자율: 8% (300초마다 자동 차감)");
    interestLabel->setStyleSheet("color: #666; font-size: 10px;");
    layout->addWidget(interestLabel);

    // 버튼
    QHBoxLayout* btnLayout = new QHBoxLayout();
    QPushButton* okBtn = new QPushButton("대출받기");
    QPushButton* cancelBtn = new QPushButton("취소");
    okBtn->setStyleSheet("QPushButton { background-color: #7B1FA2; color: white; padding: 5px; border-radius: 3px; font-weight: bold; }");
    cancelBtn->setStyleSheet("QPushButton { background-color: #999; color: white; padding: 5px; border-radius: 3px; }");
    btnLayout->addWidget(okBtn);
    btnLayout->addWidget(cancelBtn);
    layout->addLayout(btnLayout);

    connect(okBtn, &QPushButton::clicked, &borrowDialog, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, &borrowDialog, &QDialog::reject);

    if (borrowDialog.exec() != QDialog::Accepted) {
        delete amount;
        return;
    }

    double finalAmount = *amount;
    bool success = DatabaseManager::instance().borrow(finalAmount);
    QString message = success ? QString("대출 완료: %1원").arg(locale.toString((long long)finalAmount))
                              : "대출 실패";

    delete amount;
    showToast(message, success);
    if (success) {
        refreshPortfolio();
        refreshBank();
    }
}

void MainWindow::onRepayClicked()
{
    double currentLoan = DatabaseManager::instance().getLoanAmount();
    QLocale locale(QLocale::Korean);

    if (currentLoan <= 0) {
        showToast("상환할 대출금이 없습니다", false);
        return;
    }

    // 상환 전용 다이얼로그
    QDialog repayDialog(this);
    repayDialog.setWindowTitle("상환하기");
    repayDialog.setStyleSheet("QDialog { background-color: #f5f5f5; }");

    QVBoxLayout* layout = new QVBoxLayout(&repayDialog);

    // 현재 대출 금액 표시
    QLabel* currentLoanLabel = new QLabel(QString("현재 대출금: %1원").arg(locale.toString((long long)currentLoan)));
    currentLoanLabel->setStyleSheet("font-weight: bold; color: #C2185B;");
    layout->addWidget(currentLoanLabel);

    // 금액 입력 (사용자 직접 입력)
    layout->addWidget(new QLabel("상환할 금액:"));

    int maxAmount = (int)currentLoan;
    int initialAmount = (maxAmount >= 10000000) ? 10000000 : maxAmount;

    QSpinBox* amountSpin = new QSpinBox();
    amountSpin->setMaximum(maxAmount);
    amountSpin->setMinimum(1);
    amountSpin->setSingleStep(1);
    amountSpin->setValue(initialAmount);
    amountSpin->setStyleSheet(
        "QSpinBox { padding: 5px; border: 1px solid #ccc; border-radius: 3px; font-size: 12px; }"
    );
    layout->addWidget(amountSpin);

    QLabel* rangeLabel = new QLabel(QString("최소: 1원 | 최대: %1원").arg(locale.toString((long long)maxAmount)));
    rangeLabel->setStyleSheet("color: #999; font-size: 9px;");
    layout->addWidget(rangeLabel);

    // 이자율 표시
    QLabel* interestLabel = new QLabel("이자율: 8% (300초마다 자동 차감)");
    interestLabel->setStyleSheet("color: #666; font-size: 10px;");
    layout->addWidget(interestLabel);

    // 버튼
    QHBoxLayout* btnLayout = new QHBoxLayout();
    QPushButton* okBtn = new QPushButton("상환하기");
    QPushButton* cancelBtn = new QPushButton("취소");
    okBtn->setStyleSheet("QPushButton { background-color: #C2185B; color: white; padding: 5px; border-radius: 3px; font-weight: bold; }");
    cancelBtn->setStyleSheet("QPushButton { background-color: #999; color: white; padding: 5px; border-radius: 3px; }");
    btnLayout->addWidget(okBtn);
    btnLayout->addWidget(cancelBtn);
    layout->addLayout(btnLayout);

    connect(okBtn, &QPushButton::clicked, &repayDialog, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, &repayDialog, &QDialog::reject);

    if (repayDialog.exec() != QDialog::Accepted) {
        return;
    }

    double finalAmount = amountSpin->value();
    bool success = DatabaseManager::instance().repay(finalAmount);
    QString message = success ? QString("상환 완료: %1원").arg(locale.toString((long long)finalAmount))
                              : "상환 실패: 잔액 부족";

    showToast(message, success);
    if (success) {
        refreshPortfolio();
        refreshBank();
    }
}

void MainWindow::onExchangeClicked()
{
    // 환전 방향 선택 다이얼로그
    QDialog exchangeDialog(this);
    exchangeDialog.setWindowTitle("환전 (원 ↔ 달러)");
    exchangeDialog.setStyleSheet("QDialog { background-color: #f5f5f5; }");

    QVBoxLayout* layout = new QVBoxLayout(&exchangeDialog);

    // 환전 방향 선택
    QComboBox* directionCombo = new QComboBox();
    directionCombo->addItem("원 → 달러 환전", "krw_to_usd");
    directionCombo->addItem("달러 → 원 환전", "usd_to_krw");
    layout->addWidget(new QLabel("환전 방향:"));
    layout->addWidget(directionCombo);

    // 금액 입력
    QDoubleSpinBox* amountSpin = new QDoubleSpinBox();
    amountSpin->setMaximum(999999999);
    amountSpin->setValue(0);

    // 환전 방향에 따라 단위 설정
    auto updateUnitLabel = [&]() {
        if (directionCombo->currentData().toString() == "krw_to_usd") {
            amountSpin->setSingleStep(1000);  // 1000원 단위
            amountSpin->setDecimals(0);
            layout->itemAt(3)->widget()->setVisible(true);  // 원 단위 라벨
            layout->itemAt(5)->widget()->setVisible(false); // 달러 단위 라벨
        } else {
            amountSpin->setSingleStep(1);     // 1달러 단위
            amountSpin->setDecimals(2);
            layout->itemAt(3)->widget()->setVisible(false); // 원 단위 라벨
            layout->itemAt(5)->widget()->setVisible(true);  // 달러 단위 라벨
        }
    };

    layout->addWidget(new QLabel("환전할 금액:"));
    layout->addWidget(amountSpin);

    QLabel* krwUnitLabel = new QLabel("(1,000원 단위)");
    krwUnitLabel->setStyleSheet("color: #999; font-size: 9px;");
    layout->addWidget(krwUnitLabel);

    QLabel* usdUnitLabel = new QLabel("(1달러 단위)");
    usdUnitLabel->setStyleSheet("color: #999; font-size: 9px;");
    usdUnitLabel->setVisible(false);
    layout->addWidget(usdUnitLabel);

    // 환율 및 수수료 표시
    double rate = getUsdKrwRate();
    QLocale locale(QLocale::Korean);
    QLocale enLocale(QLocale::English);

    QLabel* infoLabel = new QLabel();
    infoLabel->setStyleSheet("color: #666; font-size: 10px;");
    layout->addWidget(infoLabel);

    auto updateInfo = [&]() {
        double amount = amountSpin->value();
        QString direction = directionCombo->currentData().toString();
        double fee, result;

        if (direction == "krw_to_usd") {
            fee = amount * 0.03;  // 3% 수수료
            double netAmount = amount - fee;
            result = netAmount / rate;
            infoLabel->setText(QString("수수료: %1원 (3%) | 받을 달러: $%2")
                              .arg(locale.toString(fee, 'f', 0))
                              .arg(enLocale.toString(result, 'f', 2)));
        } else {
            fee = amount * 0.03;  // 3% 수수료
            double netAmount = amount - fee;
            result = netAmount * rate;
            infoLabel->setText(QString("수수료: $%1 (3%) | 받을 원화: %2원")
                              .arg(enLocale.toString(fee, 'f', 2))
                              .arg(locale.toString(result, 'f', 0)));
        }
    };

    connect(directionCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [&]() { updateUnitLabel(); updateInfo(); });
    connect(amountSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            [&]() { updateInfo(); });

    QLabel* rateLabel = new QLabel(QString("현재 환율: 1 USD = %1 KRW").arg(locale.toString((long long)rate)));
    rateLabel->setStyleSheet("color: #666; font-size: 10px;");
    layout->insertWidget(layout->count() - 1, rateLabel);

    // 버튼
    QHBoxLayout* btnLayout = new QHBoxLayout();
    QPushButton* okBtn = new QPushButton("환전");
    QPushButton* cancelBtn = new QPushButton("취소");
    okBtn->setStyleSheet("QPushButton { background-color: #FFA726; color: white; padding: 5px; border-radius: 3px; font-weight: bold; }");
    cancelBtn->setStyleSheet("QPushButton { background-color: #999; color: white; padding: 5px; border-radius: 3px; }");
    btnLayout->addWidget(okBtn);
    btnLayout->addWidget(cancelBtn);
    layout->addLayout(btnLayout);

    connect(okBtn, &QPushButton::clicked, &exchangeDialog, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, &exchangeDialog, &QDialog::reject);

    updateUnitLabel();
    updateInfo();

    if (exchangeDialog.exec() != QDialog::Accepted) return;

    double amount = amountSpin->value();
    if (amount <= 0) {
        showToast("0보다 큰 금액을 입력하세요", false);
        return;
    }

    QString direction = directionCombo->currentData().toString();
    bool success = false;
    QString message;

    double fee = amount * 0.03;  // 3% 수수료
    double netAmount = amount - fee;

    if (direction == "krw_to_usd") {
        success = DatabaseManager::instance().exchangeToUsd(netAmount);
        message = success ? QString("원화 %1원 환전 (수수료: %2원) → 달러 수취")
                            .arg(locale.toString(amount, 'f', 0))
                            .arg(locale.toString(fee, 'f', 0))
                          : "환전 실패: 원화 잔액 부족";
    } else {
        success = DatabaseManager::instance().exchangeToKrw(netAmount);
        message = success ? QString("달러 %1 환전 (수수료: $%2) → 원화 수취")
                            .arg(enLocale.toString(amount, 'f', 2))
                            .arg(enLocale.toString(fee, 'f', 2))
                          : "환전 실패: 달러 잔액 부족";
    }

    showToast(message, success);
    if (success) {
        refreshPortfolio();
        refreshBank();
    }
}

void MainWindow::playExecutionSound(const QString& orderType)
{
    QString soundFile;
    if (orderType == "BUY") {
        soundFile = "buy.wav";
    } else if (orderType == "SELL") {
        soundFile = "sell.wav";
    } else {
        return;
    }

    QString basePath = QCoreApplication::applicationDirPath();
    QString soundPath = QDir(basePath).filePath("../../sounds/" + soundFile);
    soundPath = QFileInfo(soundPath).canonicalFilePath();

    if (QFile::exists(soundPath)) {
        m_audioPlayer->setSource(QUrl::fromLocalFile(soundPath));
        m_audioPlayer->play();
    } else {
        qWarning() << "Sound file not found:" << soundPath;
    }
}

