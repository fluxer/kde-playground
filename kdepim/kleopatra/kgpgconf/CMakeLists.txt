project(kgpgconf)

include_directories(${CMAKE_SOURCE_DIR}/libkleo/backends/qgpgme)
add_definitions(-D_ASSUAN_ONLY_GPG_ERRORS -DKDE_DEFAULT_DEBUG_AREA=5154)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

set(_kgpgconf_SRCS
  configreader.cpp
  configwriter.cpp
  configuration.cpp
  exception.cpp
  main.cpp
  mainwindow.cpp
)

add_executable(kgpgconf ${_kgpgconf_SRCS})

target_link_libraries(kgpgconf ${QGPGME_LIBRARIES} ${KDE4_KDEUI_LIBS})

install(TARGETS kgpgconf ${INSTALL_TARGETS_DEFAULT_ARGS})
