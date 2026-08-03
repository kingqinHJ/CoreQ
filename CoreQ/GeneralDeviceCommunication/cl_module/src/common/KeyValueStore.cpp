#include "KeyValueStore.h"

#include <leveldb/db.h>

class KeyValueStorePrivate
{
public:
    QString source;
    leveldb::DB *db = NULL;
};

KeyValueStore::KeyValueStore(QObject *parent)
    : QObject(parent)
{
    d.reset(new KeyValueStorePrivate);
}

KeyValueStore::~KeyValueStore()
{
    close();
}

bool KeyValueStore::open(QString filename)
{
    if (d->db) return false;

    d->source = filename;
    leveldb::Options options;
    options.create_if_missing = true;
    leveldb::Status status = leveldb::DB::Open(options, filename.toLocal8Bit().data(), &d->db);
    return status.ok();
}

void KeyValueStore::close()
{
    if (d->db) {
        delete d->db;
        d->db = NULL;
        d->source.clear();
    }
}

bool KeyValueStore::isOpen() const
{
    return d->db != NULL;
}

QString KeyValueStore::get(QString key, QString default_value)
{
    if (!isOpen())
        return QString();

    std::string value;
    leveldb::Status s = d->db->Get(leveldb::ReadOptions(), key.toStdString(), &value);
    if (!s.ok()) return default_value;
    else return value.c_str();
}

bool KeyValueStore::put(QString key, QString value)
{
    if (!isOpen())
        return false;

    leveldb::Status s = d->db->Put(leveldb::WriteOptions(), key.toStdString(), value.toStdString());
    return s.ok();
}

bool KeyValueStore::remove(QString key)
{
    if (!isOpen())
        return false;

    leveldb::Status s = d->db->Delete(leveldb::WriteOptions(), key.toStdString());
    return s.ok();
}
