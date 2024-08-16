#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "AWS::s2n" for configuration "Debug"
set_property(TARGET AWS::s2n APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(AWS::s2n PROPERTIES
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib64/libs2n.so.1.0.0"
  IMPORTED_SONAME_DEBUG "libs2n.so.1"
  )

list(APPEND _cmake_import_check_targets AWS::s2n )
list(APPEND _cmake_import_check_files_for_AWS::s2n "${_IMPORT_PREFIX}/lib64/libs2n.so.1.0.0" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
