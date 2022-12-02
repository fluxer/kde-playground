project(kfirewall)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    include(FeatureSummary)

    find_package(KDE4 4.21.0 REQUIRED)
    include(KDE4Defaults)
    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

find_program(IPTABLES_EXECUTABLE iptables-restore)
add_feature_info(iptables
    IPTABLES_EXECUTABLE
    "Firewall support"
)

add_subdirectory(kcm)
add_subdirectory(kded)
