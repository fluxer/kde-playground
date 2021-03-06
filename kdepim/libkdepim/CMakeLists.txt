set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=5300)

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )
if(BALOO_FOUND)
    add_definitions(-DHAVE_BALOO)
endif()

if(ENABLE_TESTING)
    add_subdirectory(tests)
endif()
add_subdirectory(pics)

add_definitions(${QT_QTDBUS_DEFINITIONS})

include_directories( ${Boost_INCLUDE_DIR} )
include_directories( ${BALOO_INCLUDE_DIR} )

option(KDEPIM_INPROCESS_LDAP "Use in-process rather than KIO slave LDAP" FALSE)
if (KDEPIM_INPROCESS_LDAP)
  add_definitions( -DKDEPIM_INPROCESS_LDAP )
endif ()

########### next target ###############

set(kdepim_progresswidget_LIB_SRCS
   progresswidget/progressmanager.cpp
   progresswidget/progressmanager_akonadiagent.cpp
   progresswidget/agentprogressmonitor.cpp
   progresswidget/progressdialog.cpp
   progresswidget/statusbarprogresswidget.cpp
   progresswidget/ssllabel.cpp
   progresswidget/progressstatusbarwidget.cpp
)

set(kdepim_job_LIB_SRCS
   job/addemailaddressjob.cpp
   job/addcontactjob.cpp
   job/openemailaddressjob.cpp
   job/addemaildisplayjob.cpp
)

set(kdepim_widgets_LIB_SRCS
   widgets/customlogwidget.cpp
   widgets/spellchecklineedit.cpp
   widgets/selectedcollectiondialog.cpp
   widgets/kdatepickerpopup.cpp
   widgets/kcheckcombobox.cpp
   widgets/kweekdaycheckcombo.cpp
   widgets/kwidgetlister.cpp
   widgets/pimmessagebox.cpp
   widgets/overlaywidget.cpp
   widgets/tagwidgets.cpp
)

set(kdepim_ldap_LIB_SRCS
   ldap/ldapclient.cpp
   ldap/ldapsearchdialog.cpp
   ldap/ldapclientsearch.cpp
   ldap/ldapclientsearchconfig.cpp
)

set(kdepim_misc_LIB_SRCS
   misc/broadcaststatus.cpp
   misc/maillistdrag.cpp
   misc/uistatesaver.cpp
   misc/statisticsproxymodel.cpp
)

set(kdepim_addressline_LIB_SRCS
   addressline/completionordereditor.cpp
   addressline/addresseelineedit.cpp
   addressline/recentaddresses.cpp
   addressline/kmailcompletion.cpp
)

set(kdepim_LIB_SRCS
   ${kdepim_progresswidget_LIB_SRCS}
   ${kdepim_job_LIB_SRCS}
   ${kdepim_widgets_LIB_SRCS}
   ${kdepim_ldap_LIB_SRCS}
   ${kdepim_misc_LIB_SRCS}
   ${kdepim_addressline_LIB_SRCS}
   prefs/kprefsdialog.cpp
   multiplyingline/multiplyingline.cpp
   multiplyingline/multiplyinglineeditor.cpp
   multiplyingline/multiplyinglineview_p.cpp
)

if (KDEPIM_INPROCESS_LDAP)
  set(kdepim_LIB_SRCS ${kdepim_LIB_SRCS}
    ldap/ldapsession.cpp
    ldap/ldapqueryjob.cpp
  )
endif ()

add_library(kdepim ${LIBRARY_TYPE} ${kdepim_LIB_SRCS})

target_link_libraries(kdepim
  ${KDEPIMLIBS_KRESOURCES_LIBS}
  ${KDEPIMLIBS_KABC_LIBS}
  ${KDEPIMLIBS_KPIMUTILS_LIBS}
  ${KDEPIMLIBS_KLDAP_LIBS}
  ${KDEPIMLIBS_KPIMIDENTITIES_LIBS}
  ${KDE4_KCMUTILS_LIBS}
  ${KDE4_SOLID_LIBS}
  ${KDEPIMLIBS_KMIME_LIBS}
  ${KDEPIMLIBS_KPIMTEXTEDIT_LIBS}
  ${KDEPIMLIBS_KMIME_LIBS}
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_AKONADI_CONTACT_LIBS}
  ${BALOO_LIBRARIES}
)


target_link_libraries(kdepim LINK_INTERFACE_LIBRARIES
  ${KDEPIMLIBS_KRESOURCES_LIBS}
  ${KDEPIMLIBS_KABC_LIBS}
  ${KDEPIMLIBS_KPIMUTILS_LIBS}
  ${KDEPIMLIBS_KLDAP_LIBS}
  ${KDEPIMLIBS_KPIMIDENTITIES_LIBS}
  ${KDEPIMLIBS_KPIMTEXTEDIT_LIBS}
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_AKONADI_CONTACT_LIBS}
)

set_target_properties(kdepim PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS kdepim ${INSTALL_TARGETS_DEFAULT_ARGS})


########### next target ###############

if (QT_QTDESIGNER_FOUND)
  kde4_add_widget_files(${CMAKE_CURRENT_SOURCE_DIR}/designer/kdepim.widgets)

  kde4_add_plugin(kdepimwidgets $<TARGET_OBJECTS:kdepim_autowidgets>)


  target_link_libraries(kdepimwidgets  ${KDE4_KIO_LIBS} kdepim)

  install(TARGETS kdepimwidgets  DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/designer)
endif ()

########### next target ###############

set( kcm_ldap_SRCS
     ldap/addhostdialog.cpp
     ldap/kcmldap.cpp
)

kde4_add_plugin(kcm_ldap ${kcm_ldap_SRCS})

target_link_libraries(kcm_ldap ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} kdepim)

install(TARGETS kcm_ldap DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES ldap/kcmldap.desktop DESTINATION ${SERVICES_INSTALL_DIR})

########### install files ###############

install(FILES interfaces/org.kde.addressbook.service.xml interfaces/org.kde.mailtransport.service.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})
