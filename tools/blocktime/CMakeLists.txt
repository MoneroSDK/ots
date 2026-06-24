set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Require Monero source directory
if(NOT MONERO_SOURCE_DIR)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/crypto/crypto.cpp")
        get_filename_component(MONERO_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../.." ABSOLUTE)
        get_filename_component(EPEE_INCLUDE_DIR_BASE "${MONERO_SOURCE_DIR}/contrib/epee/include" ABSOLUTE)
        get_filename_component(EASYLOGGING_SOURCE_DIR "${MONERO_SOURCE_DIR}/external/easylogging++" ABSOLUTE)
    else()
        message(FATAL_ERROR "Please specify MONERO_SOURCE_DIR pointing to Monero source")
    endif()
endif()

# LMDB setup
set(LMDB_SOURCES
    ${MONERO_SOURCE_DIR}/external/db_drivers/liblmdb/mdb.c
    ${MONERO_SOURCE_DIR}/external/db_drivers/liblmdb/midl.c
)
add_library(lmdb STATIC ${LMDB_SOURCES})
target_include_directories(lmdb PUBLIC
    ${MONERO_SOURCE_DIR}/external/db_drivers/liblmdb
)

# Blocktime static library
add_library(blocktime STATIC
    src/blocktime.cpp
)

target_include_directories(blocktime
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
    ${MONERO_SOURCE_DIR}/src
    # ${EPEE_INCLUDE_DIR_BASE}
    ${MONERO_SOURCE_DIR}/external/db_drivers/liblmdb
)

if(BUILD_BLOCKTIME)
    # Blocktime extractor executable
    add_executable(blocktime_extractor
        src/blocktime-extractor.cpp
    )

    target_include_directories(blocktime_extractor
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        PRIVATE
        ${MONERO_SOURCE_DIR}/src
        # ${EPEE_INCLUDE_DIR_BASE}
        ${MONERO_SOURCE_DIR}/external/db_drivers/liblmdb
    )

    target_link_libraries(blocktime_extractor
        PRIVATE
        blocktime
        lmdb
        -static
    )

    # Blocktime CLI executable
    add_executable(blocktime_cli
        src/blocktime-cli.cpp
    )

    target_link_libraries(blocktime_cli
        PRIVATE
        blocktime
        -static
    )

    # Install rules
    install(
        TARGETS blocktime blocktime_extractor blocktime_cli
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
    )
else()
    # Install rules
    install(
        TARGETS blocktime
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
    )
endif()

install(
    FILES include/blocktime.h
    DESTINATION include/monero-ots/blocktime
)
