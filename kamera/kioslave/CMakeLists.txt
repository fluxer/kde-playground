set(kio_kamera_PART_SRCS kamera.cpp )

kde4_add_plugin(kio_kamera ${kio_kamera_PART_SRCS})

target_link_libraries(kio_kamera
    ${KDE4_KIO_LIBS}
    ${GPHOTO2_LIBRARIES}
)

install(
    TARGETS kio_kamera
    DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
)

install(
    FILES camera.protocol
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
)
