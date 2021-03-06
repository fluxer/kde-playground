include_directories(
    ${kdepim-runtime_SOURCE_DIR}
    ${QT_QTDBUS_INCLUDE_DIR}
)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

add_subdirectory( wizard )

########### next target ###############

set( contactsresource_SRCS
  contactsresource.cpp
  settingsdialog.cpp
)

kde4_add_kcfg_files(contactsresource_SRCS settings.kcfgc)
kcfg_generate_dbus_interface(${CMAKE_CURRENT_SOURCE_DIR}/contactsresource.kcfg org.kde.Akonadi.Contacts.Settings)
qt4_add_dbus_adaptor(contactsresource_SRCS
  ${CMAKE_CURRENT_BINARY_DIR}/org.kde.Akonadi.Contacts.Settings.xml settings.h Akonadi_Contacts_Resource::ContactsResourceSettings contactsresourcesettingsadaptor ContactsResourceSettingsAdaptor
)

install( FILES contactsresource.desktop DESTINATION "${CMAKE_INSTALL_PREFIX}/share/akonadi/agents" )

kde4_add_plugin(akonadi_contacts_resource ${contactsresource_SRCS})

target_link_libraries(akonadi_contacts_resource
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${QT_QTCORE_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDEPIMLIBS_KABC_LIBS}
  ${KDE4_KIO_LIBS}
)

install(TARGETS akonadi_contacts_resource DESTINATION ${PLUGIN_INSTALL_DIR})
