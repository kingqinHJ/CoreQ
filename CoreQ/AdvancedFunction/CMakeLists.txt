add_library(AdvancedFunction
    qstatemachinewindow.h qstatemachinewindow.cpp)

target_link_libraries(AdvancedFunction PRIVATE Qt5::Core Qt5::Widgets)
target_include_directories(AdvancedFunction INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
