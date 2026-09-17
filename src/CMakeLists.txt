# Source files for the munbyn library
set(MUNBYN_SOURCES
    munbyn_printer.c
)

set(MUNBYN_HEADERS
    munbyn_printer.h
)

# Create the munbyn library
add_library(munbyn ${MUNBYN_SOURCES})

# Set library properties
set_target_properties(munbyn PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    PUBLIC_HEADER "${MUNBYN_HEADERS}"
    C_STANDARD 99
    C_STANDARD_REQUIRED ON
)

# Platform-specific libraries
if(WIN32)
    target_link_libraries(munbyn PRIVATE ws2_32)
elseif(APPLE)
    # No additional libraries needed on macOS
else()
    # Linux and other Unix-like systems
    target_link_libraries(munbyn PRIVATE m)
endif()

# Compiler-specific flags
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(munbyn PRIVATE -Wall -Wextra -pedantic)
    if(BUILD_SHARED_LIBS)
        target_compile_options(munbyn PRIVATE -fPIC)
    endif()
endif()

# Include directories
target_include_directories(munbyn
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Create an alias target for consistency with find_package
add_library(munbyn::munbyn ALIAS munbyn)

# Installation
install(TARGETS munbyn
    EXPORT munbynTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT Runtime
        NAMELINK_COMPONENT Development
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT Development
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        COMPONENT Development
)

# Create both static and shared libraries if explicitly requested
if(BUILD_SHARED_LIBS)
    # Also create a static version
    add_library(munbyn_static STATIC ${MUNBYN_SOURCES})

    set_target_properties(munbyn_static PROPERTIES
        OUTPUT_NAME munbyn
        VERSION ${PROJECT_VERSION}
        PUBLIC_HEADER "${MUNBYN_HEADERS}"
        C_STANDARD 99
        C_STANDARD_REQUIRED ON
    )

    # Platform-specific libraries for static version
    if(WIN32)
        target_link_libraries(munbyn_static PRIVATE ws2_32)
    elseif(NOT APPLE)
        target_link_libraries(munbyn_static PRIVATE m)
    endif()

    # Compiler-specific flags for static version
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(munbyn_static PRIVATE -Wall -Wextra -pedantic)
    endif()

    # Include directories for static version
    target_include_directories(munbyn_static
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )

    # Create alias for static version
    add_library(munbyn::munbyn_static ALIAS munbyn_static)

    # Install static library
    install(TARGETS munbyn_static
        EXPORT munbynTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            COMPONENT Development
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
            COMPONENT Development
    )
endif()
