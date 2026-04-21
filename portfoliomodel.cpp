#include "portfoliomodel.h"
#include "stockdata.h"
#include <QColor>
#include <QLocale>

static const QStringList HEADERS = {
    "종목코드", "종목명", "수량", "평균단가", "현재가", "평가손익", "수익률(%)"
};

PortfolioModel::PortfolioModel(QObject* parent)
    : QAbstractTableModel(parent)
{}

void PortfolioModel::refresh()
{
    beginResetModel();
    m_rows = DatabaseManager::instance().getHoldings();
    endResetModel();
}

int PortfolioModel::rowCount(const QModelIndex&) const { return m_rows.size(); }
int PortfolioModel::columnCount(const QModelIndex&) const { return HEADERS.size(); }

QVariant PortfolioModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
        return HEADERS.value(section);
    return {};
}

QVariant PortfolioModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= m_rows.size()) return {};

    const HoldingRow& r   = m_rows[index.row()];
    double rate = getUsdKrwRate();
    bool isCoin = (r.assetType == "COIN");

    // COIN: USD 단위로 표시
    double displayPrice = isCoin ? r.currentPrice / rate : r.currentPrice;
    double displayAvgPrice = isCoin ? r.avgPrice / rate : r.avgPrice;
    double pnl = (displayPrice - displayAvgPrice) * r.quantity;
    double pnlPct = displayAvgPrice > 0 ? (displayPrice - displayAvgPrice) / displayAvgPrice * 100.0 : 0.0;

    QLocale locale(QLocale::Korean);
    QLocale enLocale(QLocale::English);

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case 0: return r.symbol;
        case 1: return r.name;
        case 2: return r.quantity;
        case 3: return isCoin ? ("$" + enLocale.toString(displayAvgPrice, 'f', 2)) : locale.toString(displayAvgPrice, 'f', 0);
        case 4: return isCoin ? ("$" + enLocale.toString(displayPrice, 'f', 2)) : locale.toString(displayPrice, 'f', 0);
        case 5: return isCoin ? ("$" + enLocale.toString(pnl, 'f', 2)) : locale.toString(pnl, 'f', 0);
        case 6: return QString::number(pnlPct, 'f', 2) + "%";
        }
    }

    if (role == Qt::ForegroundRole && (index.column() == 5 || index.column() == 6)) {
        if (pnl > 0)  return QColor(229, 57, 53);
        if (pnl < 0)  return QColor(30, 136, 229);
    }

    if (role == Qt::TextAlignmentRole && index.column() >= 2)
        return int(Qt::AlignRight | Qt::AlignVCenter);

    return {};
}
