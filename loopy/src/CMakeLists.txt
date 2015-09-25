set(loopy_SRCS
  main.cpp
  mainwindow.cpp
  videowidget.cpp
  playlistdock.cpp
  timelabel.cpp
  infolabel.cpp
  options.cpp
  fullscreenbar.cpp
  fullscreenplaylist.cpp
  titlelabel.cpp
  tooltipeater.cpp
  seekingoptions.ui
  themeoptions.ui
  generaloptions.ui
)

kde4_add_kcfg_files(loopy_SRCS settings.kcfgc)
 
add_executable(loopy ${loopy_SRCS})

target_link_libraries(loopy ${KDE4_KDEUI_LIBS}
                            ${KDE4_PHONON_LIBS}
			    ${KDE4_SOLID_LIBS}
			    ${KDE4_KFILE_LIBS})

install(TARGETS loopy DESTINATION ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES loopyui.rc DESTINATION ${DATA_INSTALL_DIR}/loopy)

install(FILES loopy.kcfg DESTINATION ${KCFG_INSTALL_DIR})
