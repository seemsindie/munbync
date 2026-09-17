# CMake build and installation

Requires CMake 3.16+ and a C99 compiler. Python 3 is required when tests are enabled.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
cmake --install build --prefix /tmp/munbyn-install
```

For older CTest versions without `--test-dir`, run `ctest --output-on-failure`
inside the build directory. Use a writable installation prefix or run the install
step with privileges appropriate to the chosen destination.

| Option | Default | Purpose |
| --- | --- | --- |
| `BUILD_SHARED_LIBS` | `ON` | Build both shared and static libraries; `OFF` builds only static |
| `BUILD_EXAMPLES` | `ON` | Build the C examples |
| `BUILD_TESTING` | `OFF` | Enable protocol, serial timeout, and relocated-install checks |
| `CMAKE_INSTALL_EXAMPLES` | unset | Install example executables and assets if enabled |
| `CMAKE_INSTALL_PREFIX` | platform default | Installation prefix |

The build script accepts `--build-dir`, `--static`, `--no-examples`, `--test`,
`--type`, `--jobs`, `--prefix`, and `--install`. `--clean` rebuilds outputs while
preserving the CMake cache; it never recursively deletes the selected directory.
Run `./build.sh --help` for the full option list. Source paths are resolved from
the script location, so it also works from a different working directory.

## Use an installed package

```cmake
cmake_minimum_required(VERSION 3.16)
project(my_printer_app C)
find_package(munbyn REQUIRED COMPONENTS munbyn)
add_executable(my_app main.c)
target_link_libraries(my_app PRIVATE munbyn::munbyn)
```

Configure the consumer with `-DCMAKE_PREFIX_PATH=/tmp/munbyn-install`.
`munbyn::munbyn_static` is also exported when both variants are built.
Targets carry their own include paths. `MUNBYN_INCLUDE_DIRS`,
`MUNBYN_LIBRARIES`, and `MUNBYN_VERSION` remain available for compatibility.
The install tree can be moved to a new prefix without embedding the build
machine's header paths.

The Linux build and consumer installation are tested. Native Windows transport
I/O remains unavailable; CMake support does not imply working Windows printing.
