project(ktnef)

include(TestBigEndian)
test_big_endian(CMAKE_WORDS_BIGENDIAN)
configure_file(config-ktnef.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-ktnef.h)

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

add_subdirectory(pics)

set(ktnef_SRCS
  attachpropertydialog.cpp
  ktnefmain.cpp
  ktnefview.cpp
  main.cpp
  messagepropertydialog.cpp
  qwmf.cpp
)

add_executable(ktnef ${ktnef_SRCS})
target_link_libraries(ktnef ${KDEPIMLIBS_KTNEF_LIBS} ${KDE4_KIO_LIBS})


install(TARGETS ktnef ${INSTALL_TARGETS_DEFAULT_ARGS})

install(PROGRAMS ktnef.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})

install(FILES ktnefui.rc DESTINATION ${DATA_INSTALL_DIR}/ktnef)

