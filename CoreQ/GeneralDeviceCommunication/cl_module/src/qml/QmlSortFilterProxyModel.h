#ifndef SORTFILTERPROXYMODEL_H
#define SORTFILTERPROXYMODEL_H

#include <QtCore/qsortfilterproxymodel.h>
#include <QtQml/qjsvalue.h>
#include <QQmlParserStatus>
#include <QSharedPointer>

class QmlSortFilterProxyModelPrivate;
class QmlSortFilterProxyModel : public QSortFilterProxyModel, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(QObject *source READ source WRITE setSource NOTIFY sourceChanged)

    Q_PROPERTY(QByteArray sortRole READ sortRole WRITE setSortRole NOTIFY sortRoleChanged)
    Q_PROPERTY(Qt::SortOrder sortOrder READ sortOrder WRITE setSortOrder NOTIFY sortOrderChanged)
    Q_PROPERTY(QByteArray sortRole1 READ sortRole1 WRITE setSortRole1 NOTIFY sortRole1Changed)
    Q_PROPERTY(Qt::SortOrder sortOrder1 READ sortOrder1 WRITE setSortOrder1 NOTIFY sortOrder1Changed)
    Q_PROPERTY(QByteArray sortRole2 READ sortRole2 WRITE setSortRole2 NOTIFY sortRole2Changed)
    Q_PROPERTY(Qt::SortOrder sortOrder2 READ sortOrder2 WRITE setSortOrder2 NOTIFY sortOrder2Changed)

    Q_PROPERTY(QByteArray filterRole READ filterRole WRITE setFilterRole NOTIFY filterRoleChanged)
    Q_PROPERTY(QString filterString READ filterString WRITE setFilterString NOTIFY filterStringChanged)
    Q_PROPERTY(FilterSyntax filterSyntax READ filterSyntax WRITE setFilterSyntax NOTIFY filterSyntaxChanged)
    Q_PROPERTY(QVariantMap filterData READ filterData WRITE setFilterData NOTIFY filterDataChanged)
    Q_PROPERTY(bool andLogic READ andLogic WRITE setAndLogic NOTIFY andLogicChanged)
    Q_PROPERTY(QVariantMap ignoreFilterData READ ignoreFilterData WRITE setIgnoreFilterData NOTIFY ignoreFilterDataChanged FINAL)
    Q_PROPERTY(QVariantMap allowFilterData READ allowFilterData WRITE setAllowFilterData NOTIFY allowFilterDataChanged FINAL)

public:
    enum FilterSyntax {
        RegExp,
        Wildcard,
        FixedString
    };
    Q_ENUM(FilterSyntax);

    enum OpType {
        OpLessThan,
        OpLessThanOrEqual,
        OpEqual,
        OpUnequal,
        OpGreaterThanOrEqual,
        OpGreaterThan,
    };
    Q_ENUM(OpType);

    explicit QmlSortFilterProxyModel(QObject *parent = 0);

    int count() const;

    QObject *source() const;
    void setSource(QObject *source);

    QByteArray sortRole() const;
    void setSortRole(const QByteArray &role);

    QByteArray sortRole1() const;
    void setSortRole1(const QByteArray &role);

    QByteArray sortRole2() const;
    void setSortRole2(const QByteArray &role);

    void setSortOrder(Qt::SortOrder order);

    Qt::SortOrder sortOrder1() const;
    void setSortOrder1(Qt::SortOrder order);

    Qt::SortOrder sortOrder2() const;
    void setSortOrder2(Qt::SortOrder order);

    QByteArray filterRole() const;
    void setFilterRole(const QByteArray &role);

    QString filterString() const;
    void setFilterString(const QString &filter);

    FilterSyntax filterSyntax() const;
    void setFilterSyntax(FilterSyntax syntax);

    QVariantMap filterData() const;
    void setFilterData(QVariantMap v);

    bool andLogic() const;
    void setAndLogic(bool v);

    QVariantMap ignoreFilterData() const;
    void setIgnoreFilterData(QVariantMap v);

    QVariantMap allowFilterData() const;
    void setAllowFilterData(QVariantMap v);

    void classBegin() override;
    void componentComplete() override;

public slots:
    QJSValue get(int idx);
    int getSourceIndex(int idx);

    QVariantList getValueListByRoles(QStringList roles);

signals:
    void countChanged();
    void sourceChanged();

    void sortRoleChanged();
    void sortRole1Changed();
    void sortRole2Changed();
    void sortOrderChanged();
    void sortOrder1Changed();
    void sortOrder2Changed();

    void filterRoleChanged();
    void filterStringChanged();
    void filterSyntaxChanged();
    void filterDataChanged();
    void andLogicChanged();
    void ignoreFilterDataChanged();
    void allowFilterDataChanged();

protected:
    int roleKey(const QByteArray &role) const;
    QHash<int, QByteArray> roleNames() const override;
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;

private:
    friend QmlSortFilterProxyModelPrivate;
    QSharedPointer<QmlSortFilterProxyModelPrivate> d;
};

#endif // SORTFILTERPROXYMODEL_H
