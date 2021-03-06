
add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

set(kpimidentities_LIB_SRCS identitymanager.cpp identity.cpp signature.cpp signatureconfigurator.cpp identitycombo.cpp)

if(ENABLE_TESTING)
  add_subdirectory(tests)
endif()

qt4_add_dbus_adaptor(kpimidentities_LIB_SRCS org.kde.pim.IdentityManager.xml identitymanager.h KPIMIdentities::IdentityManager )

include_directories(
  ${CMAKE_BINARY_DIR}/kpimutils
  ${CMAKE_BINARY_DIR}/kpimtextedit
)

add_library(kpimidentities ${LIBRARY_TYPE} ${kpimidentities_LIB_SRCS})
generate_export_header(kpimidentities)

target_link_libraries(kpimidentities kpimutils ${KDE4_KIO_LIBS} kpimtextedit)

set_target_properties(kpimidentities PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kpimidentities EXPORT kdepimlibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES org.kde.pim.IdentityManager.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})
install(FILES
        identity.h
        signature.h
        signatureconfigurator.h
        identitymanager.h
        identitycombo.h
        ${CMAKE_CURRENT_BINARY_DIR}/kpimidentities_export.h
        DESTINATION ${INCLUDE_INSTALL_DIR}/kpimidentities COMPONENT Devel)

