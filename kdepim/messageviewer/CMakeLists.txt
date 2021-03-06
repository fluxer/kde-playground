set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

include_directories(${Grantlee_INCLUDE_DIRS} ${QJSON_INCLUDE_DIR})

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )


# KCFG files:
# The main messageviewer.kcfg is configured by CMake and put in the build directory.

if(KDEPIM_ENTERPRISE_BUILD)
  set(LEGACY_MANGLE_FROM_TO_HEADERS true)
  set(LEGACY_BODY_INVITES true)
  set(EXCHANGE_COMPATIBLE_INVITATIONS true)
  set(OUTLOOK_COMPATIBLE_INVITATION_REPLY_COMMENTS true)
else()
  set(LEGACY_MANGLE_FROM_TO_HEADERS false)
  set(LEGACY_BODY_INVITES false)
  set(EXCHANGE_COMPATIBLE_INVITATIONS false)
  set(OUTLOOK_COMPATIBLE_INVITATION_REPLY_COMMENTS false)
endif()

configure_file(settings/messageviewer.kcfg.cmake ${CMAKE_CURRENT_BINARY_DIR}/messageviewer.kcfg)

check_include_files(sys/inotify.h SYS_INOTIFY_H_FOUND)
macro_bool_to_01(SYS_INOTIFY_H_FOUND HAVE_SYS_INOTIFY_H)
configure_file(config-messageviewer.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-messageviewer.h)

add_subdirectory(about)
if(ENABLE_TESTING)
    add_subdirectory(tests)
    add_subdirectory(grantleefilters)
    add_subdirectory(scamdetection/tests)
    add_subdirectory(adblock/tests/)
    add_subdirectory(findbar/tests)
endif()
add_subdirectory(pics)

if(DEBUG_SIGNATURE)
   add_definitions(-DDEBUG_SIGNATURE)
endif()

if(KDEPIM_NO_WEBKIT)
  set(_mailwebview_cpp viewer/mailwebview_textbrowser.cpp )
  set(_mailwebview_lib )
else()
  set(_mailwebview_cpp viewer/mailwebview_webkit.cpp 
                       adblock/networkaccessmanager.cpp 
                       adblock/webpage.cpp
                       adblock/adblockelementhiding.cpp
                       adblock/adblockhostmatcher.cpp
                       adblock/adblockmanager.cpp
                       adblock/adblockrule.cpp
                       adblock/adblockrulefallbackimpl.cpp
                       adblock/adblockrulenullimpl.cpp
                       adblock/adblockruletextmatchimpl.cpp
                       adblock/adblocksettingwidget.cpp
                       adblock/adblockutil.cpp
                       adblock/adblockaddsubscriptiondialog.cpp
                       adblock/adblocklistwidget.cpp
                       adblock/adblocksyntaxhighlighter.cpp
                       adblock/adblockshowlistdialog.cpp
                       adblock/adblockblockableitemsdialog.cpp
                       adblock/adblockcreatefilterdialog.cpp
                       adblock/adblockblockableitemswidget.cpp
                       )
  set(_mailwebview_lib ${KDE4_KDEWEBKIT_LIBRARY} ${QT_QTWEBKIT_LIBRARY})
  add_subdirectory(a11y)
endif()

set(libmessageviewer_viewer_SRCS
  viewer/attachmentstrategy.cpp
  viewer/bodypartformatter.cpp
  viewer/bodypartformatterfactory.cpp
  viewer/chiasmuskeyselector.cpp
  viewer/csshelper.cpp
  viewer/csshelperbase.cpp
  viewer/editorwatcher.cpp
  viewer/kleojobexecutor.cpp
  viewer/mailsourceviewer.cpp
  viewer/htmlquotecolorer.cpp
  viewer/mimetreemodel.cpp
  viewer/nodehelper.cpp
  viewer/attachmenttemporaryfilesdirs.cpp
  viewer/objecttreeemptysource.cpp
  viewer/objecttreeparser.cpp
  viewer/objecttreeparser_p.cpp
  viewer/objecttreeviewersource.cpp
  viewer/partnodebodypart.cpp
  viewer/pluginloaderbase.cpp
  viewer/viewer.cpp
  viewer/viewer_p.cpp
  viewer/messagedisplayformatattribute.cpp
  viewer/urlhandlermanager.cpp
  viewer/mimeparttreeview.cpp
)

set(libmessageviewer_widgets_SRCS
  widgets/customheadersettingdialog.cpp
  widgets/attachmentdialog.cpp
  widgets/configurewidget.cpp
  widgets/printingsettings.cpp
  widgets/htmlstatusbar.cpp
  widgets/vcardviewer.cpp
  widgets/invitationsettings.cpp
  widgets/todoedit.cpp
  widgets/eventedit.cpp
  widgets/openattachmentfolderwidget.cpp
  widgets/eventdatetimewidget.cpp
)

set(libmessageviewer_header_SRCS
  header/contactdisplaymessagememento.cpp
  header/headerstrategy.cpp
  header/headerstrategy_p.cpp
  header/headerstyle.cpp
  header/briefheaderstyle.cpp
  header/grantleeheaderstyle.cpp
  header/customheaderstyle.cpp
  header/plainheaderstyle.cpp
  header/mobileheaderstyle.cpp
  header/entrepriseheaderstyle.cpp
  header/headerstyle_util.cpp
  header/fancyheaderstyle.cpp
  header/customheadersettingwidget.cpp
  header/grantleeheaderformatter.cpp
  header/grantleeheaderteststyle.cpp
  header/kxface.cpp
)

set(libmessageviewer_scamdetection_SRCS
  scamdetection/scamdetectionwarningwidget.cpp
  scamdetection/scamdetection.cpp
  scamdetection/scamdetectiondetailsdialog.cpp
  scamdetection/scamattribute.cpp
  scamdetection/scamcheckshorturl.cpp
)

set(libmessageviewer_findbar_SRCS
  findbar/findbarbase.cpp
  findbar/findbarmailwebview.cpp
  findbar/findbarsourceview.cpp
)

set(libmessageviewer_utils_SRCS
  utils/iconnamecache.cpp
  utils/markmessagereadhandler.cpp
  utils/util.cpp
)

set(libmessageviewer_htmlwriter_SRCS
  htmlwriter/filehtmlwriter.cpp
  htmlwriter/teehtmlwriter.cpp
  htmlwriter/webkitparthtmlwriter.cpp
)

set(libmessageviewer_antispam_SRCS
  antispam/spamheaderanalyzer.cpp
  antispam/antispamconfig.cpp
)

set(libmessageviewer_job_SRCS
  job/createtodojob.cpp
  job/createeventjob.cpp
)

set(libmessageviewer_SRCS
  ${libmessageviewer_viewer_SRCS}
  ${libmessageviewer_widgets_SRCS}
  ${libmessageviewer_header_SRCS}
  ${libmessageviewer_scamdetection_SRCS}
  ${libmessageviewer_findbar_SRCS}
  ${libmessageviewer_utils_SRCS}
  ${libmessageviewer_htmlwriter_SRCS}
  ${libmessageviewer_antispam_SRCS}
  ${libmessageviewer_job_SRCS}
  settings/globalsettings.cpp
  ${_mailwebview_cpp}
)

kde4_add_kcfg_files(libmessageviewer_SRCS
  settings/globalsettings_base.kcfgc
)

add_library(messageviewer ${LIBRARY_TYPE} ${libmessageviewer_SRCS})

target_link_libraries(messageviewer messagecore kdepim kpgp kleo
  ${KDEPIMLIBS_KMIME_LIBS}
  ${KDEPIMLIBS_KMBOX_LIBS}
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
  ${KDEPIMLIBS_KPIMUTILS_LIBS}
  ${KDEPIMLIBS_KABC_LIBS}
  ${KDEPIMLIBS_MAILTRANSPORT_LIBS}
  ${KDE4_KDEUI_LIBS}
  ${KDE4_KPRINTUTILS_LIBS}
  ${QGPGME_LIBRARIES}
  ${_mailwebview_lib}
  ${Grantlee_CORE_LIBRARIES} 
  ${KDE4_KNEWSTUFF3_LIBS}
  ${QJSON_LIBRARIES}
  ${KDE4_SOLID_LIBS}
  pimcommon
  grantleetheme
  kaddressbookgrantlee
  incidenceeditorsng
)

include_directories(
    ${CMAKE_SOURCE_DIR}/grantleetheme
    ${CMAKE_SOURCE_DIR}/messagecore
    ${CMAKE_SOURCE_DIR}/pimcommon
    ${CMAKE_SOURCE_DIR}/libkdepim
    ${CMAKE_SOURCE_DIR}/libkleo
    ${CMAKE_SOURCE_DIR}/kaddressbookgrantlee
    ${Boost_INCLUDE_DIR}
    ${GPGME_INCLUDES}
)

set_target_properties( messageviewer PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS messageviewer ${INSTALL_TARGETS_DEFAULT_ARGS} )

#install( FILES viewer/viewer.h viewer/attachmentstrategy.h header/headerstrategy.h messageviewer_export.h DESTINATION  ${INCLUDE_INSTALL_DIR}/messageviewer COMPONENT devel)


install(DIRECTORY themes DESTINATION ${DATA_INSTALL_DIR}/messageviewer )

install(FILES header/data/messageviewer_header_themes.knsrc DESTINATION ${CONFIG_INSTALL_DIR} )

install(FILES notify/messageviewer.notifyrc DESTINATION ${DATA_INSTALL_DIR}/messageviewer )
install(FILES scamdetection/data/longurlServices.json DESTINATION ${DATA_INSTALL_DIR}/messageviewer )
