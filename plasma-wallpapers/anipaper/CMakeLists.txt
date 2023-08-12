project(plasma-wallpaper-anipaper)

set(anipaper_SRCS
    anipaper.cpp
)

kde4_add_plugin(plasma_wallpaper_anipaper ${anipaper_SRCS})
target_link_libraries(plasma_wallpaper_anipaper KDE4::plasma KDE4::kdeui)

install(
    TARGETS plasma_wallpaper_anipaper
    DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
)
install(
    FILES plasma-wallpaper-anipaper.desktop
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
)
