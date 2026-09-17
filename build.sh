#!/usr/bin/env bash
# Build from any working directory with any CMake generator.
set -euo pipefail

SOURCE_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SOURCE_DIR/build"
BUILD_TYPE=Release
INSTALL_PREFIX=/usr/local
BUILD_EXAMPLES=ON
BUILD_SHARED=ON
BUILD_TESTING=OFF
CLEAN_BUILD=false
INSTALL=false
JOBS="$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 2)"

usage() {
    cat <<'USAGE'
Usage: build.sh [OPTIONS]
  -h, --help              Show help
  -c, --clean             Rebuild all outputs (preserves the CMake cache)
  -t, --type TYPE         Debug|Release|RelWithDebInfo|MinSizeRel
  -d, --build-dir DIR     Build directory (default: <repository>/build)
  -p, --prefix PREFIX     Install prefix (default: /usr/local)
  -j, --jobs JOBS         Number of parallel build jobs
  -s, --static            Build static library only
  --no-examples           Skip example programs
  --test                  Build and run automated checks
  --install               Install after a successful build and checks
USAGE
}

while (($#)); do
    case "$1" in
        -h|--help) usage; exit 0 ;;
        -c|--clean) CLEAN_BUILD=true; shift ;;
        -s|--static) BUILD_SHARED=OFF; shift ;;
        --no-examples) BUILD_EXAMPLES=OFF; shift ;;
        --test) BUILD_TESTING=ON; shift ;;
        --install) INSTALL=true; shift ;;
        -t|--type|-d|--build-dir|-p|--prefix|-j|--jobs)
            if (($# < 2)) || [[ -z "$2" || "$2" == -* ]]; then
                echo "Missing value for $1" >&2; exit 2
            fi
            case "$1" in
                -t|--type) BUILD_TYPE="$2" ;;
                -d|--build-dir) BUILD_DIR="$2" ;;
                -p|--prefix) INSTALL_PREFIX="$2" ;;
                -j|--jobs) JOBS="$2" ;;
            esac
            shift 2 ;;
        *) echo "Unknown option: $1" >&2; usage >&2; exit 2 ;;
    esac
done

case "$BUILD_TYPE" in
    Debug|Release|RelWithDebInfo|MinSizeRel) ;;
    *) echo "Invalid build type: $BUILD_TYPE" >&2; exit 2 ;;
esac
if [[ ! "$JOBS" =~ ^[1-9][0-9]*$ ]]; then
    echo "Jobs must be a positive integer" >&2; exit 2
fi

cmake -S "$SOURCE_DIR" -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX" \
    -DBUILD_SHARED_LIBS="$BUILD_SHARED" \
    -DBUILD_EXAMPLES="$BUILD_EXAMPLES" \
    -DBUILD_TESTING="$BUILD_TESTING"

build_args=(--build "$BUILD_DIR" --parallel "$JOBS")
if [[ "$CLEAN_BUILD" == true ]]; then build_args+=(--clean-first); fi
cmake "${build_args[@]}"
if [[ "$BUILD_TESTING" == ON ]]; then
    (cd "$BUILD_DIR" && ctest --output-on-failure)
fi
if [[ "$INSTALL" == true ]]; then cmake --install "$BUILD_DIR"; fi
printf 'Build completed: %s\n' "$BUILD_DIR"
