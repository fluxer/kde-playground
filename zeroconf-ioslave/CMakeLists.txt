project(kdnssd)

if(NOT INSIDE_KDENETWORK)
    message("Not building inside KDENetwork, loading KDE CMake Macros.")

    find_package(KDE4 REQUIRED)

    include(KDE4Defaults)
    include(MacroLibrary)

    include(CheckIncludeFile)
    include(CheckIncludeFiles)
    include(CheckSymbolExists)
    include(CheckFunctionExists)
    include(CheckLibraryExists)
    include(CheckTypeSize)

    set(CMAKE_REQUIRED_DEFINITIONS ${_KDE4_PLATFORM_DEFINITIONS})
    add_definitions(${QT_DEFINITIONS} ${QT_QTDBUS_DEFINITIONS} ${KDE4_DEFINITIONS})
    include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})
endif(NOT INSIDE_KDENETWORK)

add_subdirectory( ioslave ) 
add_subdirectory( kdedmodule ) 
