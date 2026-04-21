#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include <QTimer>
#include <QLabel>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QPixmap>
#include <QPainter>

#include "portfoliomodel.h"
#include "tickerwidget.h"
#include "tradeservice.h"
#include "exchangeservice.h"
#include "scheduleservice.h"
#include "exceptions.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void initializeUI();
    void setupUIElements();
    void onSymbolChanged(int index);
    void onQtyChanged(int value);
    void onPriceChanged(double value);
    void onOrderTypeChanged(int index);
    void onBuyClicked(bool checked);
    void onSellClicked(bool checked);
    void onExecuteClicked();
    void onTabChanged(int index);
    void onNextTimeClicked();
    void onAutoTimeUpdate();
    void onModifyScheduleClicked();
    void onCancelScheduleClicked();
    void onSettingsClicked();
    void onExchangeClicked();
    void onBorrowClicked();
    void onRepayClicked();

private:
    void setupStockCombo();
    void refreshPortfolio();
    void refreshHistory(const QString& filterSymbol = "");
    void refreshBank();
    void updateTradeSummary();
    void refreshPendingOrders(const QString& symbol = "");
    void showToast(const QString& message, bool isSuccess, bool isBuy = true);
    void refreshTicker();
    void modifyScheduleById(int id);
    void cancelScheduleById(int id);
    void playExecutionSound(const QString& orderType);

    Ui::MainWindow   *ui;
    PortfolioModel   *m_portModel;
    QTimer           *m_priceTimer;
    TickerWidget     *m_ticker;
    QLabel           *m_profitRateLabel;
    QLabel           *m_exchangeRateLabel;
    QLabel           *m_stockIconLabel;
    QLabel           *m_stockNameLabel;
    QLabel           *m_stockPriceLabel;
    QLabel           *m_stockChangeRateLabel;
    QLabel           *m_bankExchangeRateLabel;
    QLabel           *m_bankExchangeRateChangeLabel;
    int              m_interestCounter;  // 300초(300 * 1000ms) 카운터
    QMediaPlayer     *m_audioPlayer;
    QAudioOutput     *m_audioOutput;
    void setupChart();
    void setupExchangeChart();
    void updateExchangeRate();
};

#endif // MAINWINDOW_H
