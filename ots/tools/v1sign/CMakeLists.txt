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

# V1 Sign tool
if(BUILD_V1SIGN)
	add_executable(v1sign
		src/v1sign.cpp
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
