add_executable(gen-ots-error-header
    src/gen-ots-error-header.cpp
)
target_link_libraries(gen-ots-error-header
    PRIVATE
    ${CMAKE_THREAD_LIBS_INIT}
)
target_include_directories(gen-ots-error-header
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)
# Only install if explicitly requested
option(INSTALL_GEN_TOOL "Install the error header generator tool" OFF)
if(INSTALL_GEN_TOOL)
    install(TARGETS gen-ots-error-header
        RUNTIME DESTINATION bin
    )
endif()

# generate ${CMAKE_BINARY_DIR}/include/ots-errors.h
#set(C_ERROR_HEADER_FILE "${CMAKE_BINARY_DIR}/include/ots-errors.h")
#add_custom_command(
#	OUTPUT ${C_ERROR_HEADER_FILE}
#	COMMAND ${CMAKE_BINARY_DIR}/gen-ots-error-header -f ${C_ERROR_HEADER_FILE}
#	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/include/ots-exceptions.hpp gen-ots-error-header
#	COMMENT "Generating header macros for ${C_ERROR_HEADER_FILE}"
#	VERBATIM
#)
#add_custom_target(generate_ots_error_header ALL DEPENDS ${C_ERROR_HEADER_FILE})
