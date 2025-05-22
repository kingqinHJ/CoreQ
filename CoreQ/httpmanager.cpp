#include "httpmanager.h"
#include <QNetworkReply>
#include <QDebug>
#include <QHttpMultiPart>
#include <QFile>
#include <QEventLoop>
#include<string>

HttpManager::HttpManager(QObject *parent)
    : QObject{parent}
    , mCurrentReply(nullptr)
{
    mQNetworkAccessManager = QSharedPointer<QNetworkAccessManager>(new QNetworkAccessManager(this));
    connect(mQNetworkAccessManager.data(), &QNetworkAccessManager::finished, this, &HttpManager::receiveReply);
}

void HttpManager::get(QString url)
{
    QNetworkRequest request;
    request.setUrl(url);
    mCurrentReply = mQNetworkAccessManager.data()->get(request);
    mCurrentReply->setProperty("requestType", "get");
}

void HttpManager::post(QString text)
{
    QNetworkRequest request;
    request.setUrl(QString("http://httpbin.org/post"));
    mCurrentReply = mQNetworkAccessManager.data()->post(request, text.toUtf8());
    mCurrentReply->setProperty("requestType", "post");
}

void HttpManager::formTest()
{
    QHttpMultiPart *multipart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    QHttpPart part;
    part.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("image/png"));
    part.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"theimage\";filename=\"image.png\";"));
    QFile *imageFile = new QFile(":/image.png");

    if(imageFile->open(QIODevice::ReadOnly))
    {
        part.setBodyDevice(imageFile);
        multipart->append(part);
    }
    /*
     * 在原始代码中，imageFile->setParent(multipart) 在 open 和 setBodyDevice 之前调用。
     * 这可能导致 multipart 在接管 imageFile 时干扰了文件句柄的状态，尤其是在 Qt 的资源文件管理中。
     * 将 setParent 移到 multipart->append(part) 之后，确保文件完全配置后再设置父对象，避免了这个问题。
     */
    imageFile->setParent(multipart); //在删除reply时一并释放
    multipart->setBoundary("qtdata");
    QNetworkRequest request(QUrl("http://httpbin.org/post"));
    request.setRawHeader("Content-Type", "multipart/form-data;boundary=qtdata");

    mCurrentReply = mQNetworkAccessManager.data()->post(request, multipart);
    mCurrentReply->setProperty("requestType", "form");
    multipart->setParent(mCurrentReply);
}

void HttpManager::receiveReply(QNetworkReply *reply)
{
    QString requestType = reply->property("requestType").toString();
    QString response = QString::fromUtf8(reply->readAll());

    // 构建响应信息
    QString fullResponse = QString("Status Code: %1\nURL: %2\n\nResponse:\n%3")
                               .arg(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt())
                               .arg(reply->url().toString())
                               .arg(response);

    // 根据请求类型发送对应的信号
    if (requestType == "get") {
        emit getReplyReceived(fullResponse);
    } else if (requestType == "post") {
        emit postReplyReceived(fullResponse);
    } else if (requestType == "form") {
        emit formReplyReceived(fullResponse);
    }

    reply->deleteLater();
    mCurrentReply = nullptr;
}
