#include "QmlStandardItemModel.h"
#include "QmlUtils.h"
#include <QtQml>

QmlStandardItemModel::QmlStandardItemModel(QObject *parent)
    : QStandardItemModel(parent)
{
    connect(this, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SIGNAL(countChanged()));
    connect(this, SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SIGNAL(countChanged()));
    connect(this, SIGNAL(modelReset()), this, SIGNAL(countChanged()));
}

int QmlStandardItemModel::count() const
{
    return rowCount();
}

void QmlStandardItemModel::beginResetModel()
{
    QStandardItemModel::beginResetModel();
}

void QmlStandardItemModel::endResetModel()
{
    QStandardItemModel::endResetModel();
}

QVariantMap QmlStandardItemModel::get(int idx)
{
    QVariantMap value;
    if (idx >= 0 && idx < count()) {
        QHash<int, QByteArray> roles = roleNames();
        QHashIterator<int, QByteArray> it(roles);
        while (it.hasNext()) {
            it.next();
            value.insert(QString::fromUtf8(it.value()), data(index(idx, 0), it.key()));
        }
    }
    return value;
}

QVariantMap QmlStandardItemModel::dataFromIndex(const QModelIndex &index)
{
    QVariantMap value;
    if (index.isValid()) {
        QHash<int, QByteArray> roles = roleNames();
        QHashIterator<int, QByteArray> it(roles);
        while (it.hasNext()) {
            it.next();
            value.insert(QString::fromUtf8(it.value()), index.data(it.key()));
        }
    }
    return value;
}
