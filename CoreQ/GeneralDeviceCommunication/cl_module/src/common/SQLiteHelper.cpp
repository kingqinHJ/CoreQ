#include "SQLiteHelper.h"

#include <QCoreApplication>
#include <QSqlDriverPlugin>
#include <QPluginLoader>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QVariant>

#define SQL_EXEC(statement, success_expr, error_expr, loop_expr) \
    do { \
        QString tmp = QString(statement); \
        if (tmp.contains("INSERT") \
                || tmp.contains("UPDATE") \
                || tmp.contains("DELETE")) \
            LOGD("exec: %s", qUtf8Printable(statement)); \
        QSqlQuery query(m_db); \
        if (query.exec(statement)) { \
            success_expr \
            while (query.next()) { \
                loop_expr \
            } \
        } \
        else { \
            error_expr \
            LOGC("statement: %s", qUtf8Printable(statement)); \
            LOGC("error: %s", qUtf8Printable(query.lastError().text())); \
        } \
    } while(false)

SQLiteHelper::SQLiteHelper(QString connectionName, QObject *parent)
    : QObject(parent)
{
    if (!initialize(connectionName))
        qt_assert_x("AbstractSqlDatabase", "数据库初始化失败！", __FILE__, __LINE__);
}

SQLiteHelper::~SQLiteHelper()
{
    close();

    m_db = QSqlDatabase();
    QSqlDatabase::removeDatabase(m_connection_name);
}

bool SQLiteHelper::initialize(QString connectionName)
{
    m_connection_name = connectionName;

    if (!m_db.isValid()) {
        m_db = QSqlDatabase::database(m_connection_name);
        if (!m_db.isValid()) {
            m_db = QSqlDatabase::addDatabase("QSQLITE", m_connection_name);
            if (!m_db.isValid()) {
                QPluginLoader loader(QCoreApplication::applicationDirPath() + "/qsqlite.dll");
                QObject *plugin = loader.instance();
                if (plugin) {
                    QSqlDriverPlugin* sqlPlugin = qobject_cast<QSqlDriverPlugin*>(plugin);
                    m_db = QSqlDatabase::addDatabase(sqlPlugin->create("QSQLITE"));
                }
                else {
                    LOGC("SQLITE数据库插件加载失败: \n  %s\n  %s",
                              qUtf8Printable(loader.fileName()), qUtf8Printable(loader.errorString()));
                }
            }
        }
    }

    return m_db.isValid();
}

void SQLiteHelper::setReadOnly(bool v)
{
    m_read_only = v;
}

void SQLiteHelper::setSyncDisk(bool v)
{
    m_sync_disk = v;
}

bool SQLiteHelper::open(QString name)
{
    if (m_db.isOpen())
        m_db.close();

    QStringList options;
    if (m_read_only)
        options.append("QSQLITE_OPEN_READONLY");

    m_db.setConnectOptions(options.join(";"));
    m_db.setDatabaseName(name);

    bool res = m_db.open();
    if (res) {
        m_name = name;
        // 关闭写同步，提高数据库写性能
        SQL_EXEC("PRAGMA synchronous = OFF", {}, {}, {});
    }
    return res;
}

void SQLiteHelper::close()
{
    if (m_db.isOpen())
        m_db.close();
}

void SQLiteHelper::reload()
{
    open(m_name);
}

bool SQLiteHelper::isValid() const
{
    return m_db.isValid();
}

bool SQLiteHelper::isOpen() const
{
    return m_db.isOpen();
}

QString SQLiteHelper::lastError() const
{
    return m_db.lastError().text();
}

int SQLiteHelper::currentMaxID(QString tableName) const
{
    QString statement = QString("SELECT max(id) FROM %1").arg(tableName);

    int maxId = 0;    
    QSqlQuery query(m_db);
    if (query.exec(statement) && query.next()) {
        maxId = query.value("max(id)").toInt();
    }

    return maxId;
}

QString SQLiteHelper::toWhereString(QVariantMap where)
{
    return toWhereList(where).join(" AND ");
}

QStringList SQLiteHelper::toWhereList(QVariantMap where)
{
    QStringList where_list;
    for (auto it=where.begin(); it!=where.end(); it++) {
        if (it.value().userType() == QVariant::Bool)
            where_list.append(QString("%1 = %2").arg(it.key()).arg(it.value().toBool() ? 1 : 0));
        else
            where_list.append(QString(it.value().userType() == QVariant::String
                                      || it.value().userType() == QVariant::ByteArray ? "%1 = '%2'" : "%1 = %2")
                              .arg(it.key(), it.value().toString()));
    }

    return where_list;
}

int SQLiteHelper::count(QString tableName, QString where)
{
    QString statement = QString("SELECT count(*) FROM %1%2")
            .arg(tableName, where.isEmpty() ? "" : (" WHERE "+where));

    int count = -1;
    SQL_EXEC(statement, {}, {}, {
                 count = query.value("count(*)").toInt();
             });

    return count;
}

int SQLiteHelper::count(QString tableName, QVariantMap where)
{
    return count(tableName, toWhereString(where));
}

bool SQLiteHelper::contains(QString tableName, QVariantMap where)
{
    return count(tableName, where) > 0;
}

bool SQLiteHelper::append(QString tableName, QVariantMap values)
{
    QStringList kay_list;
    QStringList value_list;
    for (auto it=values.begin(); it!=values.end(); it++) {
        kay_list.append(it.key());
        if (it.value().userType() == QVariant::Bool)
            value_list.append(QString::number(it.value().toBool() ? 1 : 0));
        else
            value_list.append(QString(it.value().userType() == QVariant::String
                                      || it.value().userType() == QVariant::ByteArray ? "'%1'" : "%1")
                             .arg(it.value().toString()));
    }

    QString statement = QString("INSERT INTO %1(%2) VALUES(%3)")
            .arg(tableName,
                 kay_list.join(','),
                 value_list.join(','));

    bool state = false;
    SQL_EXEC(statement, {
                 state = true;
             }, {}, {});

    if (state && m_sync_disk)
        reload();

    return state;
}

bool SQLiteHelper::update(QString tableName, QVariantMap values, QVariantMap where)
{
    QString statement = QString("UPDATE %1 SET %2 WHERE %3")
            .arg(tableName,
                 toWhereList(values).join(','),
                 toWhereString(where));

    bool state = false;
    SQL_EXEC(statement, {
                 state = true;
             }, {}, {});

    if (state && m_sync_disk)
        reload();

    return state;
}

bool SQLiteHelper::remove(QString tableName, QVariantMap where)
{
    return remove(tableName, toWhereString(where));
}

bool SQLiteHelper::remove(QString tableName, QString where)
{
    QString statement = QString("DELETE FROM %1").arg(tableName);

    if (!where.isEmpty())
        statement.append(" WHERE " + where);

    bool state = false;
    SQL_EXEC(statement, {
                 state = true;
             }, {}, {});

    if (state && m_sync_disk)
        reload();

    return state;
}

bool SQLiteHelper::query(QList<QVariantMap> &list, QString tableName, QVariantMap where, QString order, int limit, int offset)
{
    return query(list, tableName, toWhereString(where), order, limit, offset);
}

bool SQLiteHelper::query(QList<QVariantMap> &list, QString tableName, QString where, QString order, int limit, int offset)
{
    QString statement = QString("SELECT * FROM %1").arg(tableName);

    if (!where.isEmpty())
        statement.append(" WHERE "+where);
    if (!order.isEmpty())
        statement.append(" ORDER BY "+order);
    if (limit > 0)
        statement.append(" LIMIT "+QString::number(limit));
    if (offset > 0)
        statement.append(" OFFSET "+QString::number(offset));

    bool state = false;
    SQL_EXEC(statement, {
                state = true;
             }, {}, {
                 const QSqlRecord record = query.record();
                 QVariantMap row;
                 for (int i=0; i<record.count(); i++)
                     row[record.fieldName(i)] = record.value(i);

                 list.append(row);
             });

    return state;
}

bool SQLiteHelper::query(QVariant &value, QString tableName, QString field, QVariantMap where)
{
    QString statement = QString("SELECT %1 FROM %2").arg(field, tableName);

    if (!where.isEmpty())
        statement.append(" WHERE "+toWhereString(where));

    bool state = false;
    SQL_EXEC(statement, {
                 state = true;
             }, {}, {
                 const QSqlRecord record = query.record();
                 for (int i=0; i<record.count(); i++) {
                     if (record.fieldName(i) == field) {
                         value = record.value(i);
                         break;
                     }
                 }
             });

    return state;
}

bool SQLiteHelper::containsTable(QString tableName)
{
    QString statement = QString("SELECT count(*) FROM sqlite_master WHERE type='table' AND tbl_name='%1'").arg(tableName);

    int count = 0;
    SQL_EXEC(statement, {}, {}, {
                 count = query.value("count(*)").toInt();
             });

    return count > 0;
}

bool SQLiteHelper::exec(QString statement, QList<QVariantMap> *list)
{
    bool state = false;
    SQL_EXEC(statement, {
                 state = true;
             }, {}, {
                 if (list) {
                     const QSqlRecord record = query.record();
                     QVariantMap row;
                     for (int i=0; i<record.count(); i++)
                         row[record.fieldName(i)] = record.value(i);

                     list->append(row);
                 }
             });

    return state;
}
