
########### next target ###############

set(kcm_kamera_PART_SRCS kamera.cpp kameradevice.cpp kameraconfigdialog.cpp )


kde4_add_plugin(kcm_kamera ${kcm_kamera_PART_SRCS})


target_link_libraries(kcm_kamera ${KDE4_KDEUI_LIBS} ${QT_QT3SUPPORT_LIBRARY} ${GPHOTO2_LIBRARIES})

install(TARGETS kcm_kamera  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kamera.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

