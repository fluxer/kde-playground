project(kfilereplace)

find_package(KDE4 4.14.3 REQUIRED)

include(KDE4Defaults)
include(MacroLibrary)

add_definitions(${QT_DEFINITIONS} ${QT_QTDBUS_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})

add_subdirectory( toolbar )

########### next target ###############

set(kfilereplacepart_PART_SRCS 
   kfilereplacepart.cpp 
   kfilereplaceview.cpp 
   kaddstringdlg.cpp 
   knewprojectdlg.cpp 
   koptionsdlg.cpp 
   kfilereplacelib.cpp 
   report.cpp 
   commandengine.cpp 
   configurationclasses.cpp
   kfilereplaceviewwdg.ui 
   kaddstringdlgs.ui 
   knewprojectdlgs.ui 
   koptionsdlgs.ui )

qt4_add_dbus_adaptor( kfilereplacepart_PART_SRCS org.kde.kfilereplace.xml kfilereplacepart.h KFileReplacePart )

kde4_add_plugin(kfilereplacepart WITH_PREFIX ${kfilereplacepart_PART_SRCS})

target_link_libraries(kfilereplacepart  ${KDE4_KDECORE_LIBS} ${KDE4_KPARTS_LIBS} )

install(TARGETS kfilereplacepart  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kfilereplace_SRCS kfilereplace.cpp main.cpp )


add_executable(kfilereplace ${kfilereplace_SRCS})

target_link_libraries(kfilereplace  ${KDE4_KPARTS_LIBS} )

install(TARGETS kfilereplace  ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### install files ###############

install( FILES  kfilereplace.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
install( FILES  kfilereplacepartui.rc  DESTINATION ${DATA_INSTALL_DIR}/kfilereplacepart)
install( FILES  kfilereplaceui.rc  DESTINATION ${DATA_INSTALL_DIR}/kfilereplace)
install( FILES  kfilereplacepart.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES org.kde.kfilereplace.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR} )

kde4_install_icons( ${ICON_INSTALL_DIR}   )

