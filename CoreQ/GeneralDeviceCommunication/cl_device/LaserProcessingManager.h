#ifndef LASERPROCESSINGMANAGER_H
#define LASERPROCESSINGMANAGER_H

#include <QObject>

class LaserProcessingManagerPrivate;
class CLDEVICE_EXPORT LaserProcessingManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QObject* deviceMgr READ deviceMgr NOTIFY deviceMgrChanged FINAL)
    Q_PROPERTY(QObject* cameraMgr READ cameraMgr NOTIFY cameraMgrChanged FINAL)
    Q_PROPERTY(QString dataPath READ dataPath WRITE setDataPath NOTIFY dataPathChanged FINAL)

public:
    explicit LaserProcessingManager(QObject *parent = nullptr);
    ~LaserProcessingManager();

    bool initialize();
    void cleanup();

    QObject *deviceMgr() const;
    QObject *cameraMgr() const;

    QString dataPath() const;
    void setDataPath(QString v);

public slots:

private:
    void onDeviceControllerAdded(QString address);
    void onDeviceControllerRemoved(QString address);

signals:
    void deviceMgrChanged();
    void cameraMgrChanged();
    void dataPathChanged();

private:
    std::shared_ptr<LaserProcessingManagerPrivate> d;
};

#endif // LASERPROCESSINGMANAGER_H
