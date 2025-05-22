add_library(ModernCpp
    moderncppwidget.h
    moderncppwidget.cpp
)

target_link_libraries(ModernCpp PRIVATE Qt5::Core Qt5::Widgets)
target_include_directories(ModernCpp INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /source-charset:utf-8")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /source-charset:utf-8")
endif()
