# Use Monero's GTest from parent directory
# TODO: will move to external/gtest: https://github.com/monero-project/monero/pull/9715
set(GTEST_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../tests/gtest)
set(GTEST_INCLUDE_DIRS ${GTEST_ROOT}/include)

# Add GTest as a subdirectory
add_subdirectory(${GTEST_ROOT} gtest)

# C++ test executable, to check the OTS C++ library
add_executable(gtest_ots.cpp
	gtest_ots.cpp
)

target_link_libraries(gtest_ots.cpp
	PRIVATE
	utf8proc
	monero-ots
	gtest
	gtest_main
)

# Include Monero's GTest directories
target_include_directories(gtest_ots.cpp PRIVATE 
	${GTEST_ROOT}/include
)

# C++/C sync test executable, to check if the C++ and C libraries are in sync
add_executable(gtest_cpp_c_sync.cpp
	gtest_cpp_c_sync.cpp
)

target_link_libraries(gtest_cpp_c_sync.cpp
	PRIVATE
	monero-ots
	gtest
	gtest_main
)

# Include Monero's GTest directories
target_include_directories(gtest_cpp_c_sync.cpp PRIVATE 
	${GTEST_ROOT}/include
	${GTEST_ROOT}/../../src
	${GTEST_ROOT}/../../contrib/epee/include
	${GTEST_ROOT}/../../external/easylogging++
)

# Blocktime test executable (only if BUILD_BLOCKTIME_TESTS is enabled)
if(BUILD_BLOCKTIME_TESTS)
	add_executable(gtest_blocktime
		gtest_blocktime.cpp
	)

	target_link_libraries(gtest_blocktime
		PRIVATE
		monero-ots
		blocktime
		gtest
		gtest_main
	)

	target_include_directories(gtest_blocktime PRIVATE 
		${GTEST_ROOT}/include
		${CMAKE_SOURCE_DIR}/../blocktime/include
	)
endif()

# Use local Check framework
set(CHECK_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/check)
set(CHECK_INCLUDE_DIRS
	${CHECK_ROOT}/src
	${CHECK_ROOT}/../include
)

# Add Check as a subdirectory
add_subdirectory(${CHECK_ROOT} check)

# C ABI modules tests (check_otc_c)
file(GLOB MODULE_TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/c-abi/*.c")
# Create the file for suite declarations
set(SUITE_HEADER "${CMAKE_BINARY_DIR}/generated_suites.h")
# Write includes and declarations to the header
file(WRITE ${SUITE_HEADER} "// Automatically generated Suite declarations\n\n")
foreach(TEST_FILE ${MODULE_TEST_SOURCES})
    get_filename_component(TEST_BASENAME ${TEST_FILE} NAME_WE)
    file(APPEND ${SUITE_HEADER} "Suite* ${TEST_BASENAME}_suite(void);\n")
endforeach()
# Write macro for including all suites
file(APPEND ${SUITE_HEADER} "\n#define INCLUDED_SUITES \\\n")
foreach(TEST_FILE ${MODULE_TEST_SOURCES})
    get_filename_component(TEST_BASENAME ${TEST_FILE} NAME_WE)
    file(APPEND ${SUITE_HEADER} "    srunner_add_suite(sr, ${TEST_BASENAME}_suite()); \\\n")
endforeach()
file(APPEND ${SUITE_HEADER} "// do not edit manually\n")

# C test executable, to check the OTS C ABI library
add_executable(check_ots_c
	check_ots.c
	data.cpp
	${MODULE_TEST_SOURCES}
)

target_include_directories(check_ots_c
	PRIVATE
	${CMAKE_BINARY_DIR}
	${CHECK_INCLUDE_DIRS}
)

target_link_libraries(check_ots_c
	PRIVATE
	check
	monero-ots
	pthread
	stdc++
)

# Register tests
add_test(NAME cpp_tests COMMAND gtest_ots.cpp)
add_test(NAME sync_tests COMMAND gtest_cpp_c_sync.cpp)
add_test(NAME c_tests COMMAND check_ots_c)

# Set test properties
set_tests_properties(c_tests PROPERTIES
	ENVIRONMENT "LD_LIBRARY_PATH=${CMAKE_BINARY_DIR}"
)
set_tests_properties(cpp_tests PROPERTIES
	ENVIRONMENT "LD_LIBRARY_PATH=${CMAKE_BINARY_DIR}"
)
set_tests_properties(sync_tests PROPERTIES
	ENVIRONMENT "LD_LIBRARY_PATH=${CMAKE_BINARY_DIR}"
)

# Register blocktime tests if enabled
if(BUILD_TESTS AND BUILD_BLOCKTIME_TESTS)
	add_test(NAME blocktime_tests COMMAND gtest_blocktime)
	set_tests_properties(blocktime_tests PROPERTIES
		ENVIRONMENT "LD_LIBRARY_PATH=${CMAKE_BINARY_DIR}"
	)
endif()
