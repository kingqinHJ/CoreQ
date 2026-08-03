#ifndef DEVICETASK_H
#define DEVICETASK_H

#include <QObject>
#include "DeviceDef.h"

namespace cl {
namespace device {

class DeviceTaskPrivate;
class DeviceTask : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString gcodeFile READ gcodeFile WRITE setGcodeFile NOTIFY gcodeFileChanged FINAL)

public:
    explicit DeviceTask(QObject *parent = nullptr);
    ~DeviceTask();

signals:

private:
    std::shared_ptr<DeviceTaskPrivate> d;
};

} // namespace device
} // namespace cl

#endif // DEVICETASK_H
