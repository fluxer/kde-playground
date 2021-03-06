
add_subdirectory( pics )
add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

INCLUDE(CheckIncludeFiles)
check_include_files(sys/poll.h HAVE_SYS_POLL_H)
configure_file(config-libkpgp.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-libkpgp.h)

########### next target ###############

set(kpgp_LIB_SRCS
   kpgpblock.cpp
   kpgpui.cpp
   kpgpkey.cpp
   kpgp.cpp
   kpgpbase.cpp
   kpgpbaseG.cpp
   kpgpbase2.cpp
   kpgpbase5.cpp
   kpgpbase6.cpp )


add_library(kpgp ${LIBRARY_TYPE} ${kpgp_LIB_SRCS})

target_link_libraries(kpgp ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${KDE4_KDECORE_LIBRARY} ${KDE4_KDEUI_LIBRARY})

set_target_properties(kpgp PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS kpgp ${INSTALL_TARGETS_DEFAULT_ARGS})

if(ENABLE_TESTING)
    add_subdirectory(tests)
endif()

########### install files ###############

install(FILES kconf_update/kpgp.upd  DESTINATION ${KCONF_UPDATE_INSTALL_DIR})
install(PROGRAMS kconf_update/kpgp-3.1-upgrade-address-data.pl DESTINATION ${KCONF_UPDATE_INSTALL_DIR})

