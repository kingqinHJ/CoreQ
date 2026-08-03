QT += core gui

QMAKE_APPLE_DEVICE_ARCHS = "x86_64"
greaterThan(QT_MAJOR_VERSION, 5): QT += core5compat

lessThan(QT_MAJOR_VERSION, 6) {
    win32:QMAKE_CXXFLAGS += -execution-charset:utf-8
    win32:QMAKE_CXXFLAGS += -source-charset:utf-8
    win32:QMAKE_CXXFLAGS_WARN_ON += -wd4819
}

INCLUDEPATH += \
    $$PWD/libs/spdlog/include \
    $$PWD/libs/fmt/include \
    $$PWD/src

SOURCES += \
    $$PWD/src/cl_module.cpp \
    $$PWD/src/common/Utils.cpp \
    $$PWD/src/common/WorkerThread.cpp \
    $$PWD/src/global/HttpManager.cpp \
    $$PWD/src/global/Logger.cpp \
    $$PWD/src/qml/QmlUtils.cpp

HEADERS += \
    $$PWD/src/cl_module.h \
    $$PWD/src/common/Utils.h \
    $$PWD/src/common/WorkerThread.h \
    $$PWD/src/global/HttpManager.h \
    $$PWD/src/global/Logger.h \
    $$PWD/src/qml/QmlUtils.h
