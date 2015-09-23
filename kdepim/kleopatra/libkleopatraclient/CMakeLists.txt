set(libkleopatraclient_version "0.3.0")
set(libkleopatraclient_soversion "0")

add_subdirectory(core)
add_subdirectory(gui)

if(ENABLE_TESTING)
    add_subdirectory(tests)
endif()

