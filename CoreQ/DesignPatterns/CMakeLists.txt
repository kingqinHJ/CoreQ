add_library(DesignPatterns
    designpatternswidget.h
    designpatternswidget.cpp
    TemplateMethod/CommonInfo.h
    TemplateMethod/HandlerCallbacks.h
    TemplateMethod/AbstractDeviceHandler.h
    TemplateMethod/AbstractDeviceHandler.cpp
    TemplateMethod/DeviceAHandler.h
    TemplateMethod/DeviceAHandler.cpp
    TemplateMethod/DeviceBHandler.h
    TemplateMethod/DeviceBHandler.cpp
    TemplateMethod/templatemethodwidget.h
    TemplateMethod/templatemethodwidget.cpp
)

target_link_libraries(DesignPatterns PRIVATE Qt5::Core Qt5::Widgets)
target_include_directories(DesignPatterns INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /utf-8")
endif()
