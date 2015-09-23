# Those paths need to be absolute since this file is included from other CMakeLists.txt files
set(libkdgantt2_SRCS
    kdganttabstractgrid.cpp
    kdganttabstractrowcontroller.cpp
    kdganttconstraint.cpp
    kdganttconstraintgraphicsitem.cpp
    kdganttconstraintmodel.cpp
    kdganttconstraintproxy.cpp
    kdganttdatetimegrid.cpp
    kdganttforwardingproxymodel.cpp
    kdganttglobal.cpp
    kdganttgraphicsitem.cpp
    kdganttgraphicsscene.cpp
    kdganttgraphicsview.cpp
    kdganttitemdelegate.cpp
    kdganttlegend.cpp
    kdganttlistviewrowcontroller.cpp
    kdganttproxymodel.cpp
    kdganttstyleoptionganttitem.cpp
    kdganttsummaryhandlingproxymodel.cpp
    kdgantttreeviewrowcontroller.cpp
    kdganttview.cpp
)

add_definitions(-DKDAB_NO_UNIT_TESTS -DKDGANTT_SHAREDLIB -DKDGANTT_BUILD_KDGANTT_LIB)

add_library(kdgantt2 ${LIBRARY_TYPE} ${libkdgantt2_SRCS})
target_link_libraries(kdgantt2  ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} )

set_target_properties( kdgantt2 PROPERTIES VERSION 0.1.0 SOVERSION 0)
install(TARGETS kdgantt2 ${INSTALL_TARGETS_DEFAULT_ARGS} )
