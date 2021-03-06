project(kolabproxy)

include_directories(
  ${kdepim-runtime_SOURCE_DIR}
  ${QT_QTDBUS_INCLUDE_DIR}
  ${Boost_INCLUDE_DIR}
  ${Libkolab_INCLUDES}
  ${Libkolabxml_INCLUDES}
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS} -fPIC")

########### next target ###############

set(kolabproxyresource_shared_SRCS
  kolabhandler.cpp
  addressbookhandler.cpp
  incidencehandler.cpp
  calendarhandler.cpp
  freebusyupdatehandler.cpp
  taskshandler.cpp
  journalhandler.cpp
  notehandler.cpp
  setupdefaultfoldersjob.cpp
  kolabdefs.cpp
  upgradejob.cpp
  imapitemaddedjob.cpp
  imapitemremovedjob.cpp
  itemaddedjob.cpp
  itemchangedjob.cpp
  handlermanager.cpp
  revertitemchangesjob.cpp
)

set(kolabproxyresource_SRCS
  kolabproxyresource.cpp
  collectiontreebuilder.cpp
  setupkolab.cpp
  ${kolabproxyresource_shared_SRCS}
  ${AKONADI_COLLECTIONATTRIBUTES_SHARED_SOURCES}
)

qt4_add_dbus_adaptor(kolabproxyresource_SRCS org.freedesktop.Akonadi.kolabproxy.xml  kolabproxyresource.h KolabProxyResource)

install(FILES kolabproxyresource.desktop DESTINATION "${CMAKE_INSTALL_PREFIX}/share/akonadi/agents")

kde4_add_kcfg_files(kolabproxyresource_SRCS settings.kcfgc)

kde4_add_ui_files(kolabproxyresource_SRCS kolabsettings.ui changeformat.ui)

kcfg_generate_dbus_interface(
  ${CMAKE_CURRENT_SOURCE_DIR}/kolabproxyresource.kcfg
  org.kde.Akonadi.kolabproxy.Settings
)

qt4_add_dbus_adaptor(kolabproxyresource_SRCS
  ${CMAKE_CURRENT_BINARY_DIR}/org.kde.Akonadi.kolabproxy.Settings.xml
  settings.h Settings
)

if(RUNTIME_PLUGINS_STATIC)
  add_definitions(-DRUNTIME_PLUGINS_STATIC)
endif()

kde4_add_executable(akonadi_kolabproxy_resource ${kolabproxyresource_SRCS})
install(FILES akonadi_kolabproxy_resource.notifyrc DESTINATION "${DATA_INSTALL_DIR}/akonadi_kolabproxy_resource" )

if(Q_WS_MAC)
  set_target_properties(akonadi_kolabproxy_resource PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/../Info.plist.template
  )
  set_target_properties(akonadi_kolabproxy_resource PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER "org.kde.Akonadi.kolabproxy"
  )
  set_target_properties(akonadi_kolabproxy_resource PROPERTIES
    MACOSX_BUNDLE_BUNDLE_NAME "KDE Akonadi Kolabproxy Resource"
  )
endif()

target_link_libraries(akonadi_kolabproxy_resource
  kdepim-copy
  ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_KABC_LIBS}
  ${KDEPIMLIBS_KCALCORE_LIBS}
  ${KDEPIMLIBS_KMIME_LIBS}
  ${KDE4_KIO_LIBS}
  ${QT_QTDBUS_LIBRARY}
  ${QT_QTXML_LIBRARY}
  ${Libkolab_LIBRARIES}
  ${Libkolabxml_LIBRARIES}
)

if(RUNTIME_PLUGINS_STATIC)
  target_link_libraries(akonadi_kolabproxy_resource
    akonadi_serializer_addressee
    akonadi_serializer_contactgroup
    akonadi_serializer_kcalcore
    akonadi_serializer_mail
  )
endif()

install(TARGETS akonadi_kolabproxy_resource ${INSTALL_TARGETS_DEFAULT_ARGS})

kde4_install_icons(${ICON_INSTALL_DIR})

add_subdirectory(tests)
add_subdirectory(wizard)
