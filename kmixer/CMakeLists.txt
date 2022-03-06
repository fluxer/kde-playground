project(kmixer)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    find_package(KDE4 4.19.0 REQUIRED)
    include(KDE4Defaults)
    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

find_package(ALSA REQUIRED)
set_package_properties(ALSA PROPERTIES
    DESCRIPTION "Advanced Linux Sound Architecture"
    URL "https://alsa-project.org/wiki/Main_Page"
    PURPOSE "KMixer ALSA backend"
)

include_directories(${ALSA_INCLUDE_DIR})

set(kmixer_sources
    kmixer.cpp
)

add_executable(kmixer ${kmixer_sources})
target_link_libraries(kmixer
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KFILE_LIBS}
    ${ALSA_LIBRARIES}
)

install(TARGETS kmixer DESTINATION ${KDE4_BIN_INSTALL_DIR})
