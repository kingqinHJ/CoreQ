add_library(MVxDemo
    mvcwidget.h
    mvcwidget.cpp
    mvpwidget.h
    mvpwidget.cpp
    mvvmwidget.h
    mvvmwidget.cpp
    mvxdemowidget.h
    mvxdemowidget.cpp
)

target_link_libraries(MVxDemo PRIVATE Qt5::Core Qt5::Widgets)
target_include_directories(MVxDemo INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /utf-8")
endif()
