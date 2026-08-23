#ifndef COMMONINFO_H
#define COMMONINFO_H

#include <QString>
#include <QVariantMap>

struct CommonInfo
{
    QString id;
    QString name;
    QString version;
    QString status;
    int progress = 0;

    QVariantMap toMap() const {
        return {
            {"id", id},
            {"name", name},
            {"version", version},
            {"status", status},
            {"progress", progress}
        };
    }

    void reset() {
        *this = CommonInfo{};
    }
};
#endif // COMMONINFO_H