add_subdirectory(dataengine)
add_subdirectory(applet)
add_subdirectory(redshiftAutostartEnabler)

install(FILES redshift.kcfg DESTINATION ${KDE4_KCFG_INSTALL_DIR})
