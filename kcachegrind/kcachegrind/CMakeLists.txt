include_directories( ../libcore ../libviews )

########### next target ###############

set(kcachegrind_SRCS
    main.cpp
    kdeconfig.cpp
    toplevel.cpp
    configdlg.cpp
    configdlgbase.ui
)

add_executable(kcachegrind ${kcachegrind_SRCS})

target_link_libraries(kcachegrind core views ${KDE4_KIO_LIBS})

install(TARGETS kcachegrind  ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### install files ###############

install( PROGRAMS kcachegrind.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )
install( FILES tips  DESTINATION  ${DATA_INSTALL_DIR}/kcachegrind )
install( FILES kcachegrindui.rc  DESTINATION  ${DATA_INSTALL_DIR}/kcachegrind )

kde4_install_icons( ${ICON_INSTALL_DIR}   )


