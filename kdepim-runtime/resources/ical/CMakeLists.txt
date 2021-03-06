include_directories(
    ${kdepim-runtime_SOURCE_DIR}
    ${QT_QTDBUS_INCLUDE_DIR}
    ${Boost_INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/shared
)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

add_subdirectory( wizard )
add_subdirectory( notes )
add_subdirectory( tests )

########### next target ###############
add_definitions( -DSETTINGS_NAMESPACE=Akonadi_ICal_Resource )

set( icalresource_SRCS
  ${AKONADI_SINGLEFILERESOURCE_SHARED_SOURCES}
  shared/icalresourcebase.cpp
  shared/icalresource.cpp
  icalresourceplugin.cpp
)

install( FILES icalresource.desktop DESTINATION "${CMAKE_INSTALL_PREFIX}/share/akonadi/agents" )

kde4_add_kcfg_files(icalresource_SRCS settings.kcfgc)
kcfg_generate_dbus_interface(${CMAKE_CURRENT_SOURCE_DIR}/icalresource.kcfg org.kde.Akonadi.ICal.Settings)
qt4_add_dbus_adaptor(icalresource_SRCS
  ${CMAKE_CURRENT_BINARY_DIR}/org.kde.Akonadi.ICal.Settings.xml settings.h Akonadi_ICal_Resource::Settings icalsettingsadaptor ICalSettingsAdaptor
)

kde4_add_plugin(akonadi_ical_resource ${icalresource_SRCS})

target_link_libraries(akonadi_ical_resource ${KDEPIMLIBS_AKONADI_LIBS} ${QT_QTDBUS_LIBRARY}  ${KDE4_KIO_LIBS} ${KDEPIMLIBS_KCALCORE_LIBS})

install(TARGETS akonadi_ical_resource DESTINATION ${PLUGIN_INSTALL_DIR})
