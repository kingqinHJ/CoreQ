add_library(ThreadingDemo
    qpromise.h
    qpromise.cpp
    stdthreadwidget.h
    stdthreadwidget.cpp
    threadingdemowidget.h
    threadingdemowidget.cpp
    mutexdemowidget.h
    mutexdemowidget.cpp
    conditionvariablewidget.h
    conditionvariablewidget.cpp
)

target_link_libraries(ThreadingDemo PRIVATE Qt5::Core Qt5::Widgets)
target_include_directories(ThreadingDemo INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /utf-8")
endif()
