project(kimagesorter)
 
find_package(KDE4 REQUIRED)
include(KDE4Defaults)
include_directories(${KDE4_INCLUDES})

set(sorter_SRCS 
    main.cpp
    main_window.cpp
    picture_widget.cpp
    image_loader.cpp
    image_sorter.cpp
    settings_dialog.cpp
)
 
add_executable(kimagesorter ${sorter_SRCS})
 
target_link_libraries(kimagesorter
    ${KDE4_KDEUI_LIBS} 
    ${KDE4_KIO_LIBS}
)
 
install(
    TARGETS kimagesorter
    DESTINATION ${BIN_INSTALL_DIR}
)
install(
    FILES kimagesorterui.rc 
    DESTINATION ${DATA_INSTALL_DIR}/kimagesorter
)
install(
    PROGRAMS kimagesorter.desktop
    DESTINATION ${XDG_APPS_INSTALL_DIR}
)