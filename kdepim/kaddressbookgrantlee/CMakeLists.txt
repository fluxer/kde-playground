project(kaddressbookgrantlee)

include_directories(
  ${Grantlee_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/grantleetheme 
)

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

if(Prison_FOUND)
  add_definitions(-DHAVE_PRISON)
  include_directories(${PRISON_INCLUDE_DIR})
endif()

set(kaddressbook_grantlee_LIB_SRCS
  widget/grantleecontactviewer.cpp
  formatter/grantleecontactformatter.cpp
  formatter/grantleecontactgroupformatter.cpp
  formatter/grantleecontactutils.cpp
  grantleeutil.cpp
)

add_library(kaddressbookgrantlee ${LIBRARY_TYPE} ${kaddressbook_grantlee_LIB_SRCS})
target_link_libraries(kaddressbookgrantlee
  ${Grantlee_CORE_LIBRARIES}
  grantleetheme
  ${KDE4_KDEUI_LIBS}
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_KABC_LIBS}
  ${KDEPIMLIBS_AKONADI_CONTACT_LIBS}
)

if(PRISON_FOUND)
  target_link_libraries(kaddressbookgrantlee ${PRISON_LIBRARIES})
endif()

set_target_properties(kaddressbookgrantlee PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS kaddressbookgrantlee ${INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)

