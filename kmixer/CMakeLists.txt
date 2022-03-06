project(kmixer)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    find_package(KDE4 4.19.0 REQUIRED)
    include(KDE4Defaults)
    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

set(kmixer_sources
    kmixer.cpp
)

add_executable(kmixer ${kmixer_sources})
target_link_libraries(kmixer
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KFILE_LIBS}
    asound
)

install(TARGETS kmixer DESTINATION ${KDE4_BIN_INSTALL_DIR})
