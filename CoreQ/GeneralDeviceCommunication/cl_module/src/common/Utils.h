#ifndef UTILS_H
#define UTILS_H

#include <QVariantMap>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#ifdef QT_GUI_LIB
#include <QImage>
#endif

#ifdef QT_NETWORK_LIB
#include <QNetworkInterface>
#endif

class QEventLoop;
class QTemporaryFile;

class Utils
{
public:
    /**
     * JSON
     */
    static QByteArray jsonToString(const QVariantMap &value, bool base64 = false);
    static QByteArray jsonToString(const QVariantList &value, bool base64 = false);
    static QByteArray jsonToString(const QJsonValue &value, bool base64 = false);
    static QJsonValue stringToJson(const QByteArray &data, bool base64 = false);

    static qreal jsonValueToDouble(const QJsonValue &v);
    static QString jsonValueToString(const QJsonValue &v);

    static inline QJsonValue stdStringToJson(const std::string &data, bool base64 = false) {
        return stringToJson(QByteArray::fromStdString(data), base64);
    }
    static inline std::string jsonToStdString(const QJsonValue &value, bool base64 = false) {
        return jsonToString(value, base64).toStdString();
    }

    static QStringList arrayToStringList(const QJsonArray &array);
    static std::list<std::string> arrayToStdStringList(const QJsonArray &array);

    static QJsonValue stringListToArray(const QStringList &list);
    static QJsonValue stringListToArray(const std::list<std::string> &list);

    static QVariantList stringListToVariantList(const QStringList &list);
    static QStringList variantListToStringList(const QVariantList &list);

    template<class Container>
    static QJsonValue listToArray(const Container &list) {
        QJsonArray array;
        for (auto it=list.constBegin(); it!=list.constEnd(); it++) {
            array.append(*it);
        }
        return array;
    }

    template<class Container>
    static QVariantList toVariantList(const QList<Container> &list) {
        QVariantList vlist;
        for (auto &it: list)
            vlist.append(it.toMap());
        return vlist;
    }

    template<class Container>
    static QList<Container> fromVariantList(const QVariantList &vlist) {
        QList<Container> list;
        for (auto &it: vlist)
            list.append(Container::fromMap(it.toMap()));
        return list;
    }

    template<class Container>
    static QByteArray listToString(const QList<Container> &list) {
        QJsonArray array;
        for (auto &it: list)
            array.append(QJsonObject::fromVariantMap(it.toMap()));
        return jsonToString(array, true);
    }

    template<class Container>
    static QList<Container> stringToList(const QByteArray &data) {
        auto vlist = stringToJson(data, true).toArray().toVariantList();
        return fromVariantList<Container>(vlist);
    }

    /**
     * FILE
     */
    static bool writeToFile(const QString &filename, const QByteArray &data);
    static bool writeToFile(const QString &filename, const QVariantList &list);
    static bool writeToFile(const QString &filename, const QStringList &list);
    static QByteArray readFromFile(const QString &filename);
    static QJsonArray readFromFileToJsonArray(const QString &filename);
    static void removeFile(QString path, const QStringList &nameFilters);

    static bool removeFolder(QString path);
    static bool clearFolder(QString path);

    /**
     * MISC
     */
    static QString formatFileSize(qint64 size);
    static QString formatTimestamp(qint64 t, bool show_msec = true);
    static QString generateUuid();
    static QString getFirstLetter(QString str);
    static QString getFileMD5(QString filePath);
    static QString getMD5(QByteArray data);
    static QByteArray execute(QString program, QStringList args = {}, bool *result = NULL);
    static bool isFileUrl(QString url);

    static int startEventLoop(QEventLoop *event_loop, int timeout);
    static QTemporaryFile *createFileCopies(QString filename);

#ifdef QT_GUI_LIB
    static QByteArray imageToBase64(QByteArray data, QString format);
    static QByteArray imageToBase64(QImage img);
    static QImage base64ToImage(QByteArray data);
#endif

    static QString stringToBase64(QString str);
    static QString base64ToString(QString str);

    static void recycleMemory();
    static void snapshootMemory();

    static QString applicationName();
    static QString applicationDirPath();

    static QString getMachineId();

    static void addToStartup(QString name, QString filename);
    static void delFromStartup(QString name);
};

#endif // UTILS_H
