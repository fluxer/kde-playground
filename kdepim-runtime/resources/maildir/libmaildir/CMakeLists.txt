
if(ENABLE_TESTING)
  add_subdirectory( tests )
endif()

set(maildir_LIB_SRCS keycache.cpp maildir.cpp)

add_library(maildir ${LIBRARY_TYPE} ${maildir_LIB_SRCS})

target_link_libraries(maildir ${KDE4_KDECORE_LIBS} ${KDEPIMLIBS_KPIMUTILS_LIBS} ${KDEPIMLIBS_AKONADI_KMIME_LIBS} ${QT_QTNETWORK_LIBRARY})

set_target_properties(maildir PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )

install(TARGETS maildir ${INSTALL_TARGETS_DEFAULT_ARGS})
