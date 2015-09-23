if(ASSUAN2_FOUND)
  include_directories(${ASSUAN2_INCLUDES} ${CMAKE_SOURCE_DIR}/kleopatra)
else()
  include_directories(${ASSUAN_INCLUDES} ${CMAKE_SOURCE_DIR}/kleopatra)
endif()

add_definitions(
  -D_ASSUAN_ONLY_GPG_ERRORS
  -DQT_NO_CAST_FROM_ASCII -DQT_NO_KEYWORDS -DQT_NO_CAST_TO_ASCII
)

add_library(kleopatraclientcore ${LIBRARY_TYPE}
  initialization.cpp
  command.cpp
  selectcertificatecommand.cpp
  signencryptfilescommand.cpp
  decryptverifyfilescommand.cpp
)

set_target_properties(kleopatraclientcore PROPERTIES
  VERSION ${libkleopatraclient_version}
  SOVERSION ${libkleopatraclient_soversion}
)

if(ASSUAN2_FOUND)
target_link_libraries(kleopatraclientcore
    ${QT_QTCORE_LIBRARY}
    ${ASSUAN2_LIBRARIES}
)
else()
target_link_libraries(kleopatraclientcore
    ${QT_QTCORE_LIBRARY}
    ${ASSUAN_PTHREAD_LIBRARIES}
)
endif()

install(TARGETS kleopatraclientcore ${INSTALL_TARGETS_DEFAULT_ARGS})
