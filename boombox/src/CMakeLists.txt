set(boombox_SRCS
    bbfilechangejob.cpp
    bbresultview.cpp
    bbfilterdock.cpp
    bbsetmodel.cpp
    bbmetadata.cpp
    bbdatabase.cpp
    threadjob.cpp
    dbupdatejob.cpp
    dbqueryjob.cpp
    bbmetainfodialog.cpp
    bbmainwindow.cpp
    bbplaylistsystem.cpp
    bbcollectiontab.cpp
    bbfilesystemtab.cpp
    bbstreamstab.cpp
    bbstreameditform.cpp
    mpris2player.cpp
    main.cpp
    directoriespage.ui
    bbmetainfodialog.ui
    bbstreameditform.ui
)

qt4_add_dbus_adaptor(boombox_SRCS dbus/org.mpris.MediaPlayer2.xml
                     mpris2player.h Mpris2Player)
qt4_add_dbus_adaptor(boombox_SRCS dbus/org.mpris.MediaPlayer2.Player.xml
                     mpris2player.h Mpris2Player)
qt4_add_dbus_interface(boombox_SRCS dbus/org.mpris.MediaPlayer2.Player.xml
                     mpris2playerclient)

kde4_add_kcfg_files(boombox_SRCS bbsettings.kcfgc)

add_executable(boombox ${boombox_SRCS})

target_link_libraries(boombox ${KDE4_KDEUI_LIBS} ${KDE4_PHONON_LIBS}
                      ${KDE4_THREADWEAVER_LIBS} ${KDE4_KFILE_LIBS}
                      ${SQLITE_LIBRARIES} ${TAGLIB_LIBRARIES})

add_definitions(${TAGLIB_CFLAGS} ${SQLITE_DEFINITIONS})


########### install files ###############

install(TARGETS boombox ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES boombox.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES boombox.kcfg  DESTINATION  ${KCFG_INSTALL_DIR})
