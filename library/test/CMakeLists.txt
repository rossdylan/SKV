set( TEST_EXECUTABLES test_mem_btree test_disk_tree)

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_OUTPUT_DIRECTORY}/tests )
find_package(Check REQUIRED)

include_directories(
    ${CMAKE_SOURCE_DIR}/library/src
    ${CHECK_LIBRARIES}
)


set( SOURCES
    ${CMAKE_SOURCE_DIR}/library/src/btree.c
    ${CMAKE_SOURCE_DIR}/library/src/disktree.c
)

set( LIBS ${LIBS} ${CHECK_LIBRARIES} )

foreach( TEST_EXECUTABLE ${TEST_EXECUTABLES} )
    add_executable(
        ${TEST_EXECUTABLE}
        ${TEST_EXECUTABLE}.c
        ${SOURCES}
    )
    target_link_libraries( ${TEST_EXECUTABLE} ${LIBS} )
    add_test( ${TEST_EXECUTABLE} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TEST_EXECUTABLE} )
endforeach( TEST_EXECUTABLE )
