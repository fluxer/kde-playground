include_directories( ${GPGME_INCLUDES} ${Boost_INCLUDE_DIR} )
if (KDEPIM_ONLY_KLEO)
  add_definitions( -DKDEPIM_ONLY_KLEO )
endif ()

#add_definitions( -DQT_NO_CAST_FROM_ASCII )
#add_definitions( -DQT_NO_CAST_TO_ASCII )


add_definitions( ${KDE4_ENABLE_EXCEPTIONS} )
add_definitions( -DGPGMEPP_ERR_SOURCE_DEFAULT=13 ) # 13 is GPG_ERR_SOURCE_KLEO, even if gpg-error's too old to know about
#parts of Boost don't build with the gcc pedantic option, so remove that option
string(REPLACE "-pedantic" "" FOO "${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS "${FOO}")

add_subdirectory( pics )
add_subdirectory( backends )
if(ENABLE_TESTING)
    add_subdirectory(tests)
endif()

########### next target ###############
set(libkleo_backend_qgpgme_SRCS
    backends/qgpgme/gnupgprocessbase.cpp
    backends/qgpgme/qgpgmeprogresstokenmapper.cpp
    backends/qgpgme/qgpgmebackend.cpp
    backends/qgpgme/threadedjobmixin.cpp
    backends/qgpgme/qgpgmekeylistjob.cpp
    backends/qgpgme/qgpgmelistallkeysjob.cpp
    backends/qgpgme/qgpgmekeygenerationjob.cpp
    backends/qgpgme/qgpgmeimportjob.cpp
    backends/qgpgme/qgpgmeimportfromkeyserverjob.cpp
    backends/qgpgme/qgpgmeexportjob.cpp
    backends/qgpgme/qgpgmesecretkeyexportjob.cpp
    backends/qgpgme/qgpgmechangeexpiryjob.cpp
    backends/qgpgme/qgpgmechangeownertrustjob.cpp
    backends/qgpgme/qgpgmechangepasswdjob.cpp
    backends/qgpgme/qgpgmedownloadjob.cpp
    backends/qgpgme/qgpgmedeletejob.cpp
    backends/qgpgme/qgpgmeencryptjob.cpp
    backends/qgpgme/qgpgmedecryptjob.cpp
    backends/qgpgme/qgpgmesignjob.cpp
    backends/qgpgme/qgpgmesignkeyjob.cpp
    backends/qgpgme/qgpgmeadduseridjob.cpp
    backends/qgpgme/qgpgmeverifydetachedjob.cpp
    backends/qgpgme/qgpgmeverifyopaquejob.cpp
    backends/qgpgme/qgpgmesignencryptjob.cpp
    backends/qgpgme/qgpgmedecryptverifyjob.cpp
    backends/qgpgme/qgpgmecryptoconfig.cpp
    backends/qgpgme/qgpgmenewcryptoconfig.cpp
    backends/qgpgme/qgpgmerefreshkeysjob.cpp
    )


set(libkleo_backend_chiasmus_SRCS
    backends/chiasmus/config_data.c
    backends/chiasmus/obtainkeysjob.cpp
    backends/chiasmus/symcryptrunprocessbase.cpp
    backends/chiasmus/chiasmusjob.cpp
    backends/chiasmus/chiasmuslibrary.cpp
    backends/chiasmus/chiasmusbackend.cpp
    )


set(libkleo_core_SRCS
   kleo/checksumdefinition.cpp
   kleo/cryptobackend.cpp
   kleo/cryptobackendfactory.cpp
   kleo/enum.cpp
   kleo/exception.cpp
   kleo/dn.cpp
   kleo/job.cpp
   kleo/multideletejob.cpp
   kleo/hierarchicalkeylistjob.cpp
   kleo/kconfigbasedkeyfilter.cpp
   kleo/keyfiltermanager.cpp
   )

set(libkleo_ui_common_SRCS
   ui/dnattributeorderconfigwidget.cpp
   ui/kdhorizontalline.cpp
   ui/filenamerequester.cpp
   ui/messagebox.cpp
   ui/cryptoconfigmodule.cpp
   ui/cryptoconfigdialog.cpp
   ui/directoryserviceswidget.cpp
   ui/progressbar.cpp
   ui/progressdialog.cpp
   )

set(libkleo_ui_SRCS    # make this a separate lib.
    ui/keylistview.cpp
    ui/keyselectiondialog.cpp
    ui/keyrequester.cpp
    ui/keyapprovaldialog.cpp
    )

set(kleo_LIB_SRCS ${libkleo_core_SRCS} ${libkleo_ui_common_SRCS} ${libkleo_backend_qgpgme_SRCS})
set(kleo_LIB_LIBS ${KDE4_KDEUI_LIBS} ${QGPGME_LIBRARIES})
if ( NOT KDEPIM_ONLY_KLEO )
   set( kleo_LIB_SRCS ${kleo_LIB_SRCS} ${libkleo_backend_chiasmus_SRCS} ${libkleo_ui_SRCS})
   set( kleo_LIB_LIBS ${kleo_LIB_LIBS} ${KDE4_KDECORE_LIBS} ${KDE4_KIO_LIBS})
endif ()

add_library(kleo ${LIBRARY_TYPE} ${kleo_LIB_SRCS})
target_link_libraries(kleo ${kleo_LIB_LIBS} )
set_target_properties(kleo PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kleo ${INSTALL_TARGETS_DEFAULT_ARGS})

install ( FILES libkleopatrarc.desktop DESTINATION ${CONFIG_INSTALL_DIR} RENAME libkleopatrarc )
