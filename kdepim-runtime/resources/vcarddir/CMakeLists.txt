include_directories(
    ${kdepim-runtime_SOURCE_DIR}
    ${QT_QTDBUS_INCLUDE_DIR}
)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

set(CMAKE_AUTOUIC OFF)

########### next target ###############

set( vcarddirresource_SRCS
  vcarddirresource.cpp
  ../shared/dirsettingsdialog.cpp
)

qt4_wrap_ui(vcarddirresource_SRCS settingsdialog.ui)

kde4_add_kcfg_files(vcarddirresource_SRCS settings.kcfgc)
kcfg_generate_dbus_interface(${CMAKE_CURRENT_SOURCE_DIR}/vcarddirresource.kcfg org.kde.Akonadi.VCardDirectory.Settings)
qt4_add_dbus_adaptor(vcarddirresource_SRCS
  ${CMAKE_CURRENT_BINARY_DIR}/org.kde.Akonadi.VCardDirectory.Settings.xml settings.h Settings
)

install( FILES vcarddirresource.desktop DESTINATION "${CMAKE_INSTALL_PREFIX}/share/akonadi/agents" )

add_executable(akonadi_vcarddir_resource ${vcarddirresource_SRCS})

target_link_libraries(akonadi_vcarddir_resource
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${QT_QTCORE_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDEPIMLIBS_KABC_LIBS}
  ${KDE4_KIO_LIBS}
)

add_subdirectory(wizard)

install(TARGETS akonadi_vcarddir_resource ${INSTALL_TARGETS_DEFAULT_ARGS})
