#include "DeviceTask.h"

namespace cl {
namespace device {

class DeviceTaskPrivate
{
public:

};

DeviceTask::DeviceTask(QObject *parent)
    : QObject{parent}
{
    d.reset(new DeviceTaskPrivate);
}

DeviceTask::~DeviceTask()
{

}

} // namespace device
} // namespace cl
