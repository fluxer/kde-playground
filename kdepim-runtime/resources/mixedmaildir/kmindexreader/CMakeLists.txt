include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${KDE4_INCLUDES}
)

if(ENABLE_TESTING)
  add_subdirectory( tests )
endif()

########### next target ###############

set(kmindexreader_LIB_SRCS
 kmindexreader.cpp
)

add_library(kmindexreader ${LIBRARY_TYPE} ${kmindexreader_LIB_SRCS})

target_link_libraries(kmindexreader
    ${KDE4_KDECORE_LIBS}
    ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
)

set_target_properties(kmindexreader PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )

install(TARGETS kmindexreader ${INSTALL_TARGETS_DEFAULT_ARGS})
