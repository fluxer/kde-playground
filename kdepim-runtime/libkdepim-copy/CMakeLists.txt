if(ENABLE_TESTING)
    add_subdirectory(tests)
endif()

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

#add_definitions(-DKDE_DEFAULT_DEBUG_AREA=5300)

########### next target ###############


set(kdepim-copy_LIB_SRCS
    calendardiffalgo.cpp
    diffalgo.cpp
    htmldiffalgodisplay.cpp
    kincidencechooser.cpp
)

add_library(kdepim-copy ${LIBRARY_TYPE} ${kdepim-copy_LIB_SRCS})

target_link_libraries(kdepim-copy
  ${KDE4_KDEUI_LIBS}
  ${KDEPIMLIBS_KCALCORE_LIBS}
  ${KDEPIMLIBS_KCALUTILS_LIBS}
)

set_target_properties(kdepim-copy PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS kdepim-copy ${INSTALL_TARGETS_DEFAULT_ARGS})
