project(KImageViewer)
 
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    include(FeatureSummary)

    find_package(KDELibs4 4.23.0 REQUIRED)

    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

# application
set(kimageviewer_SRCS 
    main.cpp
    kimagewidget.cpp
)

add_executable(kimageviewer ${kimageviewer_SRCS})

target_link_libraries(kimageviewer
    KDE4::kdecore
    KDE4::kparts
    KDE4::kdeui
    KDE4::kio
)

## part
set(kimageviewerpart_SRCS 
    kimagepart.cpp
    kimagewidget.cpp
)

kde4_add_plugin(kimageviewerpart ${kimageviewerpart_SRCS})

target_link_libraries(kimageviewerpart
    KDE4::kdecore
    KDE4::kparts
    KDE4::kdeui
    KDE4::kio
)

## install everything
install(
    TARGETS kimageviewer
    DESTINATION ${KDE4_BIN_INSTALL_DIR}
)
install(
    TARGETS kimageviewerpart
    DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
)
install(
    PROGRAMS kimageviewer.desktop
    DESTINATION ${KDE4_XDG_APPS_INSTALL_DIR}
)
install(
    PROGRAMS kimageviewerpart.desktop
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
)
