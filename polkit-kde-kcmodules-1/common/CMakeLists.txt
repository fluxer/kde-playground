include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

set(polkitkdekcmodulesprivate_SRCS
     PKLAEntry.cpp
     identitywidget.cpp
)

add_library(polkitkdekcmodulesprivate SHARED ${polkitkdekcmodulesprivate_SRCS})

set_target_properties(polkitkdekcmodulesprivate PROPERTIES
    VERSION   0.1.0
    SOVERSION 0
)

target_link_libraries(polkitkdekcmodulesprivate  
     ${KDE4_KDECORE_LIBS}
     ${KDE4_KDEUI_LIBRARY}
)

install(
    TARGETS polkitkdekcmodulesprivate
    DESTINATION ${KDE4_LIB_INSTALL_DIR}
)
