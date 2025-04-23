add_library(DataProcess
    jsonwidget.h jsonwidget.cpp)

target_link_libraries(DataProcess PRIVATE Qt5::Core Qt5::Widgets)
target_include_directories(DataProcess INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
