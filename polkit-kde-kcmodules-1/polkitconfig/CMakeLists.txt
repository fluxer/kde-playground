include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR})

set( kcm_polkitconfig_SRCS
     kcmpolkitconfig.cpp
)

kde4_add_plugin(kcm_polkitconfig ${kcm_polkitconfig_SRCS})

target_link_libraries(kcm_polkitconfig  
     ${KDE4_KDECORE_LIBS}
     ${KDE4_KDEUI_LIBRARY}
     polkitkdekcmodulesprivate
)

########### install files ###############

install(
    TARGETS kcm_polkitconfig
    DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
)

install(
    FILES kcm_polkitconfig.desktop
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
)
