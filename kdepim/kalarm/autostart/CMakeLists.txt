include_directories( ../  )

########### next target ###############

set(kalarmautostart_SRCS 
	autostart.cpp 
)

add_executable(kalarmautostart ${kalarmautostart_SRCS})

target_link_libraries(kalarmautostart ${KDE4_KDEUI_LIBS} )

install(TARGETS kalarmautostart  ${INSTALL_TARGETS_DEFAULT_ARGS} )
