project(plasma-wallpaper-xwallpaper)

set(xwallpaper_SRCS
    xwallpaper.cpp
)

kde4_add_plugin(plasma_wallpaper_xwallpaper ${xwallpaper_SRCS})
target_link_libraries(plasma_wallpaper_xwallpaper KDE4::plasma KDE4::kdeui)

install(
    TARGETS plasma_wallpaper_xwallpaper
    DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
)
install(
    FILES plasma-wallpaper-xwallpaper.desktop
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
)
