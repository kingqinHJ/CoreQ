add_library(ModernCpp)

target_link_libraries(ModernCpp PRIVATE Qt5::Core Qt5::Widgets)
target_include_directories(ModernCpp INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
