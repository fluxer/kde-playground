
include_directories(
    ${Boost_INCLUDE_DIR}
    tests
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS} -fPIC")

set(migrationagent_SRCS
    migrationagent.cpp
    migrationstatuswidget.cpp
    migrationexecutor.cpp
    migrationscheduler.cpp
    tests/dummymigrator.cpp
)

add_executable(akonadi_migration_agent ${migrationagent_SRCS})

target_link_libraries(akonadi_migration_agent
    gidmigration
    ${KDE4_KDEUI_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    ${KDEPIMLIBS_AKONADI_LIBS}
    ${KDEPIMLIBS_KABC_LIBS}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTDBUS_LIBRARY}
    ${KDE4_KIDLETIME_LIBRARY}
    ${KDE4_SOLID_LIBS}
)

install(TARGETS akonadi_migration_agent ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES migrationagent.desktop DESTINATION "${CMAKE_INSTALL_PREFIX}/share/akonadi/agents")

if(ENABLE_TESTING)
    add_subdirectory(tests)
endif()
