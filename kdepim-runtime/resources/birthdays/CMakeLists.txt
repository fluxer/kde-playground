include_directories( ${Boost_INCLUDE_DIR} )

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

set( birthdayresource_srcs
  birthdaysresource.cpp
  configdialog.cpp
)

kde4_add_kcfg_files( birthdayresource_srcs settings.kcfgc )
kcfg_generate_dbus_interface(${CMAKE_CURRENT_SOURCE_DIR}/birthdaysresource.kcfg org.kde.Akonadi.Birthdays.Settings)
qt4_add_dbus_adaptor(birthdayresource_srcs
  ${CMAKE_CURRENT_BINARY_DIR}/org.kde.Akonadi.Birthdays.Settings.xml settings.h Settings
)

add_executable(akonadi_birthdays_resource ${birthdayresource_srcs})

target_link_libraries(akonadi_birthdays_resource
  ${KDE4_KIO_LIBS}
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_KABC_LIBS}
  ${KDEPIMLIBS_KCALCORE_LIBS}
  ${KDEPIMLIBS_KPIMUTILS_LIBS}
)

install( TARGETS akonadi_birthdays_resource ${INSTALL_TARGETS_DEFAULT_ARGS} )
install( FILES birthdaysresource.desktop DESTINATION "${CMAKE_INSTALL_PREFIX}/share/akonadi/agents" )
