#include "database.h"
#include "stockdata.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QDebug>

DatabaseManager& DatabaseManager::instance()
{
    static DatabaseManager inst;
    return inst;
}

bool DatabaseManager::initDb()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("stock_portfolio.db");
    if (!db.open()) {
        qWarning() << "DB open failed:" << db.lastError().text();
        return false;
    }
    return createTables();
}

bool DatabaseManager::createTables()
{
    QSqlQuery q;

    q.exec("CREATE TABLE IF NOT EXISTS account ("
           "id INTEGER PRIMARY KEY, cash REAL, usd_cash REAL DEFAULT 10000.0, created_at TEXT, nickname TEXT DEFAULT '투자자')");

    q.exec("CREATE TABLE IF NOT EXISTS holdings ("
           "symbol TEXT PRIMARY KEY, quantity INTEGER, avg_price REAL, asset_type TEXT DEFAULT 'STOCK')");

    q.exec("CREATE TABLE IF NOT EXISTS trades ("
           "id INTEGER PRIMARY KEY AUTOINCREMENT, "
           "symbol TEXT, type TEXT, quantity INTEGER, price REAL, fee REAL DEFAULT 0, "
           "timestamp TEXT, asset_type TEXT DEFAULT 'STOCK')");

    q.exec("CREATE TABLE IF NOT EXISTS schedules ("
           "id INTEGER PRIMARY KEY AUTOINCREMENT, "
           "symbol TEXT, type TEXT, quantity INTEGER, target_price REAL, "
           "status TEXT DEFAULT 'PENDING', created_at TEXT)");

    // 기존 테이블에 asset_type 컬럼이 없으면 추가
    QSqlQuery checkCol;
    checkCol.exec("PRAGMA table_info(holdings)");
    bool hasAssetTypeHoldings = false;
    while (checkCol.next()) {
        if (checkCol.value(1).toString() == "asset_type") {
            hasAssetTypeHoldings = true;
            break;
        }
    }
    if (!hasAssetTypeHoldings) {
        q.exec("ALTER TABLE holdings ADD COLUMN asset_type TEXT DEFAULT 'STOCK'");
    }

    QSqlQuery checkCol2;
    checkCol2.exec("PRAGMA table_info(trades)");
    bool hasAssetTypeTrades = false;
    while (checkCol2.next()) {
        if (checkCol2.value(1).toString() == "asset_type") {
            hasAssetTypeTrades = true;
            break;
        }
    }
    if (!hasAssetTypeTrades) {
        q.exec("ALTER TABLE trades ADD COLUMN asset_type TEXT DEFAULT 'STOCK'");
    }

    QSqlQuery checkFee;
    checkFee.exec("PRAGMA table_info(trades)");
    bool hasFeeTrades = false;
    while (checkFee.next()) {
        if (checkFee.value(1).toString() == "fee") {
            hasFeeTrades = true;
            break;
        }
    }
    if (!hasFeeTrades) {
        q.exec("ALTER TABLE trades ADD COLUMN fee REAL DEFAULT 0");
    }

    QSqlQuery checkOrderType;
    checkOrderType.exec("PRAGMA table_info(trades)");
    bool hasOrderType = false;
    while (checkOrderType.next()) {
        if (checkOrderType.value(1).toString() == "order_type") {
            hasOrderType = true;
            break;
        }
    }
    if (!hasOrderType) {
        q.exec("ALTER TABLE trades ADD COLUMN order_type TEXT DEFAULT 'MARKET'");
    }

    QSqlQuery checkNickname;
    checkNickname.exec("PRAGMA table_info(account)");
    bool hasNickname = false;
    while (checkNickname.next()) {
        if (checkNickname.value(1).toString() == "nickname") {
            hasNickname = true;
            break;
        }
    }
    if (!hasNickname) {
        q.exec("ALTER TABLE account ADD COLUMN nickname TEXT DEFAULT '투자자'");
    }

    // usd_cash 컬럼이 없으면 추가 (이중통화)
    QSqlQuery checkUsdCash;
    checkUsdCash.exec("PRAGMA table_info(account)");
    bool hasUsdCash = false;
    while (checkUsdCash.next()) {
        if (checkUsdCash.value(1).toString() == "usd_cash") {
            hasUsdCash = true;
            break;
        }
    }
    if (!hasUsdCash) {
        q.exec("ALTER TABLE account ADD COLUMN usd_cash REAL DEFAULT 10000.0");
    }

    q.exec("CREATE TABLE IF NOT EXISTS profit_history ("
           "id INTEGER PRIMARY KEY AUTOINCREMENT, "
           "nickname TEXT NOT NULL DEFAULT '', "
           "profit_rate REAL NOT NULL, "
           "recorded_at TEXT NOT NULL)");

    q.exec("CREATE TABLE IF NOT EXISTS loan ("
           "id INTEGER PRIMARY KEY, "
           "loan_amount REAL DEFAULT 0, "
           "borrow_rate REAL DEFAULT 0.08, "
           "last_interest_time TEXT)");

    // 계좌가 없으면 초기 현금 5000만원(KRW) + $10,000(USD) 지급
    q.exec("SELECT COUNT(*) FROM account");
    if (q.next() && q.value(0).toInt() == 0) {
        QSqlQuery ins;
        ins.prepare("INSERT INTO account (id, cash, usd_cash, created_at) VALUES (1, 50000000, 10000.0, :t)");
        ins.bindValue(":t", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
        ins.exec();
    }

    // 대출 정보 초기화
    q.exec("SELECT COUNT(*) FROM loan");
    if (q.next() && q.value(0).toInt() == 0) {
        QSqlQuery ins;
        ins.prepare("INSERT INTO loan (id, loan_amount, borrow_rate, last_interest_time) VALUES (1, 0, 0.08, :t)");
        ins.bindValue(":t", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
        ins.exec();
    }

    return true;
}

double DatabaseManager::getCash()
{
    QSqlQuery q("SELECT cash FROM account WHERE id=1");
    return q.next() ? q.value(0).toDouble() : 0.0;
}

double DatabaseManager::getUsdCash()
{
    QSqlQuery q("SELECT usd_cash FROM account WHERE id=1");
    return q.next() ? q.value(0).toDouble() : 0.0;
}

QVector<HoldingRow> DatabaseManager::getHoldings()
{
    QVector<HoldingRow> rows;
    QSqlQuery q("SELECT symbol, quantity, avg_price, asset_type FROM holdings WHERE quantity > 0");
    while (q.next()) {
        HoldingRow r;
        r.symbol       = q.value(0).toString();
        r.name         = stockName(r.symbol);
        r.quantity     = q.value(1).toInt();
        r.avgPrice     = q.value(2).toDouble();
        r.currentPrice = stockPrice(r.symbol);
        r.assetType    = q.value(3).toString().isEmpty() ? "STOCK" : q.value(3).toString();
        rows.append(r);
    }
    return rows;
}

QVector<TradeRow> DatabaseManager::getTrades()
{
    QVector<TradeRow> rows;
    QSqlQuery q("SELECT id, symbol, type, quantity, price, fee, timestamp, asset_type, order_type FROM trades ORDER BY id DESC");
    while (q.next()) {
        TradeRow r;
        r.id        = q.value(0).toInt();
        r.symbol    = q.value(1).toString();
        r.name      = stockName(r.symbol);
        r.type      = q.value(2).toString();
        r.quantity  = q.value(3).toInt();
        r.price     = q.value(4).toDouble();
        r.fee       = q.value(5).toDouble();
        r.timestamp = q.value(6).toString();
        r.assetType = q.value(7).toString().isEmpty() ? "STOCK" : q.value(7).toString();
        r.orderType = q.value(8).toString().isEmpty() ? "MARKET" : q.value(8).toString();
        rows.append(r);
    }
    return rows;
}

bool DatabaseManager::buyStock(const QString& symbol, int qty, double price, const QString& orderType)
{
    // LIMIT 주문은 schedules에 저장
    if (orderType == "LIMIT") {
        return addSchedule(symbol, "BUY", qty, price);
    }

    // asset_type 자동 감지
    QString assetType = stockAssetType(symbol);

    QSqlDatabase::database().transaction();

    // COIN 타입일 때: USD 잔액에서 차감
    if (assetType == "COIN") {
        double usdPrice = price / getUsdKrwRate();
        double principal = usdPrice * qty;
        double fee = principal * BUY_FEE_RATE;
        double cost = principal + fee;
        double usdCash = getUsdCash();
        if (usdCash < cost) {
            QSqlDatabase::database().rollback();
            return false;
        }

        QSqlQuery q;
        q.prepare("UPDATE account SET usd_cash = usd_cash - :cost WHERE id = 1");
        q.bindValue(":cost", cost);
        q.exec();

        // holdings/trades에는 원화 가격으로 저장
        QSqlQuery chk;
        chk.prepare("SELECT quantity, avg_price FROM holdings WHERE symbol = :s");
        chk.bindValue(":s", symbol);
        chk.exec();

        if (chk.next()) {
            int oldQty = chk.value(0).toInt();
            double oldAvg = chk.value(1).toDouble();
            double newAvg = (oldAvg * oldQty + price * qty) / (oldQty + qty);
            QSqlQuery upd;
            upd.prepare("UPDATE holdings SET quantity = quantity + :qty, avg_price = :avg WHERE symbol = :s");
            upd.bindValue(":qty", qty);
            upd.bindValue(":avg", newAvg);
            upd.bindValue(":s", symbol);
            upd.exec();
        } else {
            QSqlQuery ins;
            ins.prepare("INSERT INTO holdings (symbol, quantity, avg_price, asset_type) VALUES (:s, :qty, :p, :t)");
            ins.bindValue(":s", symbol);
            ins.bindValue(":qty", qty);
            ins.bindValue(":p", price);
            ins.bindValue(":t", assetType);
            ins.exec();
        }

        QSqlQuery tr;
        tr.prepare("INSERT INTO trades (symbol, type, quantity, price, fee, timestamp, asset_type, order_type) VALUES (:s,'BUY',:qty,:p,:fee,:ts,:at,:ot)");
        tr.bindValue(":s", symbol);
        tr.bindValue(":qty", qty);
        tr.bindValue(":p", price);
        tr.bindValue(":fee", fee * getUsdKrwRate());
        tr.bindValue(":ts", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
        tr.bindValue(":at", assetType);
        tr.bindValue(":ot", orderType);
        tr.exec();

        QSqlDatabase::database().commit();
        return true;
    }

    // STOCK/FX 타입: KRW 잔액에서 차감 (기존 로직)
    double principal = price * qty;
    double fee = principal * BUY_FEE_RATE;
    double cost = principal + fee;
    double cash = getCash();
    if (cash < cost) {
        QSqlDatabase::database().rollback();
        return false;
    }

    QSqlQuery q;
    q.prepare("UPDATE account SET cash = cash - :cost WHERE id = 1");
    q.bindValue(":cost", cost);
    q.exec();

    // holding 있으면 평균단가 재계산, 없으면 삽입
    QSqlQuery chk;
    chk.prepare("SELECT quantity, avg_price FROM holdings WHERE symbol = :s");
    chk.bindValue(":s", symbol);
    chk.exec();

    if (chk.next()) {
        int    oldQty   = chk.value(0).toInt();
        double oldAvg   = chk.value(1).toDouble();
        double newAvg   = (oldAvg * oldQty + price * qty) / (oldQty + qty);
        QSqlQuery upd;
        upd.prepare("UPDATE holdings SET quantity = quantity + :qty, avg_price = :avg WHERE symbol = :s");
        upd.bindValue(":qty", qty);
        upd.bindValue(":avg", newAvg);
        upd.bindValue(":s", symbol);
        upd.exec();
    } else {
        QSqlQuery ins;
        ins.prepare("INSERT INTO holdings (symbol, quantity, avg_price, asset_type) VALUES (:s, :qty, :p, :t)");
        ins.bindValue(":s", symbol);
        ins.bindValue(":qty", qty);
        ins.bindValue(":p", price);
        ins.bindValue(":t", assetType);
        ins.exec();
    }

    QSqlQuery tr;
    tr.prepare("INSERT INTO trades (symbol, type, quantity, price, fee, timestamp, asset_type, order_type) VALUES (:s,'BUY',:qty,:p,:fee,:ts,:at,:ot)");
    tr.bindValue(":s", symbol);
    tr.bindValue(":qty", qty);
    tr.bindValue(":p", price);
    tr.bindValue(":fee", fee);
    tr.bindValue(":ts", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    tr.bindValue(":at", assetType);
    tr.bindValue(":ot", orderType);
    tr.exec();

    QSqlDatabase::database().commit();
    return true;
}

bool DatabaseManager::sellStock(const QString& symbol, int qty, double price, const QString& orderType)
{
    // LIMIT 주문은 schedules에 저장
    if (orderType == "LIMIT") {
        return addSchedule(symbol, "SELL", qty, price);
    }

    QSqlQuery chk;
    chk.prepare("SELECT quantity FROM holdings WHERE symbol = :s");
    chk.bindValue(":s", symbol);
    chk.exec();
    if (!chk.next() || chk.value(0).toInt() < qty) return false;

    // asset_type 자동 감지
    QString assetType = stockAssetType(symbol);

    QSqlDatabase::database().transaction();

    // COIN 타입일 때: USD 잔액에 추가
    if (assetType == "COIN") {
        double usdPrice = price / getUsdKrwRate();
        double principal = usdPrice * qty;
        double fee = principal * (SELL_FEE_RATE + SELL_TAX_RATE);
        double gain = principal - fee;

        QSqlQuery q;
        q.prepare("UPDATE account SET usd_cash = usd_cash + :gain WHERE id = 1");
        q.bindValue(":gain", gain);
        q.exec();

        QSqlQuery upd;
        upd.prepare("UPDATE holdings SET quantity = quantity - :qty WHERE symbol = :s");
        upd.bindValue(":qty", qty);
        upd.bindValue(":s", symbol);
        upd.exec();

        QSqlQuery tr;
        tr.prepare("INSERT INTO trades (symbol, type, quantity, price, fee, timestamp, asset_type, order_type) VALUES (:s,'SELL',:qty,:p,:fee,:ts,:at,:ot)");
        tr.bindValue(":s", symbol);
        tr.bindValue(":qty", qty);
        tr.bindValue(":p", price);
        tr.bindValue(":fee", fee * getUsdKrwRate());
        tr.bindValue(":ts", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
        tr.bindValue(":at", assetType);
        tr.bindValue(":ot", orderType);
        tr.exec();

        QSqlDatabase::database().commit();
        return true;
    }

    // STOCK/FX 타입: KRW 잔액에 추가 (기존 로직)
    double principal = price * qty;
    double fee = principal * (SELL_FEE_RATE + SELL_TAX_RATE);
    double gain = principal - fee;

    QSqlQuery q;
    q.prepare("UPDATE account SET cash = cash + :gain WHERE id = 1");
    q.bindValue(":gain", gain);
    q.exec();

    QSqlQuery upd;
    upd.prepare("UPDATE holdings SET quantity = quantity - :qty WHERE symbol = :s");
    upd.bindValue(":qty", qty);
    upd.bindValue(":s", symbol);
    upd.exec();

    QSqlQuery tr;
    tr.prepare("INSERT INTO trades (symbol, type, quantity, price, fee, timestamp, asset_type, order_type) VALUES (:s,'SELL',:qty,:p,:fee,:ts,:at,:ot)");
    tr.bindValue(":s", symbol);
    tr.bindValue(":qty", qty);
    tr.bindValue(":p", price);
    tr.bindValue(":fee", fee);
    tr.bindValue(":ts", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    tr.bindValue(":at", assetType);
    tr.bindValue(":ot", orderType);
    tr.exec();

    QSqlDatabase::database().commit();
    return true;
}

int DatabaseManager::getTradeCount()
{
    QSqlQuery q("SELECT COUNT(*) FROM trades");
    return q.next() ? q.value(0).toInt() : 0;
}

QString DatabaseManager::getStartDate()
{
    QSqlQuery q("SELECT created_at FROM account WHERE id = 1");
    if (q.next()) {
        QString createdAt = q.value(0).toString();
        if (!createdAt.isEmpty()) return createdAt;
    }
    return QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
}

double DatabaseManager::getInitialCash()
{
    return 50000000.0;
}

double DatabaseManager::getTotalFees()
{
    QSqlQuery q("SELECT SUM(fee) FROM trades");
    return q.next() ? q.value(0).toDouble() : 0.0;
}

double DatabaseManager::getTotalTax()
{
    QSqlQuery q("SELECT SUM(price * quantity * 0.0015) FROM trades WHERE type = 'SELL'");
    return q.next() ? q.value(0).toDouble() : 0.0;
}

QString DatabaseManager::getNickname()
{
    QSqlQuery q("SELECT nickname FROM account WHERE id = 1");
    if (q.next()) {
        QString nick = q.value(0).toString();
        return nick.isEmpty() ? "투자자" : nick;
    }
    return "투자자";
}

bool DatabaseManager::setNickname(const QString& nickname)
{
    QSqlQuery q;
    q.prepare("UPDATE account SET nickname = :nick WHERE id = 1");
    q.bindValue(":nick", nickname);
    return q.exec();
}

bool DatabaseManager::resetCash()
{
    QSqlQuery q;
    q.prepare("UPDATE account SET cash = 50000000 WHERE id = 1");
    return q.exec();
}

bool DatabaseManager::clearTrades()
{
    QSqlQuery q;
    return q.exec("DELETE FROM trades");
}

bool DatabaseManager::clearHoldings()
{
    QSqlQuery q;
    return q.exec("DELETE FROM holdings");
}

bool DatabaseManager::resetAll()
{
    QSqlDatabase::database().transaction();

    QSqlQuery q1;
    q1.exec("DELETE FROM trades");

    QSqlQuery q2;
    q2.exec("DELETE FROM holdings");

    QSqlQuery q3;
    q3.exec("DELETE FROM schedules");

    QSqlQuery q4;
    q4.prepare("UPDATE account SET cash = 50000000, usd_cash = 10000.0 WHERE id = 1");
    q4.exec();

    QSqlQuery q5;
    q5.prepare("UPDATE loan SET loan_amount = 0, last_interest_time = :t WHERE id = 1");
    q5.bindValue(":t", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    q5.exec();

    QSqlDatabase::database().commit();
    return true;
}

bool DatabaseManager::resetCashOnly()
{
    QSqlQuery q;
    q.prepare("UPDATE account SET cash = 50000000, usd_cash = 10000.0 WHERE id = 1");
    return q.exec();
}

bool DatabaseManager::resetTrades()
{
    QSqlQuery q;
    return q.exec("DELETE FROM trades");
}

bool DatabaseManager::resetHoldings()
{
    QSqlQuery q;
    return q.exec("DELETE FROM holdings");
}

bool DatabaseManager::addSchedule(const QString& symbol, const QString& type, int qty, double targetPrice)
{
    QSqlQuery q;
    q.prepare("INSERT INTO schedules (symbol, type, quantity, target_price, status, created_at) "
              "VALUES (:s, :t, :qty, :tp, 'PENDING', :ct)");
    q.bindValue(":s", symbol);
    q.bindValue(":t", type);
    q.bindValue(":qty", qty);
    q.bindValue(":tp", targetPrice);
    q.bindValue(":ct", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    return q.exec();
}

QVector<ScheduleRow> DatabaseManager::getSchedules(const QString& symbol)
{
    QVector<ScheduleRow> rows;
    QSqlQuery q;
    if (symbol.isEmpty()) {
        q.exec("SELECT id, symbol, type, quantity, target_price, status, created_at "
               "FROM schedules WHERE status = 'PENDING' ORDER BY created_at ASC");
    } else {
        q.prepare("SELECT id, symbol, type, quantity, target_price, status, created_at "
                  "FROM schedules WHERE symbol = :s AND status = 'PENDING' ORDER BY created_at ASC");
        q.bindValue(":s", symbol);
        q.exec();
    }
    while (q.next()) {
        ScheduleRow r;
        r.id         = q.value(0).toInt();
        r.symbol     = q.value(1).toString();
        r.type       = q.value(2).toString();
        r.quantity   = q.value(3).toInt();
        r.targetPrice= q.value(4).toDouble();
        r.status     = q.value(5).toString();
        r.createdAt  = q.value(6).toString();
        rows.append(r);
    }
    return rows;
}

bool DatabaseManager::cancelSchedule(int id)
{
    QSqlQuery q;
    q.prepare("UPDATE schedules SET status = 'CANCELLED' WHERE id = :id");
    q.bindValue(":id", id);
    return q.exec();
}

bool DatabaseManager::modifySchedule(int id, int qty, double targetPrice)
{
    QSqlQuery q;
    q.prepare("UPDATE schedules SET quantity = :qty, target_price = :tp WHERE id = :id");
    q.bindValue(":qty", qty);
    q.bindValue(":tp", targetPrice);
    q.bindValue(":id", id);
    return q.exec();
}

QString DatabaseManager::checkAndExecuteSchedules()
{
    QString lastExecutedType = "";
    QVector<ScheduleRow> schedules = getSchedules();
    for (const ScheduleRow& schedule : schedules) {
        double currentPrice = stockPrice(schedule.symbol);
        bool shouldExecute = false;

        if (schedule.type == "BUY" && currentPrice <= schedule.targetPrice) {
            shouldExecute = true;
        } else if (schedule.type == "SELL" && currentPrice >= schedule.targetPrice) {
            shouldExecute = true;
        }

        if (shouldExecute) {
            bool success = false;
            if (schedule.type == "BUY") {
                success = buyStock(schedule.symbol, schedule.quantity, currentPrice, "MARKET");
            } else {
                success = sellStock(schedule.symbol, schedule.quantity, currentPrice, "MARKET");
            }

            if (success) {
                lastExecutedType = schedule.type;
                QSqlQuery q;
                q.prepare("UPDATE schedules SET status = 'DONE' WHERE id = :id");
                q.bindValue(":id", schedule.id);
                q.exec();
            }
        }
    }
    return lastExecutedType;
}

bool DatabaseManager::saveProfit(double profitRate)
{
    QSqlQuery q;
    q.prepare("INSERT INTO profit_history (nickname, profit_rate, recorded_at) "
              "VALUES (:nn, :pr, :ts)");
    q.bindValue(":nn", getNickname());
    q.bindValue(":pr", profitRate);
    q.bindValue(":ts", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    return q.exec();
}

QVector<ProfitHistoryRow> DatabaseManager::getProfitHistory()
{
    QVector<ProfitHistoryRow> rows;
    QSqlQuery q("SELECT id, nickname, profit_rate, recorded_at "
                "FROM profit_history ORDER BY profit_rate DESC");
    while (q.next()) {
        ProfitHistoryRow r;
        r.id         = q.value(0).toInt();
        r.nickname   = q.value(1).toString();
        r.profitRate = q.value(2).toDouble();
        r.recordedAt = q.value(3).toString();
        rows.append(r);
    }
    return rows;
}

bool DatabaseManager::exchangeToUsd(double krwAmount)
{
    double rate = getUsdKrwRate();
    if (rate <= 0) return false;

    double usdAmount = krwAmount / rate;
    double cash = getCash();
    if (cash < krwAmount) return false;

    QSqlDatabase::database().transaction();

    QSqlQuery q;
    q.prepare("UPDATE account SET cash = cash - :krw, usd_cash = usd_cash + :usd WHERE id = 1");
    q.bindValue(":krw", krwAmount);
    q.bindValue(":usd", usdAmount);
    q.exec();

    QSqlDatabase::database().commit();
    return true;
}

bool DatabaseManager::exchangeToKrw(double usdAmount)
{
    double rate = getUsdKrwRate();
    if (rate <= 0) return false;

    double krwAmount = usdAmount * rate;
    double usdCash = getUsdCash();
    if (usdCash < usdAmount) return false;

    QSqlDatabase::database().transaction();

    QSqlQuery q;
    q.prepare("UPDATE account SET cash = cash + :krw, usd_cash = usd_cash - :usd WHERE id = 1");
    q.bindValue(":krw", krwAmount);
    q.bindValue(":usd", usdAmount);
    q.exec();

    QSqlDatabase::database().commit();
    return true;
}

double DatabaseManager::getLoanAmount()
{
    QSqlQuery q("SELECT loan_amount FROM loan WHERE id = 1");
    return q.next() ? q.value(0).toDouble() : 0.0;
}

bool DatabaseManager::borrow(double amount)
{
    double currentLoan = getLoanAmount();
    double cash = getCash();

    QSqlDatabase::database().transaction();

    QSqlQuery q;
    q.prepare("UPDATE account SET cash = cash + :amount WHERE id = 1");
    q.bindValue(":amount", amount);
    q.exec();

    QSqlQuery q2;
    q2.prepare("UPDATE loan SET loan_amount = :amount, last_interest_time = :t WHERE id = 1");
    q2.bindValue(":amount", currentLoan + amount);
    q2.bindValue(":t", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    q2.exec();

    QSqlDatabase::database().commit();
    return true;
}

bool DatabaseManager::repay(double amount)
{
    double currentLoan = getLoanAmount();
    if (currentLoan < amount) return false;  // 대출 금액보다 많이 상환 불가

    double cash = getCash();
    if (cash < amount) return false;  // 현금 부족

    QSqlDatabase::database().transaction();

    QSqlQuery q;
    q.prepare("UPDATE account SET cash = cash - :amount WHERE id = 1");
    q.bindValue(":amount", amount);
    q.exec();

    QSqlQuery q2;
    q2.prepare("UPDATE loan SET loan_amount = :amount WHERE id = 1");
    q2.bindValue(":amount", currentLoan - amount);
    q2.exec();

    QSqlDatabase::database().commit();
    return true;
}

bool DatabaseManager::addLoanInterest()
{
    double currentLoan = getLoanAmount();
    if (currentLoan <= 0) return true;  // 대출이 없으면 스킵

    double interest = currentLoan * 0.08 / 365 / 24 / 12;  // 일일 8% / 300초 단위
    double cash = getCash();

    // 현금이 충분하면 현금에서 차감, 없으면 대출에 추가
    QSqlDatabase::database().transaction();

    if (cash >= interest) {
        // 현금에서 이자 차감
        QSqlQuery q;
        q.prepare("UPDATE account SET cash = cash - :interest WHERE id = 1");
        q.bindValue(":interest", interest);
        q.exec();
    } else {
        // 현금이 부족하면 이자를 대출금에 추가
        QSqlQuery q;
        q.prepare("UPDATE loan SET loan_amount = loan_amount + :interest WHERE id = 1");
        q.bindValue(":interest", interest);
        q.exec();
    }

    QSqlQuery q2;
    q2.prepare("UPDATE loan SET last_interest_time = :t WHERE id = 1");
    q2.bindValue(":t", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    q2.exec();

    QSqlDatabase::database().commit();
    return true;
}
