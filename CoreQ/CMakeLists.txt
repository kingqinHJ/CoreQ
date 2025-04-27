cmake_minimum_required(VERSION 3.16)

project(CoreQ VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# 启用自动资源编译（推荐）
set(CMAKE_AUTORCC ON)

find_package(spdlog CONFIG REQUIRED)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network)

set(PROJECT_SOURCES
        main.cpp
        widget.cpp
        widget.h
        widget.ui
        httpmanager.h
        httpmanager.cpp
        httpwidget.h
        httpwidget.cpp
)
						  
add_subdirectory(DesignPatternsPrinciples)
add_subdirectory(DataProcess)
add_subdirectory(AdvancedFunction)
add_subdirectory(ModernCpp)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(CoreQ
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET CoreQ APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(CoreQ SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(CoreQ
            ${PROJECT_SOURCES}
            qsetting.h qsetting.cpp
            qpromise.cpp qpromise.h
            mysrc.qrc
            vcpkg.json
            callbackdemowidget.h callbackdemowidget.cpp
            PageFactory.h
        )
    endif()
endif()

target_link_libraries(CoreQ PRIVATE 
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
    spdlog::spdlog
)

target_link_libraries(CoreQ PUBLIC DesignPatternsPrinciples DataProcess AdvancedFunction ModernCpp)

target_include_directories(CoreQ PUBLIC
                          "${PROJECT_BINARY_DIR}"
                          "${PROJECT_SOURCE_DIR}/DesignPatternsPrinciples"
                          "${PROJECT_SOURCE_DIR}/DataProcess"
                          "${PROJECT_SOURCE_DIR}/AdvancedFunction"
                          "${PROJECT_SOURCE_DIR}/ModernCpp"
                          )
# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.CoreQ)
endif()
set_target_properties(CoreQ PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS CoreQ
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(CoreQ)
endif()
