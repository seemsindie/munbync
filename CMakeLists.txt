cmake_minimum_required(VERSION 3.16)

# Project definition
project(munbyn
    VERSION 1.0.0
    DESCRIPTION "Munbyn Thermal Printer Library"
    LANGUAGES C
)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Build options
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(BUILD_EXAMPLES "Build example programs" ON)
option(BUILD_TESTING "Build tests" OFF)
include(GNUInstallDirs)

# Platform-specific settings
if(WIN32)
    set(PLATFORM_LIBS ws2_32)
elseif(APPLE)
    set(PLATFORM_LIBS)
else()
    set(PLATFORM_LIBS m)
endif()

# Compiler flags
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2")
endif()

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# Add subdirectories
add_subdirectory(src)

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()

# Package configuration
include(CMakePackageConfigHelpers)

# Generate package config file
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/munbynConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/munbynConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/munbyn
    PATH_VARS CMAKE_INSTALL_INCLUDEDIR
)

# Generate package version file
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/munbynConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

# Install package config files
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/munbynConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/munbynConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/munbyn
    COMPONENT Development
)

# Export targets
install(EXPORT munbynTargets
    FILE munbynTargets.cmake
    NAMESPACE munbyn::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/munbyn
    COMPONENT Development
)

# CPack configuration for package generation
set(CPACK_PACKAGE_NAME "munbyn")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_DESCRIPTION})
set(CPACK_PACKAGE_VENDOR "Munbyn")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

include(CPack)

# Print configuration summary
message(STATUS "")
message(STATUS "Configuration Summary:")
message(STATUS "  Project: ${PROJECT_NAME} ${PROJECT_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Build shared libs: ${BUILD_SHARED_LIBS}")
message(STATUS "  Build examples: ${BUILD_EXAMPLES}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Platform libraries: ${PLATFORM_LIBS}")
message(STATUS "")
