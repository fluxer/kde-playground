find_package(LibXslt)
set_package_properties(LibXslt PROPERTIES
    URL "http://xmlsoft.org/XSLT"
    TYPE REQUIRED
    PURPOSE "Required by the KDE help system to process DocBook XML"
)

find_package(LibXml2)
set_package_properties(LibXml2 PROPERTIES
    URL "http://xmlsoft.org"
    TYPE REQUIRED
    PURPOSE "Required by the KDE help system to process DocBook XML"
)

include_directories( ${KDE4_KIO_INCLUDES} )

include_directories( ${LIBXML2_INCLUDE_DIR} ${LIBXSLT_INCLUDE_DIR} )

add_subdirectory( plugins )
add_subdirectory( searchhandlers )
if(ENABLE_TESTING)
    add_subdirectory( tests )
endif()

########### next target ###############

set(kio_help_PART_SRCS
   slave/kio_help.cpp
   slave/main.cpp
   slave/xslt.cpp
   slave/xslt_help.cpp
   slave/xslt_kde.cpp
)

kde4_add_plugin(kio_help ${kio_help_PART_SRCS})

target_link_libraries(kio_help
    ${KDE4_KDECORE_LIBS}
    ${QT_QTGUI_LIBRARY}
    kio
    ${LIBXML2_LIBRARIES}
    ${LIBXSLT_LIBRARIES}
    ${LIBEXSLT_LIBRARIES}
)

install(TARGETS kio_help DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES slave/help.protocol DESTINATION ${SERVICES_INSTALL_DIR})

########### next target ###############

set(khc_indexbuilder_SRCS khc_indexbuilder.cpp )


add_executable(khc_indexbuilder ${khc_indexbuilder_SRCS})

target_link_libraries(khc_indexbuilder ${KDE4_KDECORE_LIBS} )

install(TARGETS khc_indexbuilder DESTINATION ${LIBEXEC_INSTALL_DIR})


########### next target ###############

set(khelpcenter_SRCS 
   navigator.cpp 
   navigatoritem.cpp 
   navigatorappitem.cpp 
   view.cpp 
   searchwidget.cpp 
   searchengine.cpp 
   docmetainfo.cpp 
   docentrytraverser.cpp 
   formatter.cpp 
   glossary.cpp 
   toc.cpp 
   mainwindow.cpp 
   docentry.cpp 
   htmlsearch.cpp 
   history.cpp 
   application.cpp 
   treebuilder.cpp 
   infotree.cpp 
   kcmhelpcenter.cpp 
   htmlsearchconfig.cpp 
   fontdialog.cpp 
   plugintraverser.cpp 
   scrollkeepertreebuilder.cpp 
   searchhandler.cpp )

qt4_add_dbus_adaptor( khelpcenter_SRCS org.kde.khelpcenter.kcmhelpcenter.xml kcmhelpcenter.h KCMHelpCenter )

kde4_add_kcfg_files(khelpcenter_SRCS prefs.kcfgc )

add_executable( khelpcenter ${khelpcenter_SRCS})
target_link_libraries(khelpcenter  ${KDE4_KPARTS_LIBS} ${KDE4_KDEWEBKIT_LIBS} ${QT_QTXML_LIBRARY} ${KDE4_KCMUTILS_LIBS} )
install(TARGETS khelpcenter ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install( PROGRAMS Help.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )
install( FILES khelpcenter.kcfg  DESTINATION  ${KCFG_INSTALL_DIR} )
install( FILES khelpcenter.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install( FILES khelpcenterui.rc glossary.html.in table-of-contents.xslt  	glossary.xslt index.html.in  DESTINATION  ${DATA_INSTALL_DIR}/khelpcenter )
install( FILES org.kde.khelpcenter.kcmhelpcenter.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})
