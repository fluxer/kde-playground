cmake_minimum_required(VERSION 2.8.9)
PROJECT(massif-visualizer)

enable_testing()

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/ ${CMAKE_MODULE_PATH})

set(KDE_MIN_VERSION "4.7.0")
find_package(KDE4 4.7.0 REQUIRED)

include (KDE4Defaults)
include (MacroLibrary)

macro_optional_find_package(KGraphViewer 2.1)
set_package_properties(KGraphViewer PROPERTIES
    DESCRIPTION "KPart to view callgraph dot files."
    URL "http://extragear.kde.org/apps/kgraphviewer/"
    PURPOSE "Required for detailed snapshot analysis. From KDE SDK Extragear package, KGraphViewer 2.1 or higher is required."
)

include_directories(
    ${KDE4_INCLUDES}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/kdchart/include/KDChart
)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

add_subdirectory(kdchart)
add_subdirectory(massifdata)
add_subdirectory(visualizer)
add_subdirectory(app)
add_subdirectory(pics)

if(ENABLE_TESTING)
    add_subdirectory(test)
endif()
