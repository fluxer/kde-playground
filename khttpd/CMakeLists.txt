project(khttpd)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    find_package(KDE4 4.19.0 REQUIRED)
    include(KDE4Defaults)
    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

set(khttpd_sources
    khttpd.cpp
)

add_executable(khttpd ${khttpd_sources})
target_link_libraries(khttpd
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KDNSSD_LIBS}
    ${KDE4_KHTTP_LIBS}
)

install(TARGETS khttpd DESTINATION ${KDE4_BIN_INSTALL_DIR})
