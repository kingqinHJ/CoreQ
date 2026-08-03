#include "QmlSortFilterProxyModel.h"
#include <private/qabstractitemmodel_p.h>
#include "common/Utils.h"
#include <QtDebug>
#include <QtQml>
#include <QRegExp>

class QmlSortFilterProxyModelPrivate
{
public:
    QmlSortFilterProxyModel *q;
    bool is_component_complete = false;

    bool isReady() const {
        return is_component_complete && q->source();
    }

    void updateFilterAndSort();
    bool checkFilterResult(const QModelIndex &sourceIndex, const QVariantMap &filterData, bool andLogic);

    QAbstractItemModel *source = NULL;

    bool sort_dirty = true;
    QByteArray sort_role;
    QByteArray sort_role1;
    QByteArray sort_role2;
    Qt::SortOrder sort_order = Qt::AscendingOrder;
    Qt::SortOrder sort_order1 = Qt::AscendingOrder;
    Qt::SortOrder sort_order2 = Qt::AscendingOrder;

    bool filter_dirty = true;
    QByteArray filter_role;
    QString filter_string;
    QmlSortFilterProxyModel::FilterSyntax filter_syntax = QmlSortFilterProxyModel::Wildcard;
    QVariantMap filter_data;
    bool and_logic = true;
    QVariantMap ignore_filter_data;
    QVariantMap allow_filter_data;
    QRegExp filter_reg;

    QTimer timer;
    void updateTimer() {
        timer.stop();
        timer.start(100);
    }
};

void QmlSortFilterProxyModelPrivate::updateFilterAndSort()
{
    if (!isReady()) return;

    if (q->sourceModel() != NULL)
        emit q->beginResetModel();
    q->invalidate();

    if (!filter_data.isEmpty()) {
        QString tmp_filter_string = Utils::jsonToString(filter_data).toBase64();
        filter_reg = QRegExp(tmp_filter_string, q->filterCaseSensitivity(),
                             static_cast<QRegExp::PatternSyntax>(filter_syntax));
    }
    else if (!filter_string.isEmpty()) {
        filter_reg = QRegExp(filter_string, q->filterCaseSensitivity(),
                             static_cast<QRegExp::PatternSyntax>(filter_syntax));
    }
    else {
        filter_reg = QRegExp();
    }

    q->QSortFilterProxyModel::setFilterRole(q->roleKey(filter_role));
    if (filter_dirty && !filter_reg.isEmpty()) {
        q->invalidateFilter();
        filter_dirty = false;
    }
    // qDebug().noquote().nospace() << QString::asprintf("[%s] - ", __FUNCTION__) << filter_role.data() << " : " << filter_string;

    q->QSortFilterProxyModel::setSortRole(q->roleKey(sort_role));
    if (sort_dirty && q->QSortFilterProxyModel::sortRole() != -1) {
        q->QSortFilterProxyModel::sort(0, sort_order);
        sort_dirty = false;
    }
    // qDebug().noquote().nospace() << QString::asprintf("[%s] - ", __FUNCTION__) << sort_role.data() << " : " << sort_order;

    if (q->sourceModel() != NULL)
        emit q->endResetModel();

    if (q->sourceModel() == NULL)
        q->setSourceModel(source);
    emit q->countChanged();
}

bool QmlSortFilterProxyModelPrivate::checkFilterResult(const QModelIndex &sourceIndex,
                                                       const QVariantMap &filterData, bool andLogic)
{
    if (filterData.isEmpty() || !sourceIndex.isValid())
        return true;

    auto model = q->sourceModel();
    if (!model) return true;

    for (auto it=filterData.begin(); it!=filterData.end(); it++) {
        int role = q->roleKey(it.key().toUtf8());
        if (role == -1)
            continue;

        QVariant value = model->data(sourceIndex, role);
        bool containsOp = filterData.contains(it.key()+"_op");

        if (!containsOp && it.value().type() == QVariant::String) {
            QRegExp rx(it.value().toString(), q->filterCaseSensitivity(),
                       static_cast<QRegExp::PatternSyntax>(filter_syntax));
            QString key = value.toString();
            if (andLogic && !rx.exactMatch(key))
                return false;
            else if (!andLogic && rx.exactMatch(key))
                return true;
        }
        else if (!containsOp && it.value().type() == QVariant::Map) {
            if (filterData.contains(it.key()+"_include")) {
                bool is_include = filterData.value(it.key()+"_include", true).toBool();

                auto map = it.value().toMap();
                QString key = value.toString();
                if (andLogic && ((is_include && !map.contains(key))
                                 || (!is_include && map.contains(key))))
                    return false;
                if (!andLogic && ((is_include && map.contains(key))
                                  || (!is_include && !map.contains(key))))
                    return true;
            }
            else {
                auto cmd_data = it.value().toMap();
                auto cmd = cmd_data.value("cmd").toString();
                if (cmd == "bound") {
                    int min_value = cmd_data.value("min").toInt();
                    int max_value = cmd_data.value("max").toInt();
                    int cur_value = value.toInt();
                    if (andLogic && (cur_value < min_value || max_value < cur_value))
                        return false;
                    else if (!andLogic && (min_value <= cur_value && cur_value <= max_value))
                        return true;
                }
            }
        }
        else {
            int op = filterData.value(it.key()+"_op", QmlSortFilterProxyModel::OpEqual).toInt();

            bool state = false;

#if QT_VERSION_MAJOR < 6
            auto lv = value;
            auto rv = it.value();
#else
            auto lv = value.toDouble();
            auto rv = it.value().toDouble();
#endif
            switch (op) {
            case QmlSortFilterProxyModel::OpLessThan: state = lv < rv; break;
            case QmlSortFilterProxyModel::OpLessThanOrEqual: state = lv <= rv; break;
            case QmlSortFilterProxyModel::OpEqual: state = lv == rv; break;
            case QmlSortFilterProxyModel::OpUnequal: state = lv != rv; break;
            case QmlSortFilterProxyModel::OpGreaterThanOrEqual: state = lv >= rv; break;
            case QmlSortFilterProxyModel::OpGreaterThan: state = lv > rv; break;
            }

            if (andLogic && !state)
                return false;
            else if (!andLogic && state)
                return true;
        }
    }

    return andLogic;
}

QmlSortFilterProxyModel::QmlSortFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    d.reset(new QmlSortFilterProxyModelPrivate);
    d->q = this;

    connect(this, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SIGNAL(countChanged()));
    connect(this, SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SIGNAL(countChanged()));
    connect(this, SIGNAL(modelReset()), this, SIGNAL(countChanged()));

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 2)
    connect(this, &QmlSortFilterProxyModel::sortCaseSensitivityChanged, this, [this](){
        d->updateTimer();
    });
    connect(this, &QmlSortFilterProxyModel::filterCaseSensitivityChanged, this, [this](){
        d->updateTimer();
    });
#endif

    setDynamicSortFilter(true);

    d->timer.setSingleShot(true);
    connect(&d->timer, &QTimer::timeout, this, [this](){
        d->updateFilterAndSort();
    });
}

int QmlSortFilterProxyModel::count() const
{
    return rowCount();
}

QObject *QmlSortFilterProxyModel::source() const
{
    return d->source;
}

void QmlSortFilterProxyModel::setSource(QObject *source)
{
    if (d->source == source)
        return;

    d->source = qobject_cast<QAbstractItemModel *>(source);
    emit sourceChanged();

    if (d->source) {
        connect(d->source, &QAbstractItemModel::modelReset, this, [this](){
            d->updateTimer();
        });
    }

    d->sort_dirty = true;
    d->filter_dirty = true;
    d->updateTimer();
}

QByteArray QmlSortFilterProxyModel::sortRole() const
{
    return d->sort_role;
}

void QmlSortFilterProxyModel::setSortRole(const QByteArray &role)
{
    if (d->sort_role == role)
        return;

    d->sort_role = role;
    emit sortRoleChanged();

    d->sort_dirty = true;
    d->updateTimer();
}

QByteArray QmlSortFilterProxyModel::sortRole1() const
{
    return d->sort_role1;
}

void QmlSortFilterProxyModel::setSortRole1(const QByteArray &role)
{
    if (d->sort_role1 == role)
        return;

    d->sort_role1 = role;
    emit sortRole1Changed();

    d->sort_dirty = true;
    d->updateTimer();
}

QByteArray QmlSortFilterProxyModel::sortRole2() const
{
    return d->sort_role2;
}

void QmlSortFilterProxyModel::setSortRole2(const QByteArray &role)
{
    if (d->sort_role2 == role)
        return;

    d->sort_role2 = role;
    emit sortRole2Changed();

    d->sort_dirty = true;
    d->updateTimer();
}

void QmlSortFilterProxyModel::setSortOrder(Qt::SortOrder order)
{
    if (d->sort_order == order)
        return;

    d->sort_order = order;
    emit sortOrderChanged();

    d->sort_dirty = true;
    d->updateTimer();
}

Qt::SortOrder QmlSortFilterProxyModel::sortOrder1() const
{
    return d->sort_order1;
}

void QmlSortFilterProxyModel::setSortOrder1(Qt::SortOrder order)
{
    if (d->sort_order1 == order)
        return;

    d->sort_order1 = order;
    emit sortOrder1Changed();

    d->sort_dirty = true;
    d->updateTimer();
}

Qt::SortOrder QmlSortFilterProxyModel::sortOrder2() const
{
    return d->sort_order2;
}

void QmlSortFilterProxyModel::setSortOrder2(Qt::SortOrder order)
{
    if (d->sort_order2 == order)
        return;

    d->sort_order2 = order;
    emit sortOrder2Changed();

    d->sort_dirty = true;
    d->updateTimer();
}

QByteArray QmlSortFilterProxyModel::filterRole() const
{
    return d->filter_role;
}

void QmlSortFilterProxyModel::setFilterRole(const QByteArray &role)
{
    if (d->filter_role == role)
        return;

    d->filter_role = role;
    emit filterRoleChanged();

    d->sort_dirty = true;
    d->updateTimer();
}

QString QmlSortFilterProxyModel::filterString() const
{
    return d->filter_string;
}

void QmlSortFilterProxyModel::setFilterString(const QString &filter)
{
    if (d->filter_string == filter)
        return;

    d->filter_string = filter;
    emit filterStringChanged();

    d->filter_dirty = true;
    d->updateTimer();
}

QmlSortFilterProxyModel::FilterSyntax QmlSortFilterProxyModel::filterSyntax() const
{
    return d->filter_syntax;
}

void QmlSortFilterProxyModel::setFilterSyntax(FilterSyntax syntax)
{
    if (d->filter_syntax == syntax)
        return;

    d->filter_syntax = syntax;
    emit filterSyntaxChanged();

    d->filter_dirty = true;
    d->updateTimer();
}

QVariantMap QmlSortFilterProxyModel::filterData() const
{
    return d->filter_data;
}

void QmlSortFilterProxyModel::setFilterData(QVariantMap v)
{
    if (d->filter_data == v)
        return;

    d->filter_data = v;
    emit filterDataChanged();

    d->filter_dirty = true;
    d->updateTimer();
}

bool QmlSortFilterProxyModel::andLogic() const
{
    return d->and_logic;
}

void QmlSortFilterProxyModel::setAndLogic(bool v)
{
    if (d->and_logic == v)
        return;

    d->and_logic = v;
    emit andLogicChanged();

    d->filter_dirty = true;
    d->updateTimer();
}

QVariantMap QmlSortFilterProxyModel::ignoreFilterData() const
{
    return d->ignore_filter_data;
}

void QmlSortFilterProxyModel::setIgnoreFilterData(QVariantMap v)
{
    if (d->ignore_filter_data == v)
        return;

    d->ignore_filter_data = v;
    emit ignoreFilterDataChanged();

    d->filter_dirty = true;
    d->updateTimer();
}

QVariantMap QmlSortFilterProxyModel::allowFilterData() const
{
    return d->allow_filter_data;
}

void QmlSortFilterProxyModel::setAllowFilterData(QVariantMap v)
{
    if (d->allow_filter_data == v)
        return;

    d->allow_filter_data = v;
    emit allowFilterDataChanged();

    d->filter_dirty = true;
    d->updateTimer();
}

void QmlSortFilterProxyModel::classBegin()
{
}

void QmlSortFilterProxyModel::componentComplete()
{
    d->is_component_complete = true;

    d->updateTimer();
}

QJSValue QmlSortFilterProxyModel::get(int idx)
{
    QJSEngine *engine = qmlEngine(this);
    QJSValue value = engine->newObject();
    if (idx >= 0 && idx < count()) {
        QHash<int, QByteArray> roles = roleNames();
        QHashIterator<int, QByteArray> it(roles);
        while (it.hasNext()) {
            it.next();
            value.setProperty(QString::fromUtf8(it.value()), engine->toScriptValue<QVariant>(data(index(idx, 0), it.key())));
        }
    }
    return value;
}

int QmlSortFilterProxyModel::getSourceIndex(int idx)
{
    QModelIndex proxy_index = index(idx, 0);
    QModelIndex source_index = mapToSource(proxy_index);
    return source_index.row();
}

QVariantList QmlSortFilterProxyModel::getValueListByRoles(QStringList roles)
{
    auto oldRoleNames = roleNames();
    QHash<int, QByteArray> newRoleNames;
    for (auto it=oldRoleNames.begin(); it!=oldRoleNames.end(); it++) {
        if (roles.contains(it.value())) {
            newRoleNames.insert(it.key(), it.value());
        }
    }

    QVariantList list;
    for (int i=0; i<rowCount(); i++) {
        auto modelIndex = index(i, 0);

        QVariantMap data;
        for (auto it=newRoleNames.begin(); it!=newRoleNames.end(); it++)
            data.insert(it.value(), modelIndex.data(it.key()));
        list.append(data);
    }
    return list;
}

int QmlSortFilterProxyModel::roleKey(const QByteArray &role) const
{
    if (!d->source || role.isEmpty())
        return -1;

    QHash<int, QByteArray> roles = d->source->roleNames();
    QHashIterator<int, QByteArray> it(roles);
    while (it.hasNext()) {
        it.next();
        if (it.value() == role)
            return it.key();
    }
    return -1;
}

QHash<int, QByteArray> QmlSortFilterProxyModel::roleNames() const
{
    if (d->source)
        return d->source->roleNames();
    else
        return QHash<int, QByteArray>();
}

bool QmlSortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QAbstractItemModel *model = sourceModel();
    QModelIndex sourceIndex = model->index(sourceRow, 0, sourceParent);
    if (!sourceIndex.isValid())
        return true;

    // 跳过忽略过滤的项
    if (!d->ignore_filter_data.isEmpty()
        && d->checkFilterResult(sourceIndex, d->ignore_filter_data, true))
        return true;

    // 跳过不满足过滤的项
    if (!d->allow_filter_data.isEmpty()
        && !d->checkFilterResult(sourceIndex, d->allow_filter_data, true)) {

        // 不参与过滤的通常为组节点，这里的策略是组内容是空的也默认不可见
        int rowCount = model->rowCount(sourceIndex);
        for (int i=0; i<rowCount; i++) {
            if (filterAcceptsRow(i, sourceIndex))
                return true;
        }

        return false;
    }

    if (d->filter_data.isEmpty()) {
        QRegExp rx = d->filter_reg;
        if (rx.isEmpty())
            return true;

        QAbstractItemModel *model = sourceModel();
        if (filterRole().isEmpty()) {
            QHash<int, QByteArray> roles = roleNames();
            QHashIterator<int, QByteArray> it(roles);
            while (it.hasNext()) {
                it.next();
                QString key = model->data(sourceIndex, it.key()).toString();
                if (rx.exactMatch(key))
                    return true;
            }
            return false;
        }

        auto role = roleKey(filterRole());
        if (role == -1)
            return true;

        QString key = model->data(sourceIndex, role).toString();
        return rx.exactMatch(key);
    }
    else {
        return d->checkFilterResult(sourceIndex, d->filter_data, d->and_logic);
    }
}

static QList<QVariant> split(const QString& str)
{
    QList<QVariant> parts;
    QRegularExpressionMatchIterator it = QRegularExpression("(\\d+)|(\\D+)").globalMatch(str);
    while (it.hasNext()) {
        auto match = it.next();
        if (match.captured(1).isEmpty()) {
            parts.append(match.captured(2));
        } else {
            parts.append(match.captured(1).toInt());
        }
    }
    return parts;
}

static bool naturalLess(const QString& a, const QString& b)
{
    static QRegularExpression re("(\\d+)|(\\D+)");
    auto aparts = split(a);
    auto bparts = split(b);
    int count = qMin(aparts.size(), bparts.size());
    for (int i = 0; i < count; ++i) {
        if (aparts[i] != bparts[i]) {
            if (aparts[i].type() == QVariant::Int && bparts[i].type() == QVariant::Int) {
                return aparts[i].toInt() < bparts[i].toInt();
            }
            else {
                return QString::localeAwareCompare(aparts[i].toString(), bparts[i].toString()) < 0;
            }
        }
    }
    return aparts.size() < bparts.size();
}

static bool isLessThan(const QVariant& a, const QVariant& b, Qt::CaseSensitivity cs)
{
    if (a.type() == QVariant::String || b.type() == QVariant::String)
        return naturalLess(a.toString(), b.toString());
    else
        return QAbstractItemModelPrivate::isVariantLessThan(a, b, cs, true);
}

bool QmlSortFilterProxyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    if (QSortFilterProxyModel::sortRole() == -1)
        return true;

    auto sortRole = QSortFilterProxyModel::sortRole();
    QVariant l = (source_left.model() ? source_left.model()->data(source_left, sortRole) : QVariant());
    QVariant r = (source_right.model() ? source_right.model()->data(source_right, sortRole) : QVariant());
    if (l == r && !d->sort_role1.isEmpty()) {
        sortRole = roleKey(d->sort_role1);
        if (sortRole == -1)
            return true;

        l = (source_left.model() ? source_left.model()->data(source_left, sortRole) : QVariant());
        r = (source_right.model() ? source_right.model()->data(source_right, sortRole) : QVariant());

        if (l == r && !d->sort_role2.isEmpty()) {
            sortRole = roleKey(d->sort_role2);
            if (sortRole == -1)
                return true;

            l = (source_left.model() ? source_left.model()->data(source_left, sortRole) : QVariant());
            r = (source_right.model() ? source_right.model()->data(source_right, sortRole) : QVariant());

            bool res = isLessThan(l, r, sortCaseSensitivity());
            if (d->sort_order != d->sort_order1)
                res = !res;
            return res;
        }
        else {
            bool res = isLessThan(l, r, sortCaseSensitivity());
            if (d->sort_order != d->sort_order1)
                res = !res;
            return res;
        }
    }
    else {
        return isLessThan(l, r, sortCaseSensitivity());
    }
}
