set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(
 ../
)

MACRO(IE_UNIT_TESTS)
  FOREACH(_testname ${ARGN})
    kde4_add_unit_test(${_testname} NOGUI ${_testname}.cpp modeltest.cpp)
    target_link_libraries(${_testname} ${QT_QTTEST_LIBRARY} ${QT_QTGUI_LIBRARY} ${KDE4_KDEUI_LIBS} ${KDEPIMLIBS_AKONADI_LIBS}  kdgantt2 ${KDEPIMLIBS_KCALUTILS_LIBS} ${KDEPIMLIBS_KCALCORE_LIBS} incidenceeditorsng kdepimdbusinterfaces)
  ENDFOREACH(_testname)
ENDMACRO(IE_UNIT_TESTS)

MACRO(IE_EXECUTABLE_TESTS)
  FOREACH(_testname ${ARGN})
    kde4_add_executable(${_testname} NOGUI TEST ${_testname}.cpp modeltest.cpp)
    target_link_libraries(${_testname} ${KDEPIMLIBS_KCALCORE_LIBS} ${QT_QTTEST_LIBRARY} ${QT_QTCORE_LIBRARY} ${KDE4_KDECORE_LIBS} incidenceeditorsng)
  ENDFOREACH(_testname)
ENDMACRO(IE_EXECUTABLE_TESTS)

IE_UNIT_TESTS(
  conflictresolvertest
  testfreebusyitemmodel
  testfreeperiodmodel
  testfreebusyganttproxymodel
)

########### KTimeZoneComboBox unit test #############
set(ktimezonecomboboxtest_SRCS ktimezonecomboboxtest.cpp)

kde4_add_unit_test(ktimezonecomboboxtest TESTNAME ktimezonecomboboxtest ${ktimezonecomboboxtest_SRCS})

target_link_libraries(
  ktimezonecomboboxtest
  ${KDE4_KDEUI_LIBS}
  ${QT_QTTEST_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDEPIMLIBS_KCALCORE_LIBS}
  incidenceeditorsng
)

