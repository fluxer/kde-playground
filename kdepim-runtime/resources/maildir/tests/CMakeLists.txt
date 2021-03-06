set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

# Stolen from kdepimlibs/akonadi/tests
macro(add_akonadi_isolated_test _source)
  get_filename_component(_targetName ${_source} NAME_WE)
  set(_srcList ${_source} )

  kde4_add_executable(${_targetName} TEST ${_srcList})
  target_link_libraries(${_targetName}
    ${QT_QTTEST_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${KDEPIMLIBS_AKONADI_LIBS}
    ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
    ${KDE4_KDECORE_LIBS}
    ${KDEPIMLIBS_MAILTRANSPORT_LIBS}
    ${KDEPIMLIBS_KMIME_LIBS}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTDBUS_LIBRARY}
  )

  # based on kde4_add_unit_test
  if (WIN32)
    get_target_property( _loc ${_targetName} LOCATION )
    set(_executable ${_loc}.bat)
  else ()
    set(_executable ${EXECUTABLE_OUTPUT_PATH}/${_targetName})
  endif ()
  if (UNIX)
    set(_executable ${_executable}.shell)
  endif ()

  find_program(_testrunner akonaditest)

  if (KDEPIM_RUN_ISOLATED_TESTS)
    add_test( maildir-${_targetName} ${_testrunner} -c ${CMAKE_CURRENT_SOURCE_DIR}/unittestenv/config.xml ${_executable} )
  endif ()
endmacro(add_akonadi_isolated_test)



add_akonadi_isolated_test( synctest.cpp )
akonadi_add_resourcetest( maildir maildir.js )

