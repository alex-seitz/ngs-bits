#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "testing-resources" for configuration "Debug"
set_property(TARGET testing-resources APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(testing-resources PROPERTIES
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib64/libtesting-resources.so"
  IMPORTED_SONAME_DEBUG "libtesting-resources.so"
  )

list(APPEND _cmake_import_check_targets testing-resources )
list(APPEND _cmake_import_check_files_for_testing-resources "${_IMPORT_PREFIX}/lib64/libtesting-resources.so" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
