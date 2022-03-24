set(redshift_engine_SRCS
    redshiftengine.cpp
    redshiftservice.cpp
    redshiftjob.cpp
    redshiftcontroller.cpp
    redshiftcontainer.cpp
)

kde4_add_kcfg_files(redshift_engine_SRCS ../redshiftsettings.kcfgc)
kde4_add_plugin(plasma_engine_redshift ${redshift_engine_SRCS})

target_link_libraries(plasma_engine_redshift ${KDE4_PLASMA_LIBS})

install(TARGETS plasma_engine_redshift DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})
install(FILES plasma-dataengine-redshift.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR} )
install(FILES redshift.operations DESTINATION ${KDE4_DATA_INSTALL_DIR}/plasma/services)

