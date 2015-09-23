project(sendlateragent)

include_directories(
   ${CMAKE_SOURCE_DIR}/pimcommon/
   ${CMAKE_SOURCE_DIR}/messagecomposer/
   ${CMAKE_SOURCE_DIR}/messagecore/
   ${CMAKE_SOURCE_DIR}/libkleo
)


if(ENABLE_TESTING)
    add_subdirectory(tests)
endif()

set(sendlaterlib_SRCS 
    sendlaterinfo.cpp
    sendlaterdialog.cpp
    sendlaterutil.cpp
    sendlatertimedatewidget.cpp
)

kde4_add_kcfg_files(sendlaterlib_SRCS
    settings/sendlateragentsettings.kcfgc 
  )

add_library( sendlater ${LIBRARY_TYPE} ${sendlaterlib_SRCS} )
target_link_libraries( sendlater ${KDE4_KDEUI_LIBS} )

set_target_properties( sendlater PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )

install( TARGETS sendlater ${INSTALL_TARGETS_DEFAULT_ARGS} )


set(sendlateragent_SRCS
   sendlateragent.cpp
   sendlaterconfiguredialog.cpp
   sendlaterconfigurewidget.cpp
   sendlatermanager.cpp
   sendlaterjob.cpp
   sendlaterremovemessagejob.cpp
)


 

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

qt4_add_dbus_adaptor(sendlateragent_SRCS org.freedesktop.Akonadi.SendLaterAgent.xml sendlateragent.h SendLaterAgent)

add_executable(akonadi_sendlater_agent ${sendlateragent_SRCS})

target_link_libraries(akonadi_sendlater_agent
  sendlater
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
  ${KDEPIMLIBS_MAILTRANSPORT_LIBS}
  ${KDEPIMLIBS_KMIME_LIBS}
  messagecomposer
  messagecore
  pimcommon
)

install(TARGETS akonadi_sendlater_agent ${INSTALL_TARGETS_DEFAULT_ARGS} )

install(FILES sendlateragent.desktop DESTINATION "${CMAKE_INSTALL_PREFIX}/share/akonadi/agents")
install(FILES akonadi_sendlater_agent.notifyrc DESTINATION "${DATA_INSTALL_DIR}/akonadi_sendlater_agent" )

