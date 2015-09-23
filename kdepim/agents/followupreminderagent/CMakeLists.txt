project(followupreminderagent)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

include_directories(
   ${CMAKE_SOURCE_DIR}/messagecomposer/
)

if(ENABLE_TESTING)
    add_subdirectory(tests)
endif()

set(followupreminderlib_SRCS 
    followupreminderinfo.cpp
    followupreminderutil.cpp
)

kde4_add_kcfg_files(followupreminderlib_SRCS
    settings/followupreminderagentsettings.kcfgc
)


add_library( followupreminder ${LIBRARY_TYPE} ${followupreminderlib_SRCS} )
target_link_libraries( followupreminder ${KDE4_KDEUI_LIBS} )

set_target_properties( followupreminder PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )

install( TARGETS followupreminder ${INSTALL_TARGETS_DEFAULT_ARGS} )


set(followupreminderagent_job_SRCS
    jobs/followupreminderjob.cpp
    jobs/followupreminderfinishtaskjob.cpp
    jobs/followupremindershowmessagejob.cpp
)

set(followupreminderagent_SRCS
    followupreminderagent.cpp
    followupremindermanager.cpp
    followupreminderinfodialog.cpp
    followupremindernoanswerdialog.cpp
    followupreminderinfowidget.cpp
    ${followupreminderagent_job_SRCS}
)

qt4_add_dbus_adaptor(followupreminderagent_SRCS org.freedesktop.Akonadi.FollowUpReminder.xml followupreminderagent.h FollowUpReminderAgent)

add_executable(akonadi_followupreminder_agent ${followupreminderagent_SRCS})

target_link_libraries(akonadi_followupreminder_agent
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
  ${KDEPIMLIBS_KMIME_LIBS}
  ${KDEPIMLIBS_KCALCORE_LIBS}
  followupreminder
)

install(TARGETS akonadi_followupreminder_agent ${INSTALL_TARGETS_DEFAULT_ARGS} )

install(FILES followupreminder.desktop DESTINATION "${CMAKE_INSTALL_PREFIX}/share/akonadi/agents")

install(FILES akonadi_followupreminder_agent.notifyrc DESTINATION "${DATA_INSTALL_DIR}/akonadi_followupreminder_agent" )

