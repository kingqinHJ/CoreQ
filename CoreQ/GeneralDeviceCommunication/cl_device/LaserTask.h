#ifndef LASERTASK_H
#define LASERTASK_H

#include <QObject>

class LaserTaskPrivate;
class CLDEVICE_EXPORT LaserTask : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qint64 estimatedMSeconds READ estimatedMSeconds WRITE setEstimatedMSeconds NOTIFY estimatedMSecondsChanged FINAL)
    Q_PROPERTY(QString estimatedTime READ estimatedTime NOTIFY estimatedMSecondsChanged FINAL)
    Q_PROPERTY(qint64 elapsedMSeconds READ elapsedMSeconds NOTIFY elapsedMSecondsChanged FINAL)
    Q_PROPERTY(qint64 remainingMSeconds READ remainingMSeconds NOTIFY elapsedMSecondsChanged FINAL)
    Q_PROPERTY(QString elapsedTime READ elapsedTime NOTIFY elapsedMSecondsChanged FINAL)
    Q_PROPERTY(QString remainingTime READ remainingTime NOTIFY elapsedMSecondsChanged FINAL)
    Q_PROPERTY(int progress READ progress NOTIFY elapsedMSecondsChanged FINAL)

    Q_PROPERTY(bool empty READ isEmpty NOTIFY gcodeFileChanged FINAL)
    Q_PROPERTY(bool running READ isRunning NOTIFY runningChanged FINAL)
    Q_PROPERTY(bool paused READ isPaused NOTIFY pausedChanged FINAL)
    Q_PROPERTY(bool finished READ isFinished NOTIFY finishedChanged FINAL)
    Q_PROPERTY(QString gcodeFile READ gcodeFile WRITE setGcodeFile NOTIFY gcodeFileChanged FINAL)
    Q_PROPERTY(QString thumbnailFile READ thumbnailFile WRITE setThumbnailFile NOTIFY thumbnailFileChanged FINAL)

    Q_PROPERTY(bool hasError READ hasError NOTIFY errorStringChanged FINAL)
    Q_PROPERTY(QString errorString READ errorString WRITE setErrorString NOTIFY errorStringChanged FINAL)

public:
    explicit LaserTask(QObject *parent = nullptr);
    ~LaserTask();

    qint64 estimatedMSeconds() const;
    void setEstimatedMSeconds(qint64 v);
    QString estimatedTime() const;

    qint64 elapsedMSeconds() const;
    qint64 remainingMSeconds() const;
    QString elapsedTime() const;
    QString remainingTime() const;
    int progress() const;

    bool isEmpty() const;
    bool isRunning() const;
    bool isPaused() const;
    bool isFinished() const;

    QString gcodeFile() const;
    void setGcodeFile(QString v);

    QString thumbnailFile() const;
    void setThumbnailFile(QString v);

    bool hasError() const;
    QString errorString() const;
    void setErrorString(QString v);

    void setMetaData(QVariantMap v);

    void start();
    void pause();
    void resume();
    void stop();

public slots:
    void reset();
    void resetState();

protected:
    void timerEvent(QTimerEvent *e) override;

signals:
    void estimatedMSecondsChanged();
    void elapsedMSecondsChanged();

    void runningChanged();
    void pausedChanged();
    void finishedChanged();
    void gcodeFileChanged();
    void thumbnailFileChanged();

    void errorStringChanged();

private:
    std::shared_ptr<LaserTaskPrivate> d;
};

#endif // LASERTASK_H
