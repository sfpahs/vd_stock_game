#ifndef PORTFOLIOMODEL_H
#define PORTFOLIOMODEL_H

#include <QAbstractTableModel>
#include "database.h"

class PortfolioModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit PortfolioModel(QObject* parent = nullptr);

    void refresh();

    int rowCount   (const QModelIndex& = {}) const override;
    int columnCount(const QModelIndex& = {}) const override;
    QVariant data       (const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData (int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
    QVector<HoldingRow> m_rows;
};

#endif // PORTFOLIOMODEL_H
