project(calendarjanitor)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=5860)

########### next target ###############

set(calendarjanitor_SRCS
    backuper.cpp
    calendarjanitor.cpp
    collectionloader.cpp
    main.cpp
    options.cpp)

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/calendarsupport
  ${CMAKE_BINARY_DIR}/calendarsupport


  ${AKONADI_INCLUDE_DIR}
  ${Boost_INCLUDE_DIRS}
  ${QT_INCLUDES}
  ${ZLIB_INCLUDE_DIRS}
)

add_executable(calendarjanitor ${calendarjanitor_SRCS})

target_link_libraries(calendarjanitor
                      ${KDE4_KDECORE_LIBS}
                      ${KDEPIMLIBS_KCALUTILS_LIBS}
                      ${KDEPIMLIBS_KCALCORE_LIBS}
                      kdepim
                      calendarsupport
                      akonadi-calendar)

install(TARGETS calendarjanitor  ${INSTALL_TARGETS_DEFAULT_ARGS})
