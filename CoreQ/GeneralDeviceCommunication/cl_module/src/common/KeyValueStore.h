#ifndef KEYVALUESTORE_H
#define KEYVALUESTORE_H

#include <QObject>

class KeyValueStorePrivate;
class KeyValueStore : public QObject
{
    Q_OBJECT
public:
    explicit KeyValueStore(QObject *parent = nullptr);
    ~KeyValueStore();

    bool open(QString filename);
    void close();
    bool isOpen() const;

    QString get(QString key, QString default_value = QString());
    bool put(QString key, QString value);
    bool remove(QString key);

signals:

private:
    QSharedPointer<KeyValueStorePrivate> d;
};

#endif // KEYVALUESTORE_H
