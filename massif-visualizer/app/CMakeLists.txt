include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(massif-visualizer_SRCS
    main.cpp
    mainwindow.cpp
    configdialog.cpp
    documentwidget.cpp
    documenttabinterface.cpp
    charttab.cpp
    allocatorstab.cpp
)

if(KGRAPHVIEWER_FOUND)
    include_directories(
        ${KGRAPHVIEWER_INCLUDE_DIRECTORIES}
    )
    add_definitions(-DHAVE_KGRAPHVIEWER)
    list(APPEND massif-visualizer_SRCS callgraphtab.cpp)
endif(KGRAPHVIEWER_FOUND)

kde4_add_kcfg_files(massif-visualizer_SRCS massif-visualizer-settings.kcfgc)

add_executable(massif-visualizer ${massif-visualizer_SRCS})

target_link_libraries(massif-visualizer LINK_PRIVATE
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${KDE4_KPARTS_LIBS}
    mv-kdchart
    mv-massifdata
    mv-visualizer
)

install( TARGETS massif-visualizer  ${INSTALL_TARGETS_DEFAULT_ARGS} )

install(
    FILES massif-visualizerui.rc documentwidgetui.rc charttabui.rc callgraphtabui.rc
    DESTINATION ${KDE4_DATA_INSTALL_DIR}/massif-visualizer
)

install(FILES massif-visualizer-settings.kcfg DESTINATION ${KDE4_KCFG_INSTALL_DIR})

install(PROGRAMS massif-visualizer.desktop DESTINATION ${KDE4_XDG_APPS_INSTALL_DIR})

# XML mime type
set( SHARED_MIME_INFO_MINIMUM_VERSION "0.30" )
set( XDG_MIME_INSTALL_DIR "share/mime/packages" )
find_package( SharedMimeInfo )
if( SHARED_MIME_INFO_FOUND )
    install( FILES massif.xml DESTINATION ${KDE4_XDG_MIME_INSTALL_DIR} )
    update_xdg_mimetypes( ${KDE4_XDG_MIME_INSTALL_DIR} )
endif( SHARED_MIME_INFO_FOUND )
