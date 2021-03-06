project(kholidays)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=5350)
#add_definitions( -DQT_NO_CAST_FROM_ASCII )
#add_definitions( -DQT_NO_CAST_TO_ASCII )


add_subdirectory(holidays)
if(ENABLE_TESTING)
  add_subdirectory(tests)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/parsers/plan2)

check_include_files(err.h HAVE_ERR_H)
check_include_files(sys/cdefs.h HAVE_SYS_CDEFS_H)
configure_file(config-kholidays.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kholidays.h)

########### next target ###############

set(kholidays_LIB_SRCS
  astroseasons.cpp
  holiday.cpp
  holidayregion.cpp
  holidayregionselector.cpp
  lunarphase.cpp
  sunriseset.cpp
  zodiac.cpp
  parsers/plan2/holidayscannerplan.cpp
  parsers/holidayparserdriver.cpp
  parsers/plan2/holidayparserdriverplan.cpp
  parsers/plan2/holidayparserplan.cpp
)

add_library(kholidays ${LIBRARY_TYPE} ${kholidays_LIB_SRCS})
generate_export_header(kholidays)

target_link_libraries(kholidays ${KDE4_KDEUI_LIBS} ${QT_QTGUI_LIBRARY})

set_target_properties(kholidays PROPERTIES
  VERSION ${GENERIC_LIB_VERSION}
  SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(TARGETS kholidays EXPORT kdepimlibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

########### next target ###############

if(QT_QTDESIGNER_FOUND)
  kde4_add_widget_files(${CMAKE_CURRENT_SOURCE_DIR}/kholidays.widgets)

  kde4_add_plugin(kholidayswidgets $<TARGET_OBJECTS:kholidays_autowidgets>)

  target_link_libraries(kholidayswidgets ${KDE4_KIO_LIBS} kholidays)

  install(TARGETS kholidayswidgets DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/designer)
endif()

########### install files ###############

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kholidays_export.h
  astroseasons.h
  holiday.h
  holidayregion.h
  holidays.h
  lunarphase.h
  zodiac.h
  holidayregionselector.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/kholidays COMPONENT Devel
)
