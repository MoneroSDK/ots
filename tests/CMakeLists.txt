# Require C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Require C 11
set(C_STANDARD 11)
set(C_STANDARD_REQUIRED ON)

if(BUILD_SHARED_LIBS)
	set(LIB_TYPE SHARED)
	set(CMAKE_ASM_CREATE_SHARED_LIBRARY ON)
else()
	set(LIB_TYPE STATIC)
	set(SWITCH_STATIC "-static")
	set(SWITCH_STATIC_LIB "STATICLIB")
endif()

# Use Monero's GTest from parent directory
# TODO: will move to external/gtest: https://github.com/monero-project/monero/pull/9715
set(GTEST_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../tests/gtest)
set(GTEST_INCLUDE_DIRS ${GTEST_ROOT}/include)

# Add GTest as a subdirectory
add_subdirectory(${GTEST_ROOT} gtest)

# Auto-generate header file from *.{outputs,unsigned_tx} files
set(GENERATED_HEADERS_DIR "${CMAKE_BINARY_DIR}/tests/include")
file(MAKE_DIRECTORY ${GENERATED_HEADERS_DIR})
# outputs
file(GLOB OUTPUTS_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.outputs")
set(OUTPUTS_HEADER_FILE "${GENERATED_HEADERS_DIR}/data-wallet-outputs.hpp")
set(OUTPUTS_MACROS "")
foreach(OUTPUTS_FILE ${OUTPUTS_FILES})
	get_filename_component(HASH_ID ${OUTPUTS_FILE} NAME_WE)
	set(MACRO_NAME "WALLET_${HASH_ID}_OUTPUTS")
	list(APPEND OUTPUTS_MACROS "${OUTPUTS_FILE}:${MACRO_NAME}")
endforeach()
if(OUTPUTS_MACROS)
	add_custom_command(
		OUTPUT ${OUTPUTS_HEADER_FILE}
		COMMAND ${CMAKE_BINARY_DIR}/tools/binheadermacro/bin2hppmacro ${OUTPUTS_HEADER_FILE} ${OUTPUTS_MACROS}
		DEPENDS ${OUTPUTS_FILES} tools/binheadermacro/bin2hppmacro
		COMMENT "Generating header macros for ${OUTPUTS_HEADER_FILE}"
		VERBATIM
	)
	list(APPEND GENERATED_HEADERS ${OUTPUTS_HEADER_FILE})
endif()
# unsigned_tx
file(GLOB UNSIGNED_TX_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.unsigned_tx")
set(UNSIGNED_TX_HEADER_FILE "${GENERATED_HEADERS_DIR}/data-wallet-unsigned-tx.hpp")
set(UNSIGNED_TX_MACROS "")
foreach(UNSIGNED_TX_FILE ${UNSIGNED_TX_FILES})
	get_filename_component(HASH_ID ${UNSIGNED_TX_FILE} NAME_WE)
	set(MACRO_NAME "WALLET_${HASH_ID}_UNSIGNED_TX")
	list(APPEND UNSIGNED_TX_MACROS "${UNSIGNED_TX_FILE}:${MACRO_NAME}")
endforeach()
if(UNSIGNED_TX_MACROS)
	add_custom_command(
		OUTPUT ${UNSIGNED_TX_HEADER_FILE}
		COMMAND ${CMAKE_BINARY_DIR}/tools/binheadermacro/bin2hppmacro ${UNSIGNED_TX_HEADER_FILE} ${UNSIGNED_TX_MACROS}
		DEPENDS ${UNSIGNED_TX_FILES} tools/binheadermacro/bin2hppmacro
		COMMENT "Generating header macros for ${UNSIGNED_TX_HEADER_FILE}"
		VERBATIM
	)
	list(APPEND GENERATED_HEADERS ${UNSIGNED_TX_HEADER_FILE})
endif()
# unsigned_tx_json
file(GLOB UNSIGNED_TX_JSON_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.unsigned_tx_json")
set(UNSIGNED_TX_JSON_HEADER_FILE "${GENERATED_HEADERS_DIR}/data-wallet-unsigned-tx-json.hpp")
set(UNSIGNED_TX_JSON_MACROS "")
foreach(UNSIGNED_TX_JSON_FILE ${UNSIGNED_TX_JSON_FILES})
	get_filename_component(HASH_ID ${UNSIGNED_TX_JSON_FILE} NAME_WE)
	set(MACRO_NAME "WALLET_${HASH_ID}_UNSIGNED_TX_JSON")
	list(APPEND UNSIGNED_TX_JSON_MACROS "${UNSIGNED_TX_JSON_FILE}:${MACRO_NAME}")
endforeach()
if(UNSIGNED_TX_JSON_MACROS)
	add_custom_command(
		OUTPUT ${UNSIGNED_TX_JSON_HEADER_FILE}
		COMMAND ${CMAKE_BINARY_DIR}/tools/binheadermacro/bin2hppmacro ${UNSIGNED_TX_JSON_HEADER_FILE} ${UNSIGNED_TX_JSON_MACROS}
		DEPENDS ${UNSIGNED_TX_JSON_FILES} tools/binheadermacro/bin2hppmacro
		COMMENT "Generating header macros for ${UNSIGNED_TX_JSON_HEADER_FILE}"
		VERBATIM
	)
	list(APPEND GENERATED_HEADERS ${UNSIGNED_TX_JSON_HEADER_FILE})
endif()
# common
add_custom_target(generate_wallet_headers ALL DEPENDS ${GENERATED_HEADERS})
include_directories(${GENERATED_HEADERS_DIR})

add_library(rapidjson INTERFACE)

target_include_directories(rapidjson
	INTERFACE
	${CMAKE_CURRENT_SOURCE_DIR}/../../external/rapidjson/include
)

add_library(
	json-tx-description
	${LIB_TYPE}
	${CMAKE_CURRENT_SOURCE_DIR}/json-tx-description.cpp
)
target_include_directories(
	json-tx-description
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/../../external/rapidjson/include
)
target_link_libraries(
	json-tx-description
	PRIVATE
	monero-ots
	rapidjson
)


# C++ test executable, to check the OTS C++ library
add_executable(gtest_ots.cpp
	gtest_ots.cpp
	# json-tx-description.cpp
	test-tx-description.cpp
)

add_dependencies(gtest_ots.cpp
	generate_wallet_headers
)

target_link_libraries(gtest_ots.cpp
	PRIVATE
	utf8proc
	monero-ots
	rapidjson
	json-tx-description
	gtest
	gtest_main
	${CMAKE_DL_LIBS}
)

# Include Monero's GTest directories
target_include_directories(gtest_ots.cpp PRIVATE
	${GTEST_ROOT}/include
	${GENERATED_HEADERS_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/../../external/rapidjson/include
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
	set(MONERO_MAINNET_TIMESTAMPS "${CMAKE_BINARY_DIR}/tests/mainnet.timestamps")
	set(MONERO_TESTNET_TIMESTAMPS "${CMAKE_BINARY_DIR}/tests/testnet.timestamps")
	set(MONERO_STAGENET_TIMESTAMPS "${CMAKE_BINARY_DIR}/tests/stagenet.timestamps")
	if(NOT EXISTS ${MONERO_MAINNET_TIMESTAMPS})
		if(NOT MONERO_MAINNET_BLOCKS OR NOT EXISTS ${MONERO_MAINNET_BLOCKS})
			message(FATAL_ERROR "Blocktime tests require mainnet.timestamps file")
		else()
			add_custom_command(
				OUTPUT ${MONERO_MAINNET_TIMESTAMPS}
				COMMAND ${CMAKE_BINARY_DIR}/tools/blocktime/blocktime_extractor
				${MONERO_MAINNET_BLOCKS}
				${MONERO_MAINNET_TIMESTAMPS}
				DEPENDS ${MONERO_MAINNET_BLOCKS} tools/blocktime/blocktime_extractor
				COMMENT "Extracting timestamps from mainnet blocks"
				VERBATIM
			)
		endif()
	endif()
	if(NOT EXISTS ${MONERO_TESTNET_TIMESTAMPS})
		if(NOT MONERO_TESTNET_BLOCKS OR NOT EXISTS ${MONERO_TESTNET_BLOCKS})
			message(FATAL_ERROR "Blocktime tests require testnet.timestamps file")
		else()
			add_custom_command(
				OUTPUT ${MONERO_TESTNET_TIMESTAMPS}
				COMMAND ${CMAKE_BINARY_DIR}/tools/blocktime/blocktime_extractor
				${MONERO_TESTNET_BLOCKS}
				${MONERO_TESTNET_TIMESTAMPS}
				DEPENDS ${MONERO_TESTNET_BLOCKS} tools/blocktime/blocktime_extractor
				COMMENT "Extracting timestamps from testnet blocks"
				VERBATIM
			)
		endif()
	endif()
	if(NOT EXISTS ${MONERO_STAGENET_TIMESTAMPS})
		if(NOT MONERO_STAGENET_BLOCKS OR NOT EXISTS ${MONERO_STAGENET_BLOCKS})
			message(FATAL_ERROR "Blocktime tests require stagenet.timestamps file")
		else()
			add_custom_command(
				OUTPUT ${MONERO_STAGENET_TIMESTAMPS}
				COMMAND ${CMAKE_BINARY_DIR}/tools/blocktime/blocktime_extractor
				${MONERO_STAGENET_BLOCKS}
				${MONERO_STAGENET_TIMESTAMPS}
				DEPENDS ${MONERO_STAGENET_BLOCKS} tools/blocktime/blocktime_extractor
				COMMENT "Extracting timestamps from stagenet blocks"
				VERBATIM
			)
		endif()
	endif()
	add_custom_target(generate_blocktime_timestamps ALL
		DEPENDS
		${MONERO_MAINNET_TIMESTAMPS}
		${MONERO_TESTNET_TIMESTAMPS}
		${MONERO_STAGENET_TIMESTAMPS}
	)
	# Add the blocktime test executable
	add_executable(gtest_blocktime gtest_blocktime.cpp)

	add_dependencies(gtest_blocktime generate_blocktime_timestamps)
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
	# Register the test
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

add_dependencies(check_ots_c generate_wallet_headers)

target_include_directories(check_ots_c
	PRIVATE
	${CMAKE_BINARY_DIR}
	${CHECK_INCLUDE_DIRS}
	${GENERATED_HEADERS_DIR}
)

target_link_libraries(check_ots_c
	PRIVATE
	check
	monero-ots
	json-tx-description
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
