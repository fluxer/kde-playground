project(kdesktopspy)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    find_package(KDE4 4.19.0 REQUIRED)
    include(KDE4Defaults)
    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

set(kdesktopspy_sources
    kdesktopspy.cpp
)

add_executable(kdesktopspy ${kdesktopspy_sources})
target_link_libraries(kdesktopspy ${KDE4_KDEUI_LIBS} ${KDE4_KFILE_LIBS})

install(TARGETS kdesktopspy DESTINATION ${KDE4_BIN_INSTALL_DIR})
