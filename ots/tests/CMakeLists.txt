# Use Monero's GTest from parent directory
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

# Use local Check framework
set(CHECK_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/check)
set(CHECK_INCLUDE_DIRS ${CHECK_ROOT}/src ${CHECK_ROOT}/../include)

# Add Check as a subdirectory
add_subdirectory(${CHECK_ROOT} check)

# C test executable, to check the OTS C ABI library
add_executable(check_ots_c
	check_ots.c
	data.cpp
)

target_link_libraries(check_ots_c
	PRIVATE
        monero-ots
        check
        pthread
	stdc++
)

# Include Check directories for C tests
target_include_directories(check_ots_c PRIVATE 
    ${CHECK_INCLUDE_DIRS}
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
