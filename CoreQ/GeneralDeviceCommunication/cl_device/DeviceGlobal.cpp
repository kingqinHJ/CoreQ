#include "DeviceGlobal.h"
#include <QPainterPath>

Q_DECLARE_METATYPE(QPainterPath)

static void initializeResource() {
    // cl_device的资源
    Q_INIT_RESOURCE(cl_device);
}

CL_BEGIN_NAMESPACE
DEVICE_BEGIN_NAMESPACE

void initialize() {
    initializeResource();
}

DEVICE_END_NAMESPACE
CL_END_NAMESPACE
