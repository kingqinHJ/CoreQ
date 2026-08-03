#ifndef SQLITEHELPER_H
#define SQLITEHELPER_H

#include <QObject>
#include <QSqlDatabase>
#include <QMap>

class SQLiteHelper : public QObject
{
    Q_OBJECT
public:
    explicit SQLiteHelper(QString connectionName, QObject *parent = nullptr);
    ~SQLiteHelper();

public:
    bool initialize(QString connectionName);

    void setReadOnly(bool v);
    void setSyncDisk(bool v);

    bool open(QString name);
    void close();
    void reload();

    bool isValid() const;
    bool isOpen() const;

    QString lastError() const;

    int currentMaxID(QString tableName) const;
    static QString toWhereString(QVariantMap where);
    static QStringList toWhereList(QVariantMap where);

    int count(QString tableName, QString where = QString());
    int count(QString tableName, QVariantMap where);
    bool contains(QString tableName, QVariantMap where);

    bool append(QString tableName, QVariantMap values);
    bool update(QString tableName, QVariantMap values, QVariantMap where);
    bool remove(QString tableName, QVariantMap where);
    bool remove(QString tableName, QString where);
    bool query(QList<QVariantMap> &list, QString tableName, QVariantMap where,
               QString order = QString(), int limit=-1, int offset=-1);
    bool query(QList<QVariantMap> &list, QString tableName, QString where = QString(),
               QString order = QString(), int limit=-1, int offset=-1);
    bool query(QVariant &value, QString tableName, QString field, QVariantMap where);

    bool containsTable(QString tableName);

    bool exec(QString statement, QList<QVariantMap> *list = NULL);

protected:
    QString m_connection_name;
    QSqlDatabase m_db;
    QString m_name;

    bool m_read_only = false;
    bool m_sync_disk = false;
};

#endif // SQLITEHELPER_H
