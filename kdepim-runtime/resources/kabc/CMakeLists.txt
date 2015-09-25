add_definitions(
  -DKRESOURCES_DEPRECATED=
  -DKABC_DEPRECATED=
  -DKCAL_DEPRECATED=
)
# This one won't be needed when CMake 2.8.13 is depended on.
add_definitions(
  -DKRESOURCES_DEPRECATED_EXPORT=KRESOURCES_EXPORT
  -DKABC_DEPRECATED_EXPORT=KABC_EXPORT
  -DKCAL_DEPRECATED_EXPORT=KCAL_EXPORT
)

include_directories(
  ${kdepim-runtime_SOURCE_DIR}
  ${QT_QTDBUS_INCLUDE_DIR}
)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )


########### next target ###############

set( kabcresource_SRCS
  kabcresource.cpp
  kresourceassistant.cpp
)

install( FILES kabcresource.desktop DESTINATION "${CMAKE_INSTALL_PREFIX}/share/akonadi/agents" )

add_executable(akonadi_kabc_resource ${kabcresource_SRCS})

target_link_libraries(akonadi_kabc_resource ${KDEPIMLIBS_AKONADI_LIBS} ${QT_QTCORE_LIBRARY} ${QT_QTDBUS_LIBRARY} ${KDE4_KDECORE_LIBS} ${KDEPIMLIBS_KABC_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS akonadi_kabc_resource ${INSTALL_TARGETS_DEFAULT_ARGS})
