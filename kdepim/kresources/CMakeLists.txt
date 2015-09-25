add_definitions(
  -DKRESOURCES_DEPRECATED=
  -DKABC_DEPRECATED=
  -DKCAL_DEPRECATED=
)
# This one won't be needed when CMake 2.8.13 is depended on.
add_definitions(
  -DKRESOURCES_DEPRECATED_EXPORT=KRESOURCES_EXPORT
  -DKABC_DEPRECATED_EXPORT=KABC_EXPORT
  -DKCAL_DEPRECATED_EXPORT=KCAL_EXPORT
)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )

add_subdirectory( blog )
add_subdirectory( remote )
