# List of all example programs
set(EXAMPLE_PROGRAMS
    example
    feed_test
    charset_example
    codepage_table_example
    text_formatting_example
    barcode_example
    comprehensive_barcode_example
    tab_example
    raster_image_example
    png_raster_example
    network_example
    self_test_example
    dev_ticket_example
)

# Function to create an example program
function(add_example_program target_name source_file)
    add_executable(${target_name} ${source_file})

    # Link with munbyn library
    target_link_libraries(${target_name} PRIVATE munbyn::munbyn)

    # Set include directories
    target_include_directories(${target_name} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src
    )

    # Platform-specific libraries
    if(WIN32)
        target_link_libraries(${target_name} PRIVATE ws2_32)
    elseif(NOT APPLE)
        target_link_libraries(${target_name} PRIVATE m)
    endif()

    # Compiler flags
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${target_name} PRIVATE -Wall -Wextra -pedantic)
    endif()

    # Set output directory
    set_target_properties(${target_name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/examples
        C_STANDARD 99
        C_STANDARD_REQUIRED ON
    )
endfunction()

# Create all example programs
add_example_program(example example.c)
add_example_program(feed_test feed_test.c)
add_example_program(charset_example charset_example.c)
add_example_program(codepage_table_example codepage_table_example.c)
add_example_program(text_formatting_example text_formatting_example.c)
add_example_program(barcode_example barcode_example.c)
add_example_program(comprehensive_barcode_example comprehensive_barcode_example.c)
add_example_program(tab_example tab_example.c)
add_example_program(raster_image_example raster_image_example.c)
add_example_program(png_raster_example png_raster_example.c)
add_example_program(network_example network_example.c)
add_example_program(self_test_example self_test_example.c)
add_example_program(dev_ticket_example dev_ticket_example.c)

# Copy stb_image.h to build directory for png_raster_example
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/stb_image.h
    ${CMAKE_BINARY_DIR}/examples/stb_image.h
    COPYONLY
)

# Copy image files to build directory
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/munbyn.png
    ${CMAKE_BINARY_DIR}/examples/munbyn.png
    COPYONLY
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/munbyn_1.png
    ${CMAKE_BINARY_DIR}/examples/munbyn_1.png
    COPYONLY
)

# Create a custom target to build all examples
add_custom_target(examples
    DEPENDS ${EXAMPLE_PROGRAMS}
    COMMENT "Building all example programs"
)

# Installation (optional - examples are typically not installed)
if(CMAKE_INSTALL_EXAMPLES)
    install(TARGETS ${EXAMPLE_PROGRAMS}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/examples
        COMPONENT Examples
    )

    install(FILES
        stb_image.h
        munbyn.png
        munbyn_1.png
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/munbyn/examples
        COMPONENT Examples
    )
endif()

# Print information about examples
message(STATUS "Configured ${CMAKE_CURRENT_LIST_DIR}: ${CMAKE_CURRENT_LIST_FILE}")
message(STATUS "Examples will be built to: ${CMAKE_BINARY_DIR}/examples/")
