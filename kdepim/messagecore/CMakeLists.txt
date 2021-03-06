project(messagecore)
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )
add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=5300)

if(ENABLE_TESTING)
    add_subdirectory(tests)
endif()

include_directories( ${Boost_INCLUDE_DIR} 
                     ${CMAKE_SOURCE_DIR}/pimcommon
                   )

########### next target ###############

set(messagecore_attachment_LIB_SRCS
   attachment/attachmentcollector.cpp
   attachment/attachmentcompressjob.cpp
   attachment/attachmentfromfolderjob.cpp
   attachment/attachmentfrommimecontentjob.cpp
   attachment/attachmentfromurlbasejob.cpp
   attachment/attachmentfromurljob.cpp
   attachment/attachmentloadjob.cpp
   attachment/attachmentpart.cpp
   attachment/attachmentpropertiesdialog.cpp
)

set(messagecore_misc_LIB_SRCS
   misc/imagecollector.cpp
   misc/mailinglist.cpp
   misc/mdnstateattribute.cpp
)

set(messagecore_widget_LIB_SRCS
   widgets/emailaddressrequester.cpp
   widgets/annotationdialog.cpp
)

set(messagecore_helper_LIB_SRCS
   helpers/messagehelpers.cpp
   helpers/nodehelper.cpp
)

set(messagecore_LIB_SRCS
   ${messagecore_attachment_LIB_SRCS}
   ${messagecore_misc_LIB_SRCS}
   ${messagecore_widget_LIB_SRCS}
   ${messagecore_helper_LIB_SRCS}
   settings/globalsettings.cpp
   utils/stringutil.cpp
)

kde4_add_kcfg_files(messagecore_LIB_SRCS
  settings/globalsettings_base.kcfgc
)

add_library(messagecore ${LIBRARY_TYPE} ${messagecore_LIB_SRCS})

target_link_libraries(messagecore
  ${KDEPIMLIBS_KMIME_LIBS}
  ${KDE4_KDEUI_LIBS}
  ${KDE4_KIO_LIBS}
  ${KDEPIMLIBS_KABC_LIBS}
  ${KDEPIMLIBS_KPIMUTILS_LIBS}
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_KPIMTEXTEDIT_LIBS}
  ${KDEPIMLIBS_AKONADI_CONTACT_LIBS}
  ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
  pimcommon
)

target_link_libraries(messagecore LINK_INTERFACE_LIBRARIES
  ${KDEPIMLIBS_KMIME_LIBS}
  ${KDE4_KDEUI_LIBS}
  ${KDE4_KIO_LIBS}
)

set_target_properties(messagecore PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS messagecore ${INSTALL_TARGETS_DEFAULT_ARGS})
