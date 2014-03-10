cmake_minimum_required (VERSION 2.6)
project (skv)


# Shit stolen from worr
set( APPLICATION_VERSION_MAJOR "1" )
set( APPLICATION_VERSION_MINOR "0" )
set( APPLICATION_VERSION_PATCH "0" )
set( APPLICATION_VERSION_EXTRA "a1" )
set( LIBRARY_VERSION
    "${APPLICATION_VERSION_MAJOR}.${APPLICATION_VERSION_MINOR}.${APPLICATION_VERSION_PATCH}"
    )
set( LIBRARY_SOVERSION "${APPLICATION_VERSION_MAJOR}" )


option( BUILD_TESTS "Build skv unit tests" OFF )


# Deal with linker crap
set( CMAKE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_OUTPUT_DIRECTORY}/lib )
if( NOT LIB_INSTALL_DIR )
    set( libdir "${CMAKE_INSTALL_PREFIX}lib${LIB_SUFFIX}" )
else( NOT LIB_INSTALL_DIR )
    set( libdir "${CMAKE_INSTALL_PREFIX}${LIB_INSTALL_DIR}" )
endif( NOT LIB_INSTALL_DIR )
set( CMAKE_INSTALL_RPATH "${libdir}" )
set( CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE )
list( FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES
    "${libdir}" isSystemDir )
if( "${isSystemDir}" STREQUAL "-1" )
    set( CMAKE_INSTALL_RPATH "${libdir}" )
endif( "${isSystemDir}" STREQUAL "-1" )

set( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules )

include( MacroEnsureOutOfSourceBuild )
macro_ensure_out_of_source_build( "You must build outside of the source tree" )


if( BUILD_TESTS )
    enable_testing()
    add_subdirectory( library/test )
endif( BUILD_TESTS )


add_subdirectory (library/src)

