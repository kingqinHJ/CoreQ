#ifndef QMLDEVICECONTROLLER_H
#define QMLDEVICECONTROLLER_H

#include <QObject>
#include <QJSValue>

class QmlDeviceControllerPrivate;
class CLDEVICE_EXPORT QmlDeviceController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QObject* deviceMgr READ deviceMgr WRITE setDeviceMgr NOTIFY deviceMgrChanged FINAL)
    Q_PROPERTY(QObject* task READ task NOTIFY taskChanged FINAL)

    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged FINAL)
    Q_PROPERTY(QVariantMap info READ info NOTIFY infoChanged FINAL)
    Q_PROPERTY(QVariantMap config READ config NOTIFY configChanged FINAL)
    Q_PROPERTY(QVariantMap status READ status NOTIFY statusChanged FINAL)

    Q_PROPERTY(int borderSpeed READ borderSpeed WRITE setBorderSpeed NOTIFY configChanged FINAL)
    Q_PROPERTY(int borderPower READ borderPower WRITE setBorderPower NOTIFY configChanged FINAL)

    Q_PROPERTY(QVariantMap connectionInfo READ connectionInfo WRITE setConnectionInfo NOTIFY connectionInfoChanged FINAL)
    Q_PROPERTY(bool canConnect READ canConnect NOTIFY connectionInfoChanged FINAL)
    Q_PROPERTY(bool isEmpty READ isEmpty NOTIFY connectionInfoChanged FINAL)

    Q_PROPERTY(int exposure READ exposure WRITE setExposure NOTIFY exposureChanged FINAL)
    Q_PROPERTY(FireDetectLevel fireDetectLevel READ fireDetectLevel WRITE setFireDetectLevel NOTIFY fireDetectLevelChanged FINAL)

public:
    enum DeviceAxis {
        X_AXIS = 0,
        Y_AXIS,
        Z_AXIS,
        A_AXIS,
        B_AXIS,
        C_AXIS,
        XY_AXIS,        //!<  同时回零XY
        XZ_AXIS,        //!<  同时回零XZ
        YZ_AXIS,        //!<  同时回零YZ
        MASK_AXIS,      //!<  对所支持的轴全部进行回零, 最终效果由GRBL解决
        MAX_AXIS,
    };
    Q_ENUM(DeviceAxis)

    enum FireDetectLevel {
        High,
        Low,
        Off,
    };
    Q_ENUM(FireDetectLevel)

    enum PrintStatus {
        Idle,               // 空闲
        Processing,         // 加工中
        Previewing,         // 预览中
        Paused,             // 加工暂停
        PreviewPaused,      // 预览暂停
        Busy,               // 设备忙：其它未收录的状态
    };
    Q_ENUM(PrintStatus)

    enum TimeUnit {
        TU_Second,          // s
        TU_Minute,          // min(basic)
    };
    Q_ENUM(TimeUnit)

    explicit QmlDeviceController(QObject *parent = nullptr);
    ~QmlDeviceController();

    QObject *deviceMgr() const;
    void setDeviceMgr(QObject *v);

    QObject* task() const;

    bool connected() const;
    void setConnected(bool v);

    QVariantMap info() const;
    QVariantMap config() const;

    QVariantMap status();
    void onStatusChanged();

    int borderSpeed() const;
    void setBorderSpeed(int v);

    int borderPower() const;
    void setBorderPower(int v);

    QVariantMap connectionInfo() const;
    void setConnectionInfo(QVariantMap v);

    bool canConnect() const;
    bool isEmpty() const;

    int exposure() const;
    void setExposure(int v);

    FireDetectLevel fireDetectLevel() const;
    void setFireDetectLevel(FireDetectLevel v);

public slots:
    void open(QString address);
    void tryOpen(QString address, QJSValue callback);
    void connectDevice();
    void close();

    void updateLaser(QString laser);

    void processForPreview(QString filename, QJSValue callback = QJSValue());
    void processForProcessing(QString gcodeFile, QString thumbnailFile, QVariantMap metadata, QJSValue callback = QJSValue());

    void upload(QString filename, QJSValue callback = QJSValue());
    void preview(QJSValue callback = QJSValue());
    void start(QJSValue callback = QJSValue());
    void pause(QJSValue callback = QJSValue());
    void resume(QJSValue callback = QJSValue());
    void stop(QJSValue callback = QJSValue());
    void home(int axis, QJSValue callback = QJSValue());
    void move(int axis, qreal distance, qreal speed = 3000, QJSValue callback = QJSValue());

    void sendCommand(QStringList cmds, QJSValue callback = QJSValue());
    void moveXY(qreal x, qreal y, bool absolute, qreal speed = 3000, QJSValue callback = QJSValue());

    void autoFocus(QJSValue callback = QJSValue());
    void laserOn(int power, QJSValue callback = QJSValue());
    void laserOff(QJSValue callback = QJSValue());

    void getCameraConfig(QJSValue callback = QJSValue());
    void setCameraConfig(QString fileUrl, QJSValue callback = QJSValue());

    QString getFireDetectLevelName(FireDetectLevel v);

signals:
    void deviceMgrChanged();
    void taskChanged();

    void connectedChanged();
    void infoChanged();
    void configChanged();
    void statusChanged();
    void connectionInfoChanged();

    void exposureChanged();
    void fireDetectLevelChanged();

    void deviceConnectFailed();

private:
    std::shared_ptr<QmlDeviceControllerPrivate> d;
};

#endif // QMLDEVICECONTROLLER_H
