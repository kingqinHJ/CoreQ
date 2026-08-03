#ifndef QMLSTANDARDITEMMODEL_H
#define QMLSTANDARDITEMMODEL_H

#include <QStandardItemModel>
#include <QJSValue>

class QmlStandardItemModel : public QStandardItemModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    explicit QmlStandardItemModel(QObject *parent = nullptr);

    int count() const;

    void beginResetModel();
    void endResetModel();

public slots:
    QVariantMap get(int idx);
    QVariantMap dataFromIndex(const QModelIndex &index);

signals:
    void countChanged();
};

#endif // QMLSTANDARDITEMMODEL_H
