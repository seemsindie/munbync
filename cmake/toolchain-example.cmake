# CMake toolchain file for cross-compilation (example)
# This file can be used as a template for cross-compilation

# Usage: cmake -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-example.cmake ..

# System name (adjust as needed)
# set(CMAKE_SYSTEM_NAME Linux)

# Specify the cross compiler
# set(CMAKE_C_COMPILER arm-linux-gnueabihf-gcc)

# Specify the target environment
# set(CMAKE_FIND_ROOT_PATH /usr/arm-linux-gnueabihf)

# Search for programs in the build host directories
# set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# Search for libraries and headers in the target directories
# set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
# set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
