project(KImageViewer)
 
find_package(KDE4 REQUIRED)
include(KDE4Defaults)
include_directories(${KDE4_INCLUDES})

# application
set(kimageviewer_SRCS 
    main.cpp
    kimagewidget.cpp
)

add_executable(kimageviewer ${kimageviewer_SRCS})

target_link_libraries(kimageviewer
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KPARTS_LIBS}
    ${KDE4_KDEUI_LIBS} 
)

## part
set(kimageviewerpart_SRCS 
    kimagepart.cpp
    kimagewidget.cpp
)

kde4_add_plugin(kimageviewerpart ${kimageviewerpart_SRCS})

target_link_libraries(kimageviewerpart
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KPARTS_LIBS}
    ${KDE4_KDEUI_LIBS}
)

## install everything
install(
    TARGETS kimageviewer
    DESTINATION ${BIN_INSTALL_DIR}
)
install(
    TARGETS kimageviewerpart
    DESTINATION ${PLUGIN_INSTALL_DIR}
)
install(
    PROGRAMS kimageviewer.desktop
    DESTINATION ${XDG_APPS_INSTALL_DIR}
)
install(
    PROGRAMS kimageviewerpart.desktop
    DESTINATION ${SERVICES_INSTALL_DIR}
)
