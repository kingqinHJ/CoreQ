#ifndef HTTPMANAGER_H
#define HTTPMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QWeakPointer>

class HttpManager : public QObject
{
    Q_OBJECT
public:
    explicit HttpManager(QObject *parent = nullptr);

    Q_INVOKABLE void get(QString url);
    Q_INVOKABLE void post(QString text);
    Q_INVOKABLE void formTest();

signals:
    void getReplyReceived(const QString &reply);
    void postReplyReceived(const QString &reply);
    void formReplyReceived(const QString &reply);

public slots:
    //获取异步结果
    void receiveReply(QNetworkReply *reply);

private:
    QSharedPointer<QNetworkAccessManager> mQNetworkAccessManager;
    QNetworkReply *mCurrentReply;
};

#endif // HTTPMANAGER_H
