#!/bin/bash

# Vectoria Web Build Script
# Requires Emscripten SDK to be installed and activated

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_DIR/build-web"
OUTPUT_DIR="$PROJECT_DIR/web/dist"

echo "========================================="
echo "  Vectoria Web Build (Emscripten)"
echo "========================================="

# Check if Emscripten is available
if ! command -v emcc &> /dev/null; then
    echo "Error: Emscripten not found!"
    echo ""
    echo "Please install and activate Emscripten SDK:"
    echo "  git clone https://github.com/emscripten-core/emsdk.git"
    echo "  cd emsdk"
    echo "  ./emsdk install latest"
    echo "  ./emsdk activate latest"
    echo "  source ./emsdk_env.sh"
    exit 1
fi

echo "Emscripten version: $(emcc --version | head -n1)"

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure with Emscripten
echo ""
echo "Configuring CMake with Emscripten..."
emcmake cmake "$PROJECT_DIR" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_VERBOSE_MAKEFILE=ON

# Build
echo ""
echo "Building..."
emmake make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

# Create output directory and copy files
echo ""
echo "Copying output files..."
mkdir -p "$OUTPUT_DIR"
cp "$BUILD_DIR/Vectoria.html" "$OUTPUT_DIR/index.html"
cp "$BUILD_DIR/Vectoria.js" "$OUTPUT_DIR/"
cp "$BUILD_DIR/Vectoria.wasm" "$OUTPUT_DIR/"
cp "$BUILD_DIR/Vectoria.data" "$OUTPUT_DIR/" 2>/dev/null || true

echo ""
echo "========================================="
echo "  Build Complete!"
echo "========================================="
echo ""
echo "Output files in: $OUTPUT_DIR"
echo ""
echo "To test locally, run:"
echo "  cd $OUTPUT_DIR"
echo "  python3 -m http.server 8080"
echo ""
echo "Then open: http://localhost:8080"
echo ""
