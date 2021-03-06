project(ktimetracker)

#We check if X11_Xscreensaver was found
if(X11_Xscreensaver_FOUND)
  message(STATUS "Found the X11 screensaver extension")
  macro_bool_to_01(X11_Xscreensaver_LIB HAVE_LIBXSS)
else()
  message(STATUS "The X11 screensaver extension was NOT found.")
endif()

add_feature_info("KtimeTracker idle detection" X11_Xscreensaver_FOUND "Measure the screen idle time in KTimeTracker")

configure_file(config-ktimetracker.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-ktimetracker.h )

add_subdirectory( support )
add_subdirectory( pics )

include_directories( ${CMAKE_SOURCE_DIR}/kresources/remote )


########### next target ###############
set(ktimetracker_shared_SRCS ktimetrackerconfigdialog.cpp
	desktoptracker.cpp ktimetrackerpart.cpp
	edittaskdialog.cpp idletimedetector.cpp
	timekard.cpp ktimetrackerutility.cpp
	timetrackerstorage.cpp mainwindow.cpp preferences.cpp
	task.cpp taskview.cpp tray.cpp focusdetector.cpp
	csvexportdialog.cpp plannerparser.cpp historydialog.cpp
	treeviewheadercontextmenu.cpp timetrackerwidget.cpp
  focusdetectornotifier.cpp kttcalendar.cpp
 )

qt4_add_dbus_adaptor( ktimetracker_shared_SRCS
        org.kde.ktimetracker.ktimetracker.xml
        timetrackerwidget.h
        TimetrackerWidget
        mainadaptor
        MainAdaptor )

qt4_add_dbus_adaptor( ktimetracker_shared_SRCS
        org.kde.ktimetracker.storage.xml
        taskview.h
        TaskView
        storageadaptor
        StorageAdaptor )

kde4_add_kcfg_files( ktimetracker_shared_SRCS ktimetracker.kcfgc )

set(ktimetracker_SRCS main.cpp ${ktimetracker_shared_SRCS})

add_executable(karm karm.cpp)

add_executable(ktimetracker ${ktimetracker_SRCS})


target_link_libraries(karm  ${KDE4_KIO_LIBS} ${KDEPIMLIBS_KCALCORE_LIBS} ${KDEPIMLIBS_KONTACTINTERFACE_LIBS} )

target_link_libraries(ktimetracker   ${KDE4_KCMUTILS_LIBS} ${KDE4_KPARTS_LIBRARY} ${KDEPIMLIBS_KCALCORE_LIBS} ${KDEPIMLIBS_KONTACTINTERFACE_LIBS} ${X11_X11_LIB})
if(X11_Xscreensaver_LIB)
	target_link_libraries(ktimetracker  ${X11_Xscreensaver_LIB} )
endif()

install( TARGETS karm  ${INSTALL_TARGETS_DEFAULT_ARGS} )
install( TARGETS ktimetracker  ${INSTALL_TARGETS_DEFAULT_ARGS} )


###########

set(kcm_ktimetracker_PART_SRCS ${ktimetracker_shared_SRCS})

kde4_add_plugin(kcm_ktimetracker ${kcm_ktimetracker_PART_SRCS})

target_link_libraries(kcm_ktimetracker   ${KDE4_KIO_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KCMUTILS_LIBS} ${KDE4_KPARTS_LIBRARY} ${KDEPIMLIBS_KCALCORE_LIBS} ${X11_X11_LIB})
if(X11_Xscreensaver_LIB)
    target_link_libraries(kcm_ktimetracker  ${X11_Xscreensaver_LIB} )
endif()

install(TARGETS kcm_ktimetracker DESTINATION ${PLUGIN_INSTALL_DIR})



########### next target ###############

set(ktimetrackerpart_PART_SRCS ${ktimetracker_shared_SRCS})

kde4_add_plugin(ktimetrackerpart ${ktimetrackerpart_PART_SRCS})

target_link_libraries(ktimetrackerpart ${KDE4_KCMUTILS_LIBS}  ${KDEPIMLIBS_KCALCORE_LIBS}  ${KDE4_KIO_LIBS} ${KDE4_KPARTS_LIBS} ${X11_X11_LIB})
if(X11_Xscreensaver_LIB)
	target_link_libraries(ktimetrackerpart  ${X11_Xscreensaver_LIB})
endif()


install(TARGETS ktimetrackerpart  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############
install( FILES ktimetracker_config_behavior.desktop  ktimetracker_config_display.desktop  ktimetracker_config_storage.desktop DESTINATION ${SERVICES_INSTALL_DIR})

install( FILES ktimetrackerpart.desktop  DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES ktimetrackerui.rc  DESTINATION ${DATA_INSTALL_DIR}/ktimetracker )

kde4_install_icons( ${ICON_INSTALL_DIR} )
install( FILES org.kde.ktimetracker.ktimetracker.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR} )
