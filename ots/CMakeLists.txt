cmake_minimum_required(VERSION 3.10)
# Silence policy warnings from anthropic... TODO: I think it is fucked up, but need to see how to get rid of the dependencies like boost and OpenSSL to make the lib fully standalone
cmake_policy(SET CMP0148 OLD)  # For Python modules warning
cmake_policy(SET CMP0076 NEW)  # For target_sources relative paths
# cmake_policy(SET CMP0115 NEW)  # For source file extensions

# speed up turnarounds
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    message(STATUS "Using ccache: ${CCACHE_PROGRAM}")
endif()

# TODO: should think about a version strategy
project(monero-ots VERSION 0.1.0)

# TODO: probably remove, it's to test to build slow-hash without AES
# TODO: seems to compile, let for now, but check later again, what to remove and
#       how fix it better
add_definitions(-DCRYPTONOTE_NO_AES)
add_definitions(-DNO_AES)
add_definitions(-DFORCE_USE_HEAP=1)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DCRYPTONOTE_NO_AES -DNO_AES")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCRYPTONOTE_NO_AES -DNO_AES")
# without AES clusterfuck.

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Require C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Require C 11
set(C_STANDARD 11)
set(C_STANDARD_REQUIRED ON)

# Options
option(DEBUG "Build with debug symbols" OFF)
if(DEBUG)
	add_compile_definitions(DEBUG=1)
endif()
option(BUILD_TESTS "Build test suite" OFF)
option(BUILD_SHARED_LIBS "Build shared library" OFF)  # Default to static
# Monero dependencies

# lot of warnings we suppress for now
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++ -Wl,--no-warn-search-mismatch")

# Static linking setup
# TODO: maybe in the end I will change my mind about all on static standalone library, so check again later
set(Boost_USE_STATIC_LIBS ON)

set(OPENSSL_USE_STATIC_LIBS TRUE)
set(OPENSSL_CRYPTO_LIBRARY "/usr/lib/x86_64-linux-gnu/libcrypto.a")
set(OPENSSL_SSL_LIBRARY "/usr/lib/x86_64-linux-gnu/libssl.a")

find_package(Boost REQUIRED COMPONENTS filesystem thread system chrono)
find_package(OpenSSL REQUIRED)

# No Unbound, no extra dependencies

# End Monero dependencies

# Require Monero source directory
if(NOT MONERO_SOURCE_DIR)
	if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../src/crypto/crypto.cpp")
        get_filename_component(MONERO_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)
        get_filename_component(MONERO_CRYPTO_SOURCE_DIR "${MONERO_SOURCE_DIR}/src/crypto" ABSOLUTE)
		get_filename_component(MONERO_COMMON_SOURCE_DIR "${MONERO_SOURCE_DIR}/src/common" ABSOLUTE)
		get_filename_component(MONERO_MNEMONICS_SOURCE_DIR "${MONERO_SOURCE_DIR}/src/mnemonics" ABSOLUTE)
		get_filename_component(MONERO_CRYPTONOTE_BASIC_SOURCE_DIR "${MONERO_SOURCE_DIR}/src/cryptonote_basic" ABSOLUTE)
		get_filename_component(MONERO_CRYPTONOTE_CORE_SOURCE_DIR "${MONERO_SOURCE_DIR}/src/cryptonote_core" ABSOLUTE)
		get_filename_component(MONERO_RINGCT_SOURCE_DIR "${MONERO_SOURCE_DIR}/src/ringct" ABSOLUTE)
        get_filename_component(EPEE_SOURCE_DIR "${MONERO_SOURCE_DIR}/contrib/epee/src" ABSOLUTE)
        get_filename_component(EPEE_INCLUDE_DIR_BASE "${MONERO_SOURCE_DIR}/contrib/epee/include" ABSOLUTE)
        get_filename_component(RANDOMX_DIR "${MONERO_SOURCE_DIR}/external/randomx" ABSOLUTE)
        get_filename_component(RANDOMX_SOURCE_DIR "${MONERO_SOURCE_DIR}/external/randomx/src" ABSOLUTE)
        get_filename_component(RANDOMX_INCLUDE_DIR_BASE "${MONERO_SOURCE_DIR}/external/randomx/src" ABSOLUTE)
		get_filename_component(EASYLOGGING_SOURCE_DIR "${MONERO_SOURCE_DIR}/external/easylogging++" ABSOLUTE)
		set(EASYLOGGING_INCLUDE_DIR_BASE "${EASYLOGGING_SOURCE_DIR}")
		get_filename_component(POLYSEED_SOURCE_DIR "${MONERO_SOURCE_DIR}/external/polyseed/src" ABSOLUTE)
		get_filename_component(POLYSEED_INCLUDE_DIR_BASE "${MONERO_SOURCE_DIR}/external/polyseed/include" ABSOLUTE)
	else()
		message(FATAL_ERROR "Please specify MONERO_SOURCE_DIR pointing to Monero source")
	endif()
endif()

# test byte order
# add_executable(test_byte_order test_byte_order.cpp)
# target_include_directories(test_byte_order PRIVATE ${EPEE_INCLUDE_DIR_BASE})
# target_compile_definitions(test_byte_order PRIVATE
# )

# Version configuration
configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/src/version.h.in # maybe that should also be changed to ots-version.h.in TODO: check later
	${CMAKE_CURRENT_BINARY_DIR}/include/ots-version.h # changed version.h to ots-version.h to avoid conflicts with monero
)

# Create a stubs library for now, hate this cluster coitus, we sort this out later
add_library(monero-stubs STATIC
	src/stub/unbound.cpp
	# src/stub/logging.cpp
	# src/stub/div128.c is in epee int-util.cpp
	# src/stub/epee.cpp makes no sense because we seem still to need epee, let's check later again
	## src/stub/cryptonote.cpp # check later again if this makes sense, I made it to compile without dragging even more dependencies in
	src/stub/dns_utils.cpp
	# src/stub/hw_device.cpp
)

target_include_directories(monero-stubs
	PRIVATE
	${MONERO_SOURCE_DIR}/src
	${EPEE_INCLUDE_DIR_BASE}
	${EASYLOGGING_INCLUDE_DIR_BASE}
)

# EASYLOGGING dependencies
find_package(Threads)
find_package(Backtrace)

set(EASYLOGGING
	${MONERO_SOURCE_DIR}/external/easylogging++/easylogging++.cc
)

add_library(easylogging STATIC
    ${EASYLOGGING}
)

target_include_directories(easylogging
    PUBLIC
    ${EASYLOGGING_INCLUDE_DIR_BASE}
)

target_compile_definitions(easylogging
	PRIVATE
	# disable easylogging complete, another cluster coitus
	ELPP_DISABLE_LOGS
	ELPP_NO_DEFAULT_LOG_FILE
	ELPP_NO_LOG_TO_FILE
)

# EPEE
# add_definitions(-DSTATICLIB) TODO: is that really needed how we put -static in the linker flags?
# TODO: this will drag more and more dependencies inside
#	check how to include boost into the lib
#	check how to get rid of easylogging
#	also check how to get rid of storage, net in epee itself
set(EPEE_SOURCES
	${EPEE_SOURCE_DIR}/mlocker.cpp
	${EPEE_SOURCE_DIR}/hex.cpp
	${EPEE_SOURCE_DIR}/int-util.cpp
	${EPEE_SOURCE_DIR}/string_tools.cpp
	${EPEE_SOURCE_DIR}/misc_language.cpp
	${EPEE_SOURCE_DIR}/file_io_utils.cpp
	${EPEE_SOURCE_DIR}/memwipe.c
	${EPEE_SOURCE_DIR}/wipeable_string.cpp
	${EPEE_SOURCE_DIR}/net_utils_base.cpp
	${EPEE_SOURCE_DIR}/http_auth.cpp 
	${EPEE_SOURCE_DIR}/net_utils_base.cpp
	${EPEE_SOURCE_DIR}/net_parse_helpers.cpp
	${EPEE_SOURCE_DIR}/mlog.cpp
)
if (MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-D_WIN32_WINNT=0x0600)
else()
    add_definitions(-DGIT_ARCHIVE)
endif()

if (APPLE)
    add_definitions(-DPER_BLOCK_CHECKPOINT)
endif()
add_library(epee STATIC
	${EPEE_SOURCES}
)
target_include_directories(epee
	PUBLIC
	"${EPEE_INCLUDE_DIR_BASE}"
	"${EASYLOGGING_INCLUDE_DIR_BASE}"
)

target_compile_definitions(epee
	PRIVATE
	STATICLIB
	ELPP_DISABLE_LOGS
	ELPP_NO_DEFAULT_LOG_FILE
	ELPP_NO_LOG_TO_FILE
	AUTO_INITIALIZE_EASYLOGGINGPP
    _GNU_SOURCE
    __BYTE_ORDER=__LITTLE_ENDIAN
    __LITTLE_ENDIAN=1234
    __BIG_ENDIAN=4321
)
target_link_libraries(epee
	PUBLIC
	easylogging
	Boost::chrono
	Boost::filesystem
	Boost::thread
	Boost::system
	${OPENSSL_SSL_LIBRARY}
	${OPENSSL_CRYPTO_LIBRARY}
	-static
)

set_property(SOURCE ${EPEE_SOURCE_DIR}/memwipe.c PROPERTY C_STANDARD 11)

set(RANDOMX_JIT ON CACHE BOOL "Enable RandomX JIT compilation" FORCE)
set(RANDOMX_LARGE_PAGES OFF CACHE BOOL "Enable RandomX large pages support" FORCE)
add_subdirectory(${RANDOMX_DIR} ${CMAKE_BINARY_DIR}/randomx)

enable_language(ASM)
add_library(cryptonight_asm STATIC
    ${MONERO_CRYPTO_SOURCE_DIR}/CryptonightR_template.S
)
target_include_directories(cryptonight_asm PRIVATE
    ${MONERO_SOURCE_DIR}/src
)

add_library(monero-crypto STATIC
	# ${MONERO_SOURCE_DIR}/src/crypto/crypto.cpp
	# ${MONERO_SOURCE_DIR}/src/crypto/hash.c
	# ${MONERO_SOURCE_DIR}/src/crypto/random.c
	# ${MONERO_SOURCE_DIR}/src/crypto/slow-hash.c
	# ${MONERO_SOURCE_DIR}/src/crypto/hash-extra-blake.c
	# ${MONERO_SOURCE_DIR}/src/crypto/rx-slow-hash.c
	# ${MONERO_SOURCE_DIR}/src/crypto/blake256.c
	# ${MONERO_SOURCE_DIR}/src/crypto/blake2b.c
	# ${MONERO_SOURCE_DIR}/src/crypto/groestl.c
	# ${MONERO_SOURCE_DIR}/src/crypto/jh.c
	# ${MONERO_SOURCE_DIR}/src/crypto/skein.c
	# ${MONERO_SOURCE_DIR}/src/crypto/chacha.c
	# ${MONERO_SOURCE_DIR}/src/crypto/oaes_lib.c
	# ${MONERO_SOURCE_DIR}/src/crypto/crypto-ops.c
	# ${MONERO_SOURCE_DIR}/src/crypto/keccak.c
	# ${MONERO_SOURCE_DIR}/src/crypto/crypto-ops-data.c
	${CMAKE_CURRENT_BINARY_DIR}/version.cpp
	${MONERO_CRYPTO_SOURCE_DIR}/crypto.cpp
	${MONERO_CRYPTO_SOURCE_DIR}/hash.c
	${MONERO_CRYPTO_SOURCE_DIR}/random.c
	${MONERO_CRYPTO_SOURCE_DIR}/slow-hash.c
	${MONERO_CRYPTO_SOURCE_DIR}/hash-extra-blake.c
    ${MONERO_CRYPTO_SOURCE_DIR}/hash-extra-groestl.c
    ${MONERO_CRYPTO_SOURCE_DIR}/hash-extra-jh.c
    ${MONERO_CRYPTO_SOURCE_DIR}/hash-extra-skein.c
    ${MONERO_CRYPTO_SOURCE_DIR}/aesb.c
    ${MONERO_CRYPTO_SOURCE_DIR}/tree-hash.c
    ${MONERO_CRYPTO_SOURCE_DIR}/CryptonightR_JIT.c
	${MONERO_CRYPTO_SOURCE_DIR}/rx-slow-hash.c
	${MONERO_CRYPTO_SOURCE_DIR}/blake256.c
	${MONERO_CRYPTO_SOURCE_DIR}/blake2b.c
	${MONERO_CRYPTO_SOURCE_DIR}/groestl.c
	${MONERO_CRYPTO_SOURCE_DIR}/jh.c
	${MONERO_CRYPTO_SOURCE_DIR}/skein.c
	${MONERO_CRYPTO_SOURCE_DIR}/chacha.c
	${MONERO_CRYPTO_SOURCE_DIR}/oaes_lib.c
	${MONERO_CRYPTO_SOURCE_DIR}/crypto-ops.c
	${MONERO_CRYPTO_SOURCE_DIR}/keccak.c
	${MONERO_CRYPTO_SOURCE_DIR}/crypto-ops-data.c
	${MONERO_CRYPTO_SOURCE_DIR}/crypto_ops_builder/verify.c
	# ${MONERO_CRYPTO_SOURCE_DIR}/crypto_ops_builder/ref10CommentedCombined/ge.c
    ${MONERO_CRYPTO_SOURCE_DIR}/CryptonightR_JIT.c
    ${MONERO_CRYPTO_SOURCE_DIR}/CryptonightR_template.h
)

# Disable AES in slow-hash, TODO: figure out how to make the best for the target platform
set_source_files_properties(
	${MONERO_CRYPTO_SOURCE_DIR}/slow-hash.c
	PROPERTIES COMPILE_DEFINITIONS CRYPTONOTE_NO_AES
)

target_include_directories(monero-crypto
    PUBLIC
    ${MONERO_SOURCE_DIR}/src
    ${EPEE_INCLUDE_DIR_BASE}
    PRIVATE
	${RANDOMX_INCLUDE_DIR_BASE}
    ${EASYLOGGING_INCLUDE_DIR_BASE}
)

target_compile_definitions(monero-crypto
    PRIVATE
    _GNU_SOURCE
    __BYTE_ORDER=__LITTLE_ENDIAN
    __LITTLE_ENDIAN=1234
    __BIG_ENDIAN=4321
    CRYPTONOTE_NO_AES
    NO_AES
	USE_CRYPTONIGHT_JIT
)

target_link_libraries(monero-crypto
    PRIVATE
	cryptonight_asm
    randomx
    epee
    easylogging
	${OPENSSL_SSL_LIBRARY}
	${OPENSSL_CRYPTO_LIBRARY} # same as above
)

# we need the MONERO_VERSION from the version.cpp
set(VERSION_IS_RELEASE true)
set(VERSIONTAG "OTS")
# TODO: should figure out how to use the current version how this seems to be a bit trickyto get. Like there is still 0.18.1.0 where the master is in reality 0.18.3.4
configure_file(
	${MONERO_SOURCE_DIR}/src/version.cpp.in
	${CMAKE_CURRENT_BINARY_DIR}/version.cpp
	@ONLY
)

set(MONERO_SOURCES
	${MONERO_MNEMONICS_SOURCE_DIR}/electrum-words.cpp
	${MONERO_COMMON_SOURCE_DIR}/util.cpp
	# ${MONERO_COMMON_SOURCE_DIR}/dns_utils.cpp # use only a stub, we don't ant to drag more and more dependencies in
	${MONERO_COMMON_SOURCE_DIR}/base58.cpp
	${MONERO_CRYPTONOTE_CORE_SOURCE_DIR}/cryptonote_core.cpp
	${MONERO_CRYPTONOTE_BASIC_SOURCE_DIR}/account.cpp
	${MONERO_CRYPTONOTE_BASIC_SOURCE_DIR}/cryptonote_basic_impl.cpp
	${MONERO_CRYPTONOTE_BASIC_SOURCE_DIR}/cryptonote_format_utils_basic.cpp
	${MONERO_CRYPTONOTE_BASIC_SOURCE_DIR}/cryptonote_format_utils.cpp
	${MONERO_RINGCT_SOURCE_DIR}/rctOps.cpp
	${MONERO_RINGCT_SOURCE_DIR}/rctTypes.cpp
	${MONERO_RINGCT_SOURCE_DIR}/rctCryptoOps.c
	${MONERO_RINGCT_SOURCE_DIR}/rctSigs.cpp
	${MONERO_RINGCT_SOURCE_DIR}/bulletproofs.cc
	# ${MONERO_RINGCT_SOURCE_DIR}/bulletproof_plus.cc
	${MONERO_RINGCT_SOURCE_DIR}/multiexp.cc
	${MONERO_SOURCE_DIR}/src/device/device.cpp
	${MONERO_SOURCE_DIR}/src/device/device_default.cpp
)

# TODO: what to do about polyseed? I don't like it, but I also shouldn't rework the whole thing, but how it is it forces me also to ugly code duplication
set(POLYSEED_SOURCES
	${POLYSEED_SOURCE_DIR}/dependency.c
	${POLYSEED_SOURCE_DIR}/features.c
	${POLYSEED_SOURCE_DIR}/gf.c
	${POLYSEED_SOURCE_DIR}/lang.c
	${POLYSEED_SOURCE_DIR}/lang_cs.c
	${POLYSEED_SOURCE_DIR}/lang_en.c
	${POLYSEED_SOURCE_DIR}/lang_es.c
	${POLYSEED_SOURCE_DIR}/lang_fr.c
	${POLYSEED_SOURCE_DIR}/lang_it.c
	${POLYSEED_SOURCE_DIR}/lang_jp.c
	${POLYSEED_SOURCE_DIR}/lang_ko.c
	${POLYSEED_SOURCE_DIR}/lang_pt.c
	${POLYSEED_SOURCE_DIR}/lang_zh_s.c
	${POLYSEED_SOURCE_DIR}/lang_zh_t.c
	${POLYSEED_SOURCE_DIR}/storage.c
	${POLYSEED_SOURCE_DIR}/polyseed.c
)

# Main library
add_library(monero-ots
	${MONERO_SOURCE_DIR}/src/version.cpp
	src/ots.cpp
	src/ots-abi.cpp
	src/wipeable-string.cpp
	src/wipeable-vector.cpp
	src/address.cpp
	src/seed-jar.cpp
	src/seed-language.cpp
	src/seed.cpp
	src/seed-legacy.cpp
	src/seed-monero.cpp
	src/seed-polyseed.cpp
	src/wallet.cpp
	src/key-store.cpp
	src/account.cpp
	# ${EPEE_SOURCES} # TODO: solve later with implementation, first finish ABI
	${EASYLOGGING} # WTF! All this chingadera makes me desires to rewrite the whole thing :@ But need to calm my horses because it will probably take a eternity to get it done
	${MONERO_SOURCES} # TODO: solve later with implementation, first finish ABI
	${POLYSEED_SOURCES}
)

target_link_libraries(monero-ots
	PRIVATE
	monero-stubs # use stubs as module to get easier rid of later
	epee
	randomx
	monero-crypto
	Boost::filesystem
	Boost::thread
	Boost::system
	# OpenSSL::SSL
	# OpenSSL::Crypto
	${OPENSSL_SSL_LIBRARY} # TODO: cluster coitus and makumba, need to get rid of this
	${OPENSSL_CRYPTO_LIBRARY} # same as above
	-static
)

# Add version definitions
target_compile_definitions(monero-ots
	PRIVATE
	OTS_VERSION="${PROJECT_VERSION}"
	# disable easylogging complete, another cluster coitus
	ELPP_DISABLE_LOGS
	ELPP_NO_DEFAULT_LOG_FILE
	ELPP_NO_LOG_TO_FILE
	CRYPTONOTE_NO_AES
)

# Include directories
target_include_directories(monero-ots
	PRIVATE
	# ${MONERO_SOURCE_DIR}/external/randomx/src
	# ${MONERO_SOURCE_DIR}/external/randomx/src/blake2
	PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
	$<INSTALL_INTERFACE:include>
	PRIVATE
	${MONERO_SOURCE_DIR}/src
	# ${MONERO_SOURCE_DIR}/contrib/epee/include
	${EPEE_INCLUDE_DIR_BASE}
	${EASYLOGGING_INCLUDE_DIR_BASE}
	# ${MONERO_SOURCE_DIR}/external/easylogging++
	${MONERO_SOURCE_DIR}/external/boost/archive
	# ${MONERO_SOURCE_DIR}/external/polyseed/include
	${POLYSEED_INCLUDE_DIR_BASE}
	${RANDOMX_INCLUDE_DIR_BASE}
	${CMAKE_THREAD_LIBS_INIT} # TODO: how came this in? Do we need it? And ifso, why and for what? Easylogging?
	${Backtrace_LIBRARIES} # TODO: how came this in? Easylogging?
)

# Library properties
set_target_properties(monero-ots PROPERTIES
	VERSION ${PROJECT_VERSION}
	SOVERSION ${PROJECT_VERSION_MAJOR}
	C_VISIBILITY_PRESET hidden
	CXX_VISIBILITY_PRESET hidden
	VISIBILITY_INLINES_HIDDEN ON
	POSITION_INDEPENDENT_CODE ON
	LINK_FLAGS "$<$<NOT:$<BOOL:${DEBUG}>>:-s>" # do not strip in debug mode
)

# Tests configuration
if(BUILD_TESTS)
	enable_testing()
	add_subdirectory(tests)
endif()

# Installation rules
install(
	TARGETS monero-ots
	monero-stubs
	epee
	easylogging
	randomx
	cryptonight_asm
	monero-crypto
	EXPORT monero-ots-targets
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	RUNTIME DESTINATION bin
	INCLUDES DESTINATION include
)

install(FILES
	include/ots.h
	include/ots.hpp
	${CMAKE_CURRENT_BINARY_DIR}/include/version.h
	DESTINATION include/monero-ots
)

install(EXPORT monero-ots-targets
	FILE monero-ots-targets.cmake
	NAMESPACE MoneroOTS::
	DESTINATION lib/cmake/monero-ots
)

# Generate and install package config files
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
	${CMAKE_CURRENT_BINARY_DIR}/monero-ots-config-version.cmake
	VERSION ${PROJECT_VERSION}
	COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
	${CMAKE_CURRENT_SOURCE_DIR}/cmake/monero-ots-config.cmake.in
	${CMAKE_CURRENT_BINARY_DIR}/monero-ots-config.cmake
	INSTALL_DESTINATION lib/cmake/monero-ots
)

install(FILES
	${CMAKE_CURRENT_BINARY_DIR}/monero-ots-config.cmake
	${CMAKE_CURRENT_BINARY_DIR}/monero-ots-config-version.cmake
	DESTINATION lib/cmake/monero-ots
)

# Doxygen documentation
find_package(Doxygen)
if(DOXYGEN_FOUND)
	# Set the Doxyfile input and output directories
	set(DOXYGEN_INPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/manual")
	set(DOXYGEN_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/docs")
	set(DOXYGEN_CCS "${CMAKE_CURRENT_SOURCE_DIR}/manual/doxygen-awesome.css ${CMAKE_CURRENT_SOURCE_DIR}/manual/doxygen-awesome-sidebar-only.css")
	set(DOXYGEN_MD_MAINPAGE "${CMAKE_CURRENT_SOURCE_DIR}/manual/index.md")

	# Configure the Doxyfile
	configure_file(
		${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
		${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
		@ONLY
	)

	# Add custom target for documentation
	add_custom_target(docs
		COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "Generating API documentation with Doxygen"
		VERBATIM
	)
endif()
