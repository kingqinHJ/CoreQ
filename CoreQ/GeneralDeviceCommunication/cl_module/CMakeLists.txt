cmake_minimum_required(VERSION 3.16)

project(cl_module LANGUAGES CXX)
set(TARGET_NAME cl_module)

set(CL_ENABLE_SPDLOG ON CACHE BOOL "Enable spdlog")
set(CL_ENABLE_TOUCHFILE OFF CACHE BOOL "Enable Touching cl_module.cpp")

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui Quick Qml)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Quick Qml)
if (QT_VERSION_MAJOR GREATER 5)
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core5Compat)
endif()

add_library(${TARGET_NAME} STATIC
    src/cl_module.cpp
    src/cl_module.h
    src/common/Utils.cpp
    src/common/Utils.h
    src/common/Snowflake.h
    src/common/WorkerThread.cpp
    src/common/WorkerThread.h
    src/global/HttpManager.cpp
    src/global/HttpManager.h
    src/qml/QmlUtils.cpp
    src/qml/QmlUtils.h
    src/qml/QmlSortFilterProxyModel.cpp src/qml/QmlSortFilterProxyModel.h src/qml/QmlStandardItemModel.cpp src/qml/QmlStandardItemModel.h
    src/qml/FocusDismissor.h src/qml/FocusDismissor.cpp
)

if (CL_ENABLE_TOUCHFILE)
    add_custom_command(TARGET ${TARGET_NAME} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/src/cl_module.cpp" "${CMAKE_BINARY_DIR}/temp_cl_module.cpp"
        COMMAND ${CMAKE_COMMAND} -E remove -f "${CMAKE_CURRENT_SOURCE_DIR}/src/cl_module.cpp"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/temp_cl_module.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/cl_module.cpp"
        COMMENT "Touching cl_module.cpp to force recompilation"
    )
endif()

if (CL_ENABLE_SPDLOG)
    target_compile_definitions(${TARGET_NAME} PRIVATE
        _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING=1
        _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS=1
    )

    target_sources(${TARGET_NAME} PRIVATE
        src/global/Logger.cpp
        src/global/Logger.h
    )
    target_include_directories(${TARGET_NAME} PRIVATE
        libs/spdlog/include
        libs/fmt/include
    )
endif()

target_precompile_headers(${TARGET_NAME} PRIVATE src/cl_module.h)
target_link_libraries(${TARGET_NAME} PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::CorePrivate
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::Qml
)
if (QT_VERSION_MAJOR GREATER 5)
    target_link_libraries(${TARGET_NAME} PUBLIC
        Qt${QT_VERSION_MAJOR}::Core5Compat
    )
endif()
target_include_directories(${TARGET_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
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
