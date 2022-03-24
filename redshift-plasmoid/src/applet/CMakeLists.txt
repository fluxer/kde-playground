set(redshiftapplet_SRCS
    redshiftapplet.cpp
    redshiftosdwidget.cpp
    redshift.ui
)

kde4_add_kcfg_files(redshiftapplet_SRCS ../redshiftsettings.kcfgc)
kde4_add_plugin(plasma_applet_redshift ${redshiftapplet_SRCS})

target_link_libraries(plasma_applet_redshift ${KDE4_PLASMA_LIBS})

install(TARGETS plasma_applet_redshift DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-redshift.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR})
