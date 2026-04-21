/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.11.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCharts/QChartView>
#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTableView>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QVBoxLayout *main_vlayout;
    QTabWidget *tabWidget;
    QWidget *tab_portfolio;
    QVBoxLayout *port_vlayout;
    QVBoxLayout *port_summary_vlayout;
    QHBoxLayout *port_summary_hlayout1;
    QLabel *port_cash_label;
    QSpacerItem *port_summary_spacer1;
    QHBoxLayout *port_summary_hlayout2;
    QLabel *port_total_label;
    QSpacerItem *port_summary_spacer2;
    QHBoxLayout *port_summary_hlayout3;
    QLabel *port_pnl_label;
    QSpacerItem *port_summary_spacer3;
    QHBoxLayout *port_content_hlayout;
    QTableView *port_table_view;
    QTableWidget *history_table_widget;
    QWidget *tab_trade;
    QVBoxLayout *trade_vlayout;
    QChartView *trade_chart_view;
    QHBoxLayout *trade_bottom_hlayout;
    QVBoxLayout *trade_left_vlayout;
    QFormLayout *trade_form;
    QLabel *label_symbol;
    QComboBox *trade_symbol_combo;
    QLabel *label_order_type;
    QComboBox *trade_order_type_combo;
    QLabel *label_price;
    QDoubleSpinBox *trade_price_input;
    QLabel *label_qty;
    QSpinBox *trade_qty_spin;
    QLabel *label_total_cost;
    QLabel *trade_total_label;
    QLabel *label_fee_preview;
    QLabel *trade_fee_label;
    QHBoxLayout *trade_type_hlayout;
    QPushButton *trade_buy_btn;
    QPushButton *trade_sell_btn;
    QHBoxLayout *trade_button_hlayout;
    QPushButton *trade_execute_btn;
    QPushButton *trade_next_time_btn;
    QSpacerItem *trade_left_spacer;
    QGroupBox *trade_pending_group;
    QVBoxLayout *pending_vlayout;
    QTableWidget *trade_pending_table;
    QHBoxLayout *pending_button_hlayout;
    QPushButton *trade_modify_btn;
    QPushButton *trade_cancel_btn;
    QSpacerItem *pending_button_spacer;
    QWidget *tab_bank;
    QVBoxLayout *bank_vlayout;
    QFormLayout *bank_form;
    QLabel *label_bank_fee_title;
    QLabel *bank_fee_label;
    QLabel *label_bank_tax_title;
    QLabel *bank_tax_label;
    QLabel *label_bank_total_cost_title;
    QLabel *bank_total_cost_label;
    QChartView *bank_exchange_chart;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(1000, 754);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        centralwidget->setAutoFillBackground(true);
        main_vlayout = new QVBoxLayout(centralwidget);
        main_vlayout->setObjectName("main_vlayout");
        tabWidget = new QTabWidget(centralwidget);
        tabWidget->setObjectName("tabWidget");
        tab_portfolio = new QWidget();
        tab_portfolio->setObjectName("tab_portfolio");
        port_vlayout = new QVBoxLayout(tab_portfolio);
        port_vlayout->setObjectName("port_vlayout");
        port_summary_vlayout = new QVBoxLayout();
        port_summary_vlayout->setObjectName("port_summary_vlayout");
        port_summary_hlayout1 = new QHBoxLayout();
        port_summary_hlayout1->setObjectName("port_summary_hlayout1");
        port_cash_label = new QLabel(tab_portfolio);
        port_cash_label->setObjectName("port_cash_label");
        QFont font;
        font.setPointSize(11);
        font.setBold(true);
        port_cash_label->setFont(font);

        port_summary_hlayout1->addWidget(port_cash_label);

        port_summary_spacer1 = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        port_summary_hlayout1->addItem(port_summary_spacer1);


        port_summary_vlayout->addLayout(port_summary_hlayout1);

        port_summary_hlayout2 = new QHBoxLayout();
        port_summary_hlayout2->setObjectName("port_summary_hlayout2");
        port_total_label = new QLabel(tab_portfolio);
        port_total_label->setObjectName("port_total_label");
        port_total_label->setFont(font);

        port_summary_hlayout2->addWidget(port_total_label);

        port_summary_spacer2 = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        port_summary_hlayout2->addItem(port_summary_spacer2);


        port_summary_vlayout->addLayout(port_summary_hlayout2);

        port_summary_hlayout3 = new QHBoxLayout();
        port_summary_hlayout3->setObjectName("port_summary_hlayout3");
        port_pnl_label = new QLabel(tab_portfolio);
        port_pnl_label->setObjectName("port_pnl_label");
        port_pnl_label->setFont(font);

        port_summary_hlayout3->addWidget(port_pnl_label);

        port_summary_spacer3 = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        port_summary_hlayout3->addItem(port_summary_spacer3);


        port_summary_vlayout->addLayout(port_summary_hlayout3);


        port_vlayout->addLayout(port_summary_vlayout);

        port_content_hlayout = new QHBoxLayout();
        port_content_hlayout->setObjectName("port_content_hlayout");
        port_table_view = new QTableView(tab_portfolio);
        port_table_view->setObjectName("port_table_view");
        port_table_view->setEditTriggers(QAbstractItemView::EditTrigger::NoEditTriggers);
        port_table_view->setAlternatingRowColors(true);
        port_table_view->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);

        port_content_hlayout->addWidget(port_table_view);

        history_table_widget = new QTableWidget(tab_portfolio);
        if (history_table_widget->columnCount() < 7)
            history_table_widget->setColumnCount(7);
        history_table_widget->setObjectName("history_table_widget");
        history_table_widget->setEditTriggers(QAbstractItemView::EditTrigger::NoEditTriggers);
        history_table_widget->setAlternatingRowColors(true);
        history_table_widget->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
        history_table_widget->setColumnCount(7);

        port_content_hlayout->addWidget(history_table_widget);


        port_vlayout->addLayout(port_content_hlayout);

        tabWidget->addTab(tab_portfolio, QString());
        tab_trade = new QWidget();
        tab_trade->setObjectName("tab_trade");
        trade_vlayout = new QVBoxLayout(tab_trade);
        trade_vlayout->setObjectName("trade_vlayout");
        trade_chart_view = new QChartView(tab_trade);
        trade_chart_view->setObjectName("trade_chart_view");

        trade_vlayout->addWidget(trade_chart_view);

        trade_bottom_hlayout = new QHBoxLayout();
        trade_bottom_hlayout->setObjectName("trade_bottom_hlayout");
        trade_left_vlayout = new QVBoxLayout();
        trade_left_vlayout->setObjectName("trade_left_vlayout");
        trade_form = new QFormLayout();
        trade_form->setObjectName("trade_form");
        label_symbol = new QLabel(tab_trade);
        label_symbol->setObjectName("label_symbol");

        trade_form->setWidget(0, QFormLayout::ItemRole::LabelRole, label_symbol);

        trade_symbol_combo = new QComboBox(tab_trade);
        trade_symbol_combo->setObjectName("trade_symbol_combo");
        trade_symbol_combo->setAutoFillBackground(false);

        trade_form->setWidget(0, QFormLayout::ItemRole::FieldRole, trade_symbol_combo);

        label_order_type = new QLabel(tab_trade);
        label_order_type->setObjectName("label_order_type");

        trade_form->setWidget(1, QFormLayout::ItemRole::LabelRole, label_order_type);

        trade_order_type_combo = new QComboBox(tab_trade);
        trade_order_type_combo->addItem(QString());
        trade_order_type_combo->addItem(QString());
        trade_order_type_combo->setObjectName("trade_order_type_combo");

        trade_form->setWidget(1, QFormLayout::ItemRole::FieldRole, trade_order_type_combo);

        label_price = new QLabel(tab_trade);
        label_price->setObjectName("label_price");

        trade_form->setWidget(2, QFormLayout::ItemRole::LabelRole, label_price);

        trade_price_input = new QDoubleSpinBox(tab_trade);
        trade_price_input->setObjectName("trade_price_input");
        trade_price_input->setDecimals(2);
        trade_price_input->setMinimum(0.000000000000000);
        trade_price_input->setMaximum(99999999.000000000000000);
        trade_price_input->setSingleStep(0.010000000000000);
        trade_price_input->setValue(0.000000000000000);

        trade_form->setWidget(2, QFormLayout::ItemRole::FieldRole, trade_price_input);

        label_qty = new QLabel(tab_trade);
        label_qty->setObjectName("label_qty");

        trade_form->setWidget(3, QFormLayout::ItemRole::LabelRole, label_qty);

        trade_qty_spin = new QSpinBox(tab_trade);
        trade_qty_spin->setObjectName("trade_qty_spin");
        trade_qty_spin->setMinimum(1);
        trade_qty_spin->setMaximum(100000);
        trade_qty_spin->setValue(1);

        trade_form->setWidget(3, QFormLayout::ItemRole::FieldRole, trade_qty_spin);

        label_total_cost = new QLabel(tab_trade);
        label_total_cost->setObjectName("label_total_cost");

        trade_form->setWidget(4, QFormLayout::ItemRole::LabelRole, label_total_cost);

        trade_total_label = new QLabel(tab_trade);
        trade_total_label->setObjectName("trade_total_label");

        trade_form->setWidget(4, QFormLayout::ItemRole::FieldRole, trade_total_label);

        label_fee_preview = new QLabel(tab_trade);
        label_fee_preview->setObjectName("label_fee_preview");

        trade_form->setWidget(5, QFormLayout::ItemRole::LabelRole, label_fee_preview);

        trade_fee_label = new QLabel(tab_trade);
        trade_fee_label->setObjectName("trade_fee_label");

        trade_form->setWidget(5, QFormLayout::ItemRole::FieldRole, trade_fee_label);


        trade_left_vlayout->addLayout(trade_form);

        trade_type_hlayout = new QHBoxLayout();
        trade_type_hlayout->setObjectName("trade_type_hlayout");
        trade_buy_btn = new QPushButton(tab_trade);
        trade_buy_btn->setObjectName("trade_buy_btn");
        trade_buy_btn->setCheckable(true);
        trade_buy_btn->setChecked(true);

        trade_type_hlayout->addWidget(trade_buy_btn);

        trade_sell_btn = new QPushButton(tab_trade);
        trade_sell_btn->setObjectName("trade_sell_btn");
        trade_sell_btn->setCheckable(true);

        trade_type_hlayout->addWidget(trade_sell_btn);


        trade_left_vlayout->addLayout(trade_type_hlayout);

        trade_button_hlayout = new QHBoxLayout();
        trade_button_hlayout->setObjectName("trade_button_hlayout");
        trade_execute_btn = new QPushButton(tab_trade);
        trade_execute_btn->setObjectName("trade_execute_btn");

        trade_button_hlayout->addWidget(trade_execute_btn);

        trade_next_time_btn = new QPushButton(tab_trade);
        trade_next_time_btn->setObjectName("trade_next_time_btn");

        trade_button_hlayout->addWidget(trade_next_time_btn);


        trade_left_vlayout->addLayout(trade_button_hlayout);

        trade_left_spacer = new QSpacerItem(0, 0, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        trade_left_vlayout->addItem(trade_left_spacer);


        trade_bottom_hlayout->addLayout(trade_left_vlayout);

        trade_pending_group = new QGroupBox(tab_trade);
        trade_pending_group->setObjectName("trade_pending_group");
        QSizePolicy sizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
        sizePolicy.setHorizontalStretch(2);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(trade_pending_group->sizePolicy().hasHeightForWidth());
        trade_pending_group->setSizePolicy(sizePolicy);
        pending_vlayout = new QVBoxLayout(trade_pending_group);
        pending_vlayout->setObjectName("pending_vlayout");
        trade_pending_table = new QTableWidget(trade_pending_group);
        if (trade_pending_table->columnCount() < 5)
            trade_pending_table->setColumnCount(5);
        trade_pending_table->setObjectName("trade_pending_table");
        trade_pending_table->setEditTriggers(QAbstractItemView::EditTrigger::NoEditTriggers);
        trade_pending_table->setAlternatingRowColors(true);
        trade_pending_table->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
        trade_pending_table->setColumnCount(5);

        pending_vlayout->addWidget(trade_pending_table);

        pending_button_hlayout = new QHBoxLayout();
        pending_button_hlayout->setObjectName("pending_button_hlayout");
        trade_modify_btn = new QPushButton(trade_pending_group);
        trade_modify_btn->setObjectName("trade_modify_btn");
        trade_modify_btn->setEnabled(false);

        pending_button_hlayout->addWidget(trade_modify_btn);

        trade_cancel_btn = new QPushButton(trade_pending_group);
        trade_cancel_btn->setObjectName("trade_cancel_btn");
        trade_cancel_btn->setEnabled(false);

        pending_button_hlayout->addWidget(trade_cancel_btn);

        pending_button_spacer = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        pending_button_hlayout->addItem(pending_button_spacer);


        pending_vlayout->addLayout(pending_button_hlayout);


        trade_bottom_hlayout->addWidget(trade_pending_group);


        trade_vlayout->addLayout(trade_bottom_hlayout);

        tabWidget->addTab(tab_trade, QString());
        tab_bank = new QWidget();
        tab_bank->setObjectName("tab_bank");
        bank_vlayout = new QVBoxLayout(tab_bank);
        bank_vlayout->setObjectName("bank_vlayout");
        bank_form = new QFormLayout();
        bank_form->setObjectName("bank_form");
        label_bank_fee_title = new QLabel(tab_bank);
        label_bank_fee_title->setObjectName("label_bank_fee_title");

        bank_form->setWidget(0, QFormLayout::ItemRole::LabelRole, label_bank_fee_title);

        bank_fee_label = new QLabel(tab_bank);
        bank_fee_label->setObjectName("bank_fee_label");
        QFont font1;
        font1.setPointSize(12);
        font1.setBold(true);
        bank_fee_label->setFont(font1);

        bank_form->setWidget(0, QFormLayout::ItemRole::FieldRole, bank_fee_label);

        label_bank_tax_title = new QLabel(tab_bank);
        label_bank_tax_title->setObjectName("label_bank_tax_title");

        bank_form->setWidget(1, QFormLayout::ItemRole::LabelRole, label_bank_tax_title);

        bank_tax_label = new QLabel(tab_bank);
        bank_tax_label->setObjectName("bank_tax_label");
        bank_tax_label->setFont(font1);

        bank_form->setWidget(1, QFormLayout::ItemRole::FieldRole, bank_tax_label);

        label_bank_total_cost_title = new QLabel(tab_bank);
        label_bank_total_cost_title->setObjectName("label_bank_total_cost_title");

        bank_form->setWidget(2, QFormLayout::ItemRole::LabelRole, label_bank_total_cost_title);

        bank_total_cost_label = new QLabel(tab_bank);
        bank_total_cost_label->setObjectName("bank_total_cost_label");
        bank_total_cost_label->setFont(font1);

        bank_form->setWidget(2, QFormLayout::ItemRole::FieldRole, bank_total_cost_label);


        bank_vlayout->addLayout(bank_form);

        bank_exchange_chart = new QChartView(tab_bank);
        bank_exchange_chart->setObjectName("bank_exchange_chart");

        bank_vlayout->addWidget(bank_exchange_chart);

        tabWidget->addTab(tab_bank, QString());

        main_vlayout->addWidget(tabWidget);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 1000, 22));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        tabWidget->setCurrentIndex(2);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "\353\252\250\354\235\230\355\210\254\354\236\220 \354\225\261", nullptr));
        port_cash_label->setText(QCoreApplication::translate("MainWindow", "\354\213\234\354\236\221\352\270\210\354\225\241: --- | \355\230\204\352\270\210: ---", nullptr));
        port_total_label->setText(QCoreApplication::translate("MainWindow", "\354\264\235 \355\217\211\352\260\200\354\225\241: --- | \354\210\230\354\235\265\353\245\240: ---%", nullptr));
        port_pnl_label->setText(QCoreApplication::translate("MainWindow", "\354\206\220\354\235\265: --- | \352\261\260\353\236\230\355\232\237\354\210\230: --- | \352\270\260\352\260\204: ---", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_portfolio), QCoreApplication::translate("MainWindow", "\355\217\254\355\212\270\355\217\264\353\246\254\354\230\244", nullptr));
        label_symbol->setText(QCoreApplication::translate("MainWindow", "\354\242\205\353\252\251", nullptr));
        label_order_type->setText(QCoreApplication::translate("MainWindow", "\354\243\274\353\254\270 \354\234\240\355\230\225", nullptr));
        trade_order_type_combo->setItemText(0, QCoreApplication::translate("MainWindow", "\354\213\234\354\236\245\352\260\200", nullptr));
        trade_order_type_combo->setItemText(1, QCoreApplication::translate("MainWindow", "\354\247\200\354\240\225\352\260\200", nullptr));

        label_price->setText(QCoreApplication::translate("MainWindow", "\353\247\244\354\210\230\352\260\200\352\262\251", nullptr));
        label_qty->setText(QCoreApplication::translate("MainWindow", "\354\210\230\353\237\211", nullptr));
        label_total_cost->setText(QCoreApplication::translate("MainWindow", "\354\264\235 \352\270\210\354\225\241", nullptr));
        trade_total_label->setText(QCoreApplication::translate("MainWindow", "---", nullptr));
        label_fee_preview->setText(QCoreApplication::translate("MainWindow", "\354\230\210\354\203\201 \354\210\230\354\210\230\353\243\214", nullptr));
        trade_fee_label->setText(QCoreApplication::translate("MainWindow", "---", nullptr));
        trade_buy_btn->setText(QCoreApplication::translate("MainWindow", "\353\247\244\354\210\230", nullptr));
        trade_sell_btn->setText(QCoreApplication::translate("MainWindow", "\353\247\244\353\217\204", nullptr));
        trade_execute_btn->setText(QCoreApplication::translate("MainWindow", "\354\243\274\353\254\270\354\240\204\354\206\241", nullptr));
        trade_next_time_btn->setText(QCoreApplication::translate("MainWindow", "\354\213\234\352\260\204 \352\262\275\352\263\274 \342\206\222", nullptr));
        trade_pending_group->setTitle(QCoreApplication::translate("MainWindow", "\353\257\270\354\262\264\352\262\260 \354\243\274\353\254\270", nullptr));
        trade_modify_btn->setText(QCoreApplication::translate("MainWindow", "\354\240\225\354\240\225", nullptr));
        trade_cancel_btn->setText(QCoreApplication::translate("MainWindow", "\354\267\250\354\206\214", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_trade), QCoreApplication::translate("MainWindow", "\353\247\244\353\247\244", nullptr));
        label_bank_fee_title->setText(QCoreApplication::translate("MainWindow", "\353\210\204\354\240\201 \354\210\230\354\210\230\353\243\214", nullptr));
        bank_fee_label->setText(QCoreApplication::translate("MainWindow", "---", nullptr));
        label_bank_tax_title->setText(QCoreApplication::translate("MainWindow", "\353\210\204\354\240\201 \352\261\260\353\236\230\354\204\270", nullptr));
        bank_tax_label->setText(QCoreApplication::translate("MainWindow", "---", nullptr));
        label_bank_total_cost_title->setText(QCoreApplication::translate("MainWindow", "\354\264\235 \353\271\204\354\232\251 (\354\210\230\354\210\230\353\243\214+\354\204\270\352\270\210)", nullptr));
        bank_total_cost_label->setText(QCoreApplication::translate("MainWindow", "---", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_bank), QCoreApplication::translate("MainWindow", "\354\235\200\355\226\211", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
