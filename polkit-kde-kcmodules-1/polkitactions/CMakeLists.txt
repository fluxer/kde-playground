include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

set(kcm_polkitactions_SRCS
    PolkitActionsKCM.cpp
    AuthorizationsFilterModel.cpp
    PoliciesModel.cpp
    PolicyItem.cpp
    ActionWidget.cpp
    explicitauthorizationdialog.cpp
    pkitemdelegate.cpp
)

kde4_add_plugin(kcm_polkitactions ${kcm_polkitactions_SRCS})

target_link_libraries(kcm_polkitactions
     ${KDE4_KDECORE_LIBS}
     ${KDE4_KDEUI_LIBRARY}
     ${POLKITQT-1_CORE_LIBRARY}
     polkitkdekcmodulesprivate
)

########### install files ###############

install(
    TARGETS kcm_polkitactions
    DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
)

install(
    FILES kcm_polkitactions.desktop
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
)
