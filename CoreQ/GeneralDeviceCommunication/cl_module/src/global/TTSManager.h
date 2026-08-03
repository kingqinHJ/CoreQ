#ifndef TTSMANAGER_H
#define TTSMANAGER_H

#include <QObject>
#include <QJSValue>

#define s_tts TTSManager::instance()

class TTSManagerPrivate;
class TTSManager : public QObject
{
    Q_OBJECT
public:
    explicit TTSManager(QObject *parent = nullptr);
    ~TTSManager();

    static TTSManager *instance() { return self; }

    void start();
    void stop();

    bool create(const QString &filename, const QString &content);
    bool preread(const QString &content);

    QString errorString() const;

public slots:
    void create(const QString &filename, const QString &content, QJSValue callback);
    void preread(const QString &content, QJSValue callback);

private:
    static TTSManager *self;
    std::shared_ptr<TTSManagerPrivate> d;
};

#endif // TTSMANAGER_H
