include_directories(${KDE4_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KIPI_INCLUDE_DIR} ${QTGSTREAMER_INCLUDE_DIR} ${GLIB2_INCLUDE_DIR} ${GSTREAMER_INCLUDE_DIR})

add_definitions(-DGST_DISABLE_DEPRECATED)
add_definitions(-fexceptions)

add_subdirectory(plugins)
add_subdirectory(pluginTester)

set(kamoso_SRCS
	burstshootmode.cpp
	videoshootmode.cpp
	photoshootmode.cpp
	shootmode.cpp 
	thumbnailview.cpp
	kamoso.cpp
	main.cpp
	countdownwidget.cpp
	webcamwidget.cpp
	customDelegate.cpp
	device.cpp
	devicemanager.cpp
	kamosojobtracker.cpp 
	kipiinterface.cpp
	kamosoimagecollectionshared.cpp
	infoshared.cpp
	kipiaction.cpp
	webcamdialog.cpp
	pagewebcamconfigmanager.cpp
	mainWidget.ui
	generalConfig.ui
	pictureConfig.ui
	videoConfig.ui
)
qt4_add_dbus_interface(kamoso_SRCS
    org.kde.Solid.PowerManagement.xml
    brightness_interface)

set(kamoso_UI )

kde4_add_kcfg_files(kamoso_SRCS settings.kcfgc)
add_executable(kamoso ${kamoso_SRCS})

target_link_libraries(kamoso ${KDE4_KDEUI_LIBS} ${KDE4_KFILE_LIBS} ${KDE4_KIO_LIBS}
        ${KDE4_SOLID_LIBS} ${KDE4_KMEDIAPLAYER_LIBS} ${KDE4_KUTILS_LIBS} ${KIPI_LIBRARIES}
        ${SolidControl} ${QTGSTREAMER_LIBRARY} ${QTGSTREAMER_UI_LIBRARY}
)

install(TARGETS kamoso ${INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS kamoso.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install(FILES kamosoplugin.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})
