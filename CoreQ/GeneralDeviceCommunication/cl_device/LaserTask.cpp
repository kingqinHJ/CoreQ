#include "LaserTask.h"
#include "common/Utils.h"

class LaserTaskPrivate
{
public:
    qint64 estimatedMSeconds = 0;
    qint64 elapsedMSeconds = 0;

    QElapsedTimer elapsed;

    bool running = false;
    bool paused = false;
    bool finished = false;

    int timerId = 0;

    QString gcodeFile;
    QString thumbnailFile;

    QString errorString;
};

LaserTask::LaserTask(QObject *parent)
    : QObject{parent}
{
    d.reset(new LaserTaskPrivate);
}

LaserTask::~LaserTask()
{

}

qint64 LaserTask::estimatedMSeconds() const
{
    return d->estimatedMSeconds;
}

void LaserTask::setEstimatedMSeconds(qint64 v)
{
    if (d->estimatedMSeconds == v)
        return;

    d->estimatedMSeconds = v;
    emit estimatedMSecondsChanged();
}

QString LaserTask::estimatedTime() const
{
    return Utils::formatTimestamp(estimatedMSeconds(), false);
}

qint64 LaserTask::elapsedMSeconds() const
{
    if (d->elapsed.isValid())
        return d->elapsedMSeconds + d->elapsed.elapsed();
    else
        return d->elapsedMSeconds;
}

qint64 LaserTask::remainingMSeconds() const
{
    return std::max(0LL, estimatedMSeconds()-elapsedMSeconds());
}

QString LaserTask::elapsedTime() const
{
    return Utils::formatTimestamp(elapsedMSeconds(), false);
}

QString LaserTask::remainingTime() const
{
    return Utils::formatTimestamp(remainingMSeconds(), false);
}

int LaserTask::progress() const
{
    if (isEmpty() || estimatedMSeconds() == 0)
        return 0;

    if (!d->running && !hasError())
        return d->finished ? 100 : 0;

    qreal v = 0;
    if (elapsedMSeconds() > 0)
        v = 1.0 * elapsedMSeconds() / estimatedMSeconds();
    v = std::clamp<qreal>(v, 0, 0.99);
    return v*100;
}

bool LaserTask::isEmpty() const
{
    return d->gcodeFile.isEmpty();
}

bool LaserTask::isRunning() const
{
    return d->running;
}

bool LaserTask::isPaused() const
{
    return d->paused;
}

bool LaserTask::isFinished() const
{
    return d->finished;
}

QString LaserTask::gcodeFile() const
{
    return d->gcodeFile;
}

void LaserTask::setGcodeFile(QString v)
{
    if (d->gcodeFile == v)
        return;

    d->gcodeFile = v;
    emit gcodeFileChanged();
}

QString LaserTask::thumbnailFile() const
{
    return d->thumbnailFile;
}

void LaserTask::setThumbnailFile(QString v)
{
    if (d->thumbnailFile == v)
        return;

    d->thumbnailFile = v;
    emit thumbnailFileChanged();
}

bool LaserTask::hasError() const
{
    return !d->errorString.isEmpty();
}

QString LaserTask::errorString() const
{
    return d->errorString;
}

void LaserTask::setErrorString(QString v)
{
    if (d->errorString == v)
        return;

    d->errorString = v;
    emit errorStringChanged();
}

void LaserTask::setMetaData(QVariantMap v)
{
    if (v.contains("estimated"))
        setEstimatedMSeconds(v.value("estimated").toDouble());
}

void LaserTask::start()
{
    if (d->running || isEmpty())
        return;

    d->running = true;
    d->paused = false;
    d->finished = false;
    d->timerId = startTimer(200);

    d->elapsedMSeconds = 0;
    d->elapsed.restart();

    emit runningChanged();
    emit pausedChanged();
    emit finishedChanged();

    setErrorString(QString());

    LOG_THIS();
}

void LaserTask::pause()
{
    if (d->paused)
        return;

    d->paused = true;
    emit pausedChanged();

    if (d->elapsed.isValid()) {
        d->elapsedMSeconds += d->elapsed.elapsed();
        d->elapsed.invalidate();
    }

    LOG_THIS();
}

void LaserTask::resume()
{
    if (!d->paused)
        return;

    d->paused = false;
    emit pausedChanged();

    d->elapsed.restart();

    LOG_THIS();
}

void LaserTask::stop()
{
    if (!d->running)
        return;

    d->running = false;
    d->paused = false;
    d->finished = true;
    if (d->timerId != 0) {
        killTimer(d->timerId);
        d->timerId = 0;
    }

    if (d->elapsed.isValid()) {
        d->elapsedMSeconds += d->elapsed.elapsed();
        d->elapsed.invalidate();
        emit elapsedMSecondsChanged();
    }

    emit runningChanged();
    emit pausedChanged();
    emit finishedChanged();

    LOGD("reason:") << d->errorString;
}

void LaserTask::reset()
{
    d->running = false;
    d->paused = false;
    d->finished = false;

    d->estimatedMSeconds = 0;
    d->elapsedMSeconds = 0;
    d->elapsed.invalidate();

    emit runningChanged();
    emit pausedChanged();
    emit finishedChanged();
    emit estimatedMSecondsChanged();
    emit elapsedMSecondsChanged();

    setGcodeFile(QString());
    setThumbnailFile(QString());
    setErrorString(QString());
}

void LaserTask::resetState()
{
    d->running = false;
    d->paused = false;
    d->finished = false;

    d->elapsedMSeconds = 0;
    d->elapsed.invalidate();

    emit runningChanged();
    emit pausedChanged();
    emit finishedChanged();
    emit elapsedMSecondsChanged();
}

void LaserTask::timerEvent(QTimerEvent *e)
{
    emit elapsedMSecondsChanged();
}
