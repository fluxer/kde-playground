project(kdm)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=1603)

add_executable(kdm kdm.cpp)
target_link_libraries(kdm
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    KDE4::kdeui
    KDE4::plasma
)

install(
    TARGETS kdm
    DESTINATION ${KDE4_SBIN_INSTALL_DIR}
)

########### next target ###############

add_executable(kdmhelper kdmhelper.cpp)
target_link_libraries(kdmhelper
    KDE4::kdecore
    crypt
)

install(
    TARGETS kdmhelper
    DESTINATION ${KDE4_LIBEXEC_INSTALL_DIR}
)

kde4_install_auth_helper_files(kdmhelper org.kde.kdm.helper)
