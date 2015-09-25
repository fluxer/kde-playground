include_directories(
    ${kdepim-runtime_SOURCE_DIR}
    ${QT_QTDBUS_INCLUDE_DIR}
)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

set(CMAKE_AUTOUIC OFF)

########### next target ###############

set( icaldirresource_SRCS
  icaldirresource.cpp
  ../shared/dirsettingsdialog.cpp
)


qt4_wrap_ui(icaldirresource_SRCS settingsdialog.ui)
kde4_add_kcfg_files(icaldirresource_SRCS settings.kcfgc)
kcfg_generate_dbus_interface(${CMAKE_CURRENT_SOURCE_DIR}/icaldirresource.kcfg org.kde.Akonadi.ICalDirectory.Settings)
qt4_add_dbus_adaptor(icaldirresource_SRCS
  ${CMAKE_CURRENT_BINARY_DIR}/org.kde.Akonadi.ICalDirectory.Settings.xml settings.h Settings
)

install( FILES icaldirresource.desktop DESTINATION "${CMAKE_INSTALL_PREFIX}/share/akonadi/agents" )

add_executable(akonadi_icaldir_resource ${icaldirresource_SRCS})

target_link_libraries(akonadi_icaldir_resource
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${QT_QTCORE_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDEPIMLIBS_KCALCORE_LIBS}
  ${KDE4_KIO_LIBS}
)

install(TARGETS akonadi_icaldir_resource ${INSTALL_TARGETS_DEFAULT_ARGS})
