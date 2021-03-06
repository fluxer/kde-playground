set(kres-migrator_srcs
    main.cpp
    kabcmigrator.cpp
    kcalmigrator.cpp
    kresmigratorbase.cpp
    ${MIGRATION_AKONADI_SHARED_SOURCES}
)

macro(add_resource_iface _kcfgFile _ifaceName _className)
  kcfg_generate_dbus_interface(${kdepim-runtime_SOURCE_DIR}/resources/${_kcfgFile} ${_ifaceName})
  string(TOLOWER ${_className} _codeFile)
  qt4_add_dbus_interface(kres-migrator_srcs
    ${CMAKE_CURRENT_BINARY_DIR}/${_ifaceName}.xml ${_codeFile} ${_className}
  )
endmacro()

add_resource_iface( vcard/vcardresource.kcfg org.kde.Akonadi.VCard.Settings VCardSettings )
add_resource_iface( ical/icalresource.kcfg org.kde.Akonadi.ICal.Settings ICalSettings )
add_resource_iface( birthdays/birthdaysresource.kcfg org.kde.Akonadi.Birthdays.Settings BirthdaysSettings )
add_resource_iface( maildir/maildirresource.kcfg org.kde.Akonadi.Maildir.Settings MaildirSettings )
add_resource_iface( dav/resource/davgroupwareresource.kcfg org.kde.Akonadi.davGroupware.Settings DavSettings )

add_executable(kres-migrator ${kres-migrator_srcs})

target_link_libraries(kres-migrator
    ${KDEPIMLIBS_AKONADI_LIBS}
    ${KDE4_KRESOURCE_LIBS}
    ${KDEPIMLIBS_KABC_LIBS}
    ${KDEPIMLIBS_KCAL_LIBS}
    ${KDEPIMLIBS_KMIME_LIBS}
    ${KDE4_KDECORE_LIBS}
    ${QT_QTGUI_LIBRARY}
)

install(TARGETS kres-migrator ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES kres-migratorrc DESTINATION ${CONFIG_INSTALL_DIR})
