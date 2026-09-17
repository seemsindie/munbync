"""Check the documented find_package API in a relocated install tree."""
import pathlib
import subprocess
import sys
import tempfile

cmake, build = sys.argv[1:]

def run(*args):
    subprocess.run([cmake, *map(str, args)], check=True)

with tempfile.TemporaryDirectory(prefix="munbyn-install-") as directory:
    root = pathlib.Path(directory)
    installed = root / "installed"
    run("--install", build, "--prefix", installed)
    moved = root / "relocated"
    installed.rename(moved)
    source = root / "consumer"
    source.mkdir()
    (source / "main.c").write_text(
        '#include <munbyn_printer.h>\n'
        'int main(void) { return munbyn_self_test(NULL) == MUNBYN_ERROR_INVALID_PARAMETER ? 0 : 1; }\n'
    )
    (source / "CMakeLists.txt").write_text('''cmake_minimum_required(VERSION 3.16)
project(consumer C)
find_package(munbyn REQUIRED COMPONENTS munbyn)
if(NOT EXISTS "${MUNBYN_INCLUDE_DIRS}/munbyn_printer.h")
  message(FATAL_ERROR "The compatibility include path is not relocatable")
endif()
add_executable(consumer main.c)
target_link_libraries(consumer PRIVATE munbyn::munbyn)
if(TARGET munbyn::munbyn_static)
  add_executable(static_consumer main.c)
  target_link_libraries(static_consumer PRIVATE munbyn::munbyn_static)
endif()
''')
    run("-S", source, "-B", root / "build", f"-DCMAKE_PREFIX_PATH={moved}")
    run("--build", root / "build")
    subprocess.run([str(root / "build/consumer")], check=True)
    if (root / "build/static_consumer").exists():
        subprocess.run([str(root / "build/static_consumer")], check=True)
