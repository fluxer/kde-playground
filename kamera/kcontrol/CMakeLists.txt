set(kcm_kamera_PART_SRCS kamera.cpp kameradevice.cpp kameraconfigdialog.cpp )

kde4_add_plugin(kcm_kamera ${kcm_kamera_PART_SRCS})

target_link_libraries(kcm_kamera
    ${KDE4_KDEUI_LIBS}
    ${GPHOTO2_LIBRARIES}
)

install(
    TARGETS kcm_kamera
    DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
)

install(
    FILES kamera.desktop
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
)
