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
option(BUILD_BLOCKTIME "Build blocktime analysis tools" OFF)
option(BUILD_BLOCKTIME_TESTS "Include blocktime in test suite" OFF)
option(BUILD_GEN_OTS_ERROR_HEADER "Build c_error_codes_generator tool" ON) # TODO: restructure, is always needed to build the C ABI library
option(BUILD_TESTS "Build test suite" OFF)
option(BUILD_BIN_HEADER_MACRO "Build tools to move binary files into header macros" OFF)
option(BUILD_DOCS "Build documentation" OFF)
option(BUILD_V1SIGN "Build v1sign tool" OFF)
option(RUN_TESTS "Run tests after build" OFF)
option(BUILD_SHARED_LIBS "Build shared library" OFF)  # Default to static

# Set library type based on BUILD_SHARED_LIBS
if(BUILD_SHARED_LIBS)
	set(LIB_TYPE SHARED)
	set(CMAKE_ASM_CREATE_SHARED_LIBRARY ON)
	set(SWITCH_STATIC "")
	# When building shared libraries, ensure PIC is enabled globally
	set(CMAKE_POSITION_INDEPENDENT_CODE ON)
	# Adjust linker flags for shared libraries
	#string(REPLACE "-static" "" CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
	#string(REPLACE "-static" "" CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}")
	#set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-as-needed")
else()
	set(LIB_TYPE STATIC)
	set(SWITCH_STATIC "-static")
endif()

# TODO: should restructure all this helper tools into a separate folder
#       and make naming more consistent
# gen-ots-error-header tool
# TODO: should be restructured, is always needed to build the C ABI library
if(BUILD_GEN_OTS_ERROR_HEADER)
	add_executable(gen-ots-error-header
		gen-ots-error-header/src/gen-ots-error-header.cpp
	)
	target_link_libraries(gen-ots-error-header
		PRIVATE
		${CMAKE_THREAD_LIBS_INIT}
		-static
	)
	target_include_directories(gen-ots-error-header
		PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/include
	)
	install(TARGETS gen-ots-error-header
		RUNTIME DESTINATION bin
	)
endif()

# Monero dependencies

if(!BUILD_SHARED_LIBS)
	# TODO: need to be fixed
	# lot of warnings we suppress for now
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++ -Wl,--no-warn-search-mismatch")
endif()

# Static linking setup
# TODO: maybe in the end I will change my mind about all on static standalone library, so check again later
if(NOT BUILD_SHARED_LIBS)
	set(Boost_USE_STATIC_LIBS ON)

	set(OPENSSL_USE_STATIC_LIBS TRUE)
	set(OPENSSL_CRYPTO_LIBRARY "/usr/lib/x86_64-linux-gnu/libcrypto.a") # TODO: must not be a fixed path!
	set(OPENSSL_SSL_LIBRARY "/usr/lib/x86_64-linux-gnu/libssl.a") # TODO: must not be a fixed path!
else()
	set(Boost_USE_STATIC_LIBS OFF)
	set(OPENSSL_USE_STATIC_LIBS FALSE)
endif()

find_package(Boost REQUIRED COMPONENTS filesystem thread system chrono serialization)
find_package(OpenSSL REQUIRED)

# Require Monero source directory
if(NOT MONERO_SOURCE_DIR)
	if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../src/crypto/crypto.cpp")
		get_filename_component(MONERO_STUBS_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/stubs/include" ABSOLUTE)
		get_filename_component(MONERO_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)
		get_filename_component(MONERO_CRYPTO_SOURCE_DIR "${MONERO_SOURCE_DIR}/src/crypto" ABSOLUTE)
		get_filename_component(MONERO_COMMON_SOURCE_DIR "${MONERO_SOURCE_DIR}/src/common" ABSOLUTE)
		get_filename_component(MONERO_MNEMONICS_SOURCE_DIR "${MONERO_SOURCE_DIR}/src/mnemonics" ABSOLUTE)
		get_filename_component(MONERO_CRYPTONOTE_BASIC_SOURCE_DIR "${MONERO_SOURCE_DIR}/src/cryptonote_basic" ABSOLUTE)
		get_filename_component(MONERO_CRYPTONOTE_CORE_SOURCE_DIR "${MONERO_SOURCE_DIR}/src/cryptonote_core" ABSOLUTE)
		get_filename_component(MONERO_RINGCT_SOURCE_DIR "${MONERO_SOURCE_DIR}/src/ringct" ABSOLUTE)
		get_filename_component(EPEE_SOURCE_DIR "${MONERO_SOURCE_DIR}/contrib/epee/src" ABSOLUTE)
		get_filename_component(EPEE_INCLUDE_DIR_BASE "${MONERO_SOURCE_DIR}/contrib/epee/include" ABSOLUTE)
		get_filename_component(BOOST_DIR "${MONERO_SOURCE_DIR}/external" ABSOLUTE)
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

if(BUILD_BLOCKTIME OR (BUILD_TESTS AND BUILD_BLOCKTIME_TESTS))
	# LMDB setup
	set(LMDB_SOURCES
		${MONERO_SOURCE_DIR}/external/db_drivers/liblmdb/mdb.c
		${MONERO_SOURCE_DIR}/external/db_drivers/liblmdb/midl.c
	)
	add_library(lmdb STATIC ${LMDB_SOURCES})
	target_include_directories(lmdb PUBLIC
		${MONERO_SOURCE_DIR}/external/db_drivers/liblmdb
	)
endif()

if(BUILD_BLOCKTIME OR (BUILD_TESTS AND BUILD_BLOCKTIME_TESTS))
	add_subdirectory(blocktime)
endif()

# Version configuration
configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/src/version.h.in # maybe that should also be changed to ots-version.h.in TODO: check later
	${CMAKE_CURRENT_BINARY_DIR}/include/ots-version.h # changed version.h to ots-version.h to avoid conflicts with monero
)

# generate ${CMAKE_BINARY_DIR}/include/ots-errors.h
set(C_ERROR_HEADER_FILE "${CMAKE_BINARY_DIR}/include/ots-errors.h")
add_custom_command(
	OUTPUT ${C_ERROR_HEADER_FILE}
	COMMAND ${CMAKE_BINARY_DIR}/gen-ots-error-header -f ${C_ERROR_HEADER_FILE}
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/include/ots-exceptions.hpp gen-ots-error-header
	COMMENT "Generating header macros for ${C_ERROR_HEADER_FILE}"
	VERBATIM
)
add_custom_target(generate_ots_error_header ALL DEPENDS ${C_ERROR_HEADER_FILE})

# Create a stubs library for now, hate this cluster coitus, we sort this out later
add_library(monero-stubs STATIC
	stubs/src/unbound.cpp
	stubs/src/dns_utils.cpp
)

target_include_directories(monero-stubs
	PRIVATE
	${MONERO_SOURCE_DIR}/src
	${EPEE_INCLUDE_DIR_BASE}
	${EASYLOGGING_INCLUDE_DIR_BASE}
)

set_target_properties(monero-stubs PROPERTIES POSITION_INDEPENDENT_CODE ON)

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

set_target_properties(easylogging PROPERTIES POSITION_INDEPENDENT_CODE ON)

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
	PRIVATE
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
)

set_target_properties(epee PROPERTIES POSITION_INDEPENDENT_CODE ON)

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

set_target_properties(cryptonight_asm PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_library(monero-crypto STATIC
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
	${MONERO_CRYPTO_SOURCE_DIR}/CryptonightR_JIT.c
	${MONERO_CRYPTO_SOURCE_DIR}/CryptonightR_template.h
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

set_target_properties(monero-crypto PROPERTIES POSITION_INDEPENDENT_CODE ON)

# we need the MONERO_VERSION from the version.cpp
set(VERSION_IS_RELEASE true)
set(VERSIONTAG "OTS")
# TODO: should figure out how to use the current version how this seems to be a bit tricky to get. Like there is still 0.18.1.0 where the master is in reality 0.18.3.4
configure_file(
	${MONERO_SOURCE_DIR}/src/version.cpp.in
	${CMAKE_CURRENT_BINARY_DIR}/version.cpp
	@ONLY
)

set(MONERO_SOURCES
	${MONERO_MNEMONICS_SOURCE_DIR}/electrum-words.cpp
	${MONERO_COMMON_SOURCE_DIR}/util.cpp
	${MONERO_COMMON_SOURCE_DIR}/base58.cpp
	${MONERO_COMMON_SOURCE_DIR}/threadpool.cpp
	${MONERO_COMMON_SOURCE_DIR}/perf_timer.cpp
	${MONERO_COMMON_SOURCE_DIR}/aligned.c
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
	${MONERO_RINGCT_SOURCE_DIR}/bulletproofs_plus.cc
	${MONERO_RINGCT_SOURCE_DIR}/multiexp.cc
	${MONERO_SOURCE_DIR}/src/device/device.cpp
	${MONERO_SOURCE_DIR}/src/device/device_default.cpp
)

add_library(monero-minimal STATIC
	${MONERO_SOURCES}
)

# Library properties
set_target_properties(monero-minimal PROPERTIES
	C_VISIBILITY_PRESET hidden
	CXX_VISIBILITY_PRESET hidden
	VISIBILITY_INLINES_HIDDEN ON
	POSITION_INDEPENDENT_CODE ON
)

target_include_directories(monero-minimal
	PUBLIC
	${MONERO_SOURCE_DIR}/src
	${EPEE_INCLUDE_DIR_BASE}
	${EASYLOGGING_INCLUDE_DIR_BASE}
	${BOOST_DIR}
	PRIVATE
	${MONERO_STUBS_INCLUDE_DIR}
)

target_link_libraries(monero-minimal
	PUBLIC
	epee
	randomx
	monero-crypto
	monero-stubs
	easylogging
	Boost::filesystem
	Boost::thread
	Boost::system
	${OPENSSL_SSL_LIBRARY}
	${OPENSSL_CRYPTO_LIBRARY}
	${CMAKE_THREAD_LIBS_INIT}
	${Backtrace_LIBRARIES}
)

set_target_properties(monero-minimal PROPERTIES POSITION_INDEPENDENT_CODE ON)

# TODO: what to do about polyseed? I don't like it, but I also shouldn't rework the whole thing, but how it is it forces me also to ugly code duplication
# Polyseed library
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
	${POLYSEED_SOURCE_DIR}/polyseed.c
	${POLYSEED_SOURCE_DIR}/storage.c
)

# Create polyseed library
add_library(polyseed STATIC ${POLYSEED_SOURCES})
set_property(TARGET polyseed PROPERTY POSITION_INDEPENDENT_CODE ON)

set_target_properties(polyseed PROPERTIES
	POSITION_INDEPENDENT_CODE ON
	C_STANDARD 11
	C_STANDARD_REQUIRED ON
)

target_include_directories(polyseed
	PUBLIC
	${POLYSEED_INCLUDE_DIR_BASE}
	PRIVATE
	# ${POLYSEED_SOURCE_DIR}
)

target_compile_definitions(polyseed
	PRIVATE
	POLYSEED_STATIC
	_DEFAULT_SOURCE
	_BSD_SOURCE
	_POSIX_C_SOURCE=200809L
)

# Force C compilation for .c files
set_source_files_properties(${POLYSEED_SOURCES} PROPERTIES LANGUAGE C)

set_target_properties(polyseed PROPERTIES POSITION_INDEPENDENT_CODE ON)

# Create utf8proc library
set(UTF8PROC_INSTALL OFF)
set(UTF8PROC_ENABLE_TESTING OFF)
# set(BUILD_SHARED_LIBS OFF) # what did I thought? TODO: fix if needed, probably is needed for shared libs

add_library(utf8proc STATIC
	${CMAKE_CURRENT_SOURCE_DIR}/external/utf8proc/utf8proc.c
)

target_include_directories(utf8proc
	PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/external/utf8proc>
	$<INSTALL_INTERFACE:include>
)

target_compile_definitions(utf8proc
	PUBLIC
	UTF8PROC_STATIC
	PRIVATE
	UTF8PROC_EXPORTS
)

if (NOT MSVC)
	set_target_properties(
		utf8proc PROPERTIES
		COMPILE_FLAGS "-O2 -std=c99 -pedantic -Wall"
	)
endif()

set_target_properties(utf8proc PROPERTIES
	POSITION_INDEPENDENT_CODE ON
)

# Main library
add_library(monero-ots ${LIB_TYPE}
	${MONERO_SOURCE_DIR}/src/version.cpp
	src/ots.cpp
	src/wipeable-string.cpp
	src/seedindices.cpp
	src/address.cpp
	src/seed-language.cpp
	src/seed.cpp
	src/seed-legacy.cpp
	src/seed-monero.cpp
	src/seed-polyseed.cpp
	src/wallet.cpp
	src/key-store.cpp
	src/entropy.cpp
	src/polyseed-key-store.cpp
	src/account.cpp
	src/cryptonote-tx-utils.cpp
	src/c-abi/internal.cpp
	src/c-abi/common.cpp
	src/c-abi/ots.cpp
	src/c-abi/wipeable-string.cpp
	src/c-abi/seed-indices.cpp
	src/c-abi/seed-language.cpp
	src/c-abi/seed.cpp
	src/c-abi/seed-legacy.cpp
	src/c-abi/seed-monero.cpp
	src/c-abi/seed-polyseed.cpp
	src/c-abi/address.cpp
	src/c-abi/wallet.cpp
	src/c-abi/seed-jar.cpp
	src/c-abi/transaction.cpp
	${EASYLOGGING} # TODO: WTF! All this chingadera makes me desires to rewrite the whole thing :@ But need to calm my horses because it will probably take a eternity to get it done
)

target_link_libraries(monero-ots
	PRIVATE
	utf8proc
	polyseed
	monero-minimal
	Boost::serialization # TODO: needed at the moment for account.cpp, possibe to get rid of it?
	${SWITCH_STATIC}
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
	PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
	$<INSTALL_INTERFACE:include>
	PRIVATE
	${MONERO_SOURCE_DIR}/src
	${EPEE_INCLUDE_DIR_BASE}
	${EASYLOGGING_INCLUDE_DIR_BASE}
	${MONERO_SOURCE_DIR}/external/boost/archive
	${RANDOMX_INCLUDE_DIR_BASE}
	${CMAKE_THREAD_LIBS_INIT} # TODO: how came this in? Do we need it? And ifso, why and for what? Easylogging?
	${Backtrace_LIBRARIES} # TODO: how came this in? Easylogging?
	${POLYSEED_INCLUDE_DIR_BASE}
)

# Library properties
set_target_properties(monero-ots PROPERTIES
	VERSION ${PROJECT_VERSION}
	SOVERSION ${PROJECT_VERSION_MAJOR}
	C_VISIBILITY_PRESET default
	CXX_VISIBILITY_PRESET default
	VISIBILITY_INLINES_HIDDEN ON
	POSITION_INDEPENDENT_CODE ON
)

# Installation rules
install(
	TARGETS monero-ots
	utf8proc
	polyseed
	monero-stubs
	monero-minimal
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
	${CMAKE_CURRENT_BINARY_DIR}/include/ots-version.h
	${CMAKE_CURRENT_BINARY_DIR}/include/ots-errors.h
	DESTINATION include
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

# tools to move binary files into header macros
if (BUILD_TESTS OR BUILD_BIN_HEADER_MACRO)
	add_subdirectory(binheadermacro)
endif()

# Tests configuration
if(BUILD_TESTS)
	enable_testing()
	add_subdirectory(tests)
endif()

# Doxygen documentation
if(BUILD_DOCS)
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
endif()

# V1 Sign tool
if(BUILD_V1SIGN)
	add_executable(v1sign
		v1sign/src/v1sign.cpp
	)
	target_link_libraries(v1sign
		PRIVATE
		monero-minimal
		${CMAKE_THREAD_LIBS_INIT}
		${SWITCH_STATIC}
	)
	target_include_directories(v1sign
		PRIVATE
		${MONERO_SOURCE_DIR}/src
		${EPEE_INCLUDE_DIR_BASE}
		${EASYLOGGING_INCLUDE_DIR_BASE}
		${MONERO_SOURCE_DIR}/external/boost/archive
	)
	install(TARGETS v1sign
		RUNTIME DESTINATION bin
	)
endif()
