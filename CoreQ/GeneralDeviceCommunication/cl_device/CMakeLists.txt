cmake_minimum_required(VERSION 3.16)

project(cl_device LANGUAGES CXX)
set(TARGET_NAME cl_device)

set(CL_ENABLE_DEVICE_SHARED OFF CACHE BOOL "Enable cl_device shared library")
set(CL_ENABLE_SERIAL_PRINT OFF CACHE BOOL "Enable serial print")
set(CL_ENABLE_HTTP_PRINT OFF CACHE BOOL "Enable http print")

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Network SerialPort Multimedia WebSockets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Network SerialPort Multimedia WebSockets)
if (QT_VERSION_MAJOR GREATER 5)
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS QmlIntegration)
endif()

list(APPEND CL_DEVICE_SOURCES
    device/DeviceDef.h
    device/DeviceCapabilityManager.cpp
    device/DeviceCapabilityManager.h
    device/DeviceConfigManager.cpp
    device/DeviceConfigManager.h
    device/DeviceDiscovery.h
    device/DeviceDiscoveryUdp.cpp
    device/DeviceDiscoveryUdp.h
    device/DeviceDiscoverySerial.cpp
    device/DeviceDiscoverySerial.h
    device/DeviceProber.h
    device/DeviceProberSerial.h
    device/DeviceProberSerial.cpp
    device/DeviceProberHttp.h
    device/DeviceProberHttp.cpp
    device/CommunicationInterface.h
    device/CommunicationSerial.h
    device/CommunicationSerial.cpp
    device/CommunicationHttp.h
    device/CommunicationHttp.cpp
    device/ProtocolHandler.h
    device/ProtocolHandlerSerial.h
    device/ProtocolHandlerSerial.cpp
    device/ProtocolHandlerHttp.h
    device/ProtocolHandlerHttp.cpp
    device/DeviceController.h
    device/DeviceController.cpp
    device/DeviceManager.h
    device/DeviceManager.cpp
    device/DeviceDiscoveryModel.h
    device/DeviceDiscoveryModel.cpp
    device/DeviceCatalogModel.h
    device/DeviceCatalogModel.cpp
    device/CommunicationManager.h
    device/CommunicationManager.cpp
    device/CameraManager.h
    device/CameraManager.cpp
    device/CameraBase.h
    device/CameraUsb.h
    device/CameraUsb.cpp
    device/CameraHttp.h
    device/CameraHttp.cpp
    device/CameraModel.h
    device/CameraModel.cpp
    device/CameraFactory.h
    device/CameraFactory.cpp
    LaserProcessingManager.h
    LaserProcessingManager.cpp
    LaserTask.h
    LaserTask.cpp
    QmlDeviceController.h
    QmlDeviceController.cpp
    QmlCameraController.h
    QmlCameraController.cpp
    DeviceGlobal.cpp
    DeviceGlobal.h
    DeviceDefine.h
    res/cl_device.qrc
)

if(CL_ENABLE_DEVICE_SHARED AND NOT IOS)
    add_library(${TARGET_NAME} SHARED ${CL_DEVICE_SOURCES})
    target_compile_definitions(${TARGET_NAME} PUBLIC BUILD_CLDEVICE_LIB)
    set(LINK_VISIBILITY PRIVATE)
else()
    add_library(${TARGET_NAME} STATIC ${CL_DEVICE_SOURCES})
    target_compile_definitions(${TARGET_NAME} PUBLIC BUILD_CLDEVICE_STATIC)
    set(LINK_VISIBILITY PUBLIC)
endif()

if (CL_ENABLE_SERIAL_PRINT)
    target_compile_definitions(${TARGET_NAME} PUBLIC CL_ENABLE_SERIAL_PRINT)
endif()

if (CL_ENABLE_HTTP_PRINT)
    target_compile_definitions(${TARGET_NAME} PUBLIC CL_ENABLE_HTTP_PRINT)
endif()

target_include_directories(${TARGET_NAME} PUBLIC ${PROJECT_SOURCE_DIR})

target_precompile_headers(${TARGET_NAME} PRIVATE DeviceDefine.h)
target_link_libraries(${TARGET_NAME} ${LINK_VISIBILITY}
    cl_module
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::SerialPort
    Qt${QT_VERSION_MAJOR}::Multimedia
    Qt${QT_VERSION_MAJOR}::WebSockets
)

####################################
# Debug
####################################
if(MSVC)
    target_compile_options(${TARGET_NAME} PRIVATE
        $<$<CONFIG:Release>:/Zi /MP /Gm- /Ob0 /Od /Zc:preprocessor /Fd${CMAKE_BINARY_DIR}/$<TARGET_FILE_BASE_NAME:${TARGET_NAME}>.pdb>
    )
    target_link_options(${TARGET_NAME} PRIVATE
        $<$<CONFIG:Release>:/DEBUG>
    )
elseif(APPLE)
    target_compile_options(${TARGET_NAME} PRIVATE $<$<CONFIG:Release>:-g>)
endif()

