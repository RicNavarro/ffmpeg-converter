#!/usr/bin/env bash
# deploy-linux.sh
# Builds the project and generates a .deb package ready for installation.
#
# Usage:
#   chmod +x deploy-linux.sh
#   ./deploy-linux.sh
#
# Optional env vars:
#   BUILD_DIR   — build directory (default: build-release)
#   INSTALL_QT  — set to 1 if using a manually installed Qt6 (not system Qt)
#   QT_PREFIX   — path to Qt6 install prefix (e.g. ~/Qt/6.7.0/gcc_64)

set -euo pipefail

BUILD_DIR="${BUILD_DIR:-build-release}"
QT_PREFIX="${QT_PREFIX:-}"

echo ""
echo "=== FFmpeg Converter — Linux build ==="
echo ""

# -----------------------------------------------------------------------
# 1. Check dependencies
# -----------------------------------------------------------------------
echo "[1/5] Checking dependencies..."

MISSING=""
for cmd in cmake g++ pkg-config ffmpeg; do
    if ! command -v "$cmd" &>/dev/null; then
        MISSING="$MISSING $cmd"
    fi
done

# Qt6 check — either system Qt or manually installed
if [ -n "$QT_PREFIX" ]; then
    CMAKE_PREFIX_ARG="-DCMAKE_PREFIX_PATH=$QT_PREFIX"
    echo "      Using Qt6 from: $QT_PREFIX"
else
    # Try to find system Qt6 via CMake (more reliable than pkg-config on Ubuntu/Pop!OS)
    QT6_CMAKE_HINT=$(dpkg -L qt6-base-dev 2>/dev/null | grep "Qt6Config.cmake" | head -1)
    QT6_CMAKE_HINT=$(dirname "$QT6_CMAKE_HINT" 2>/dev/null || true)

    if [ -n "$QT6_CMAKE_HINT" ]; then
        CMAKE_PREFIX_ARG="-DCMAKE_PREFIX_PATH=$(dirname "$(dirname "$QT6_CMAKE_HINT")")"
        QT6_VER=$(cat "$(dirname "$QT6_CMAKE_HINT")/Qt6ConfigVersion.cmake" 2>/dev/null \
            | grep "PACKAGE_VERSION " | head -1 | grep -o '[0-9.]*' || echo "?")
        echo "      Found system Qt6 $QT6_VER via dpkg"
    elif pkg-config --exists Qt6Widgets 2>/dev/null; then
        CMAKE_PREFIX_ARG=""
        echo "      Using system Qt6 ($(pkg-config --modversion Qt6Core))"
    else
        echo ""
        echo "ERROR: Qt6 not found. Try:"
        echo "  sudo apt install qt6-base-dev"
        echo "  # or for a manual Qt install:"
        echo "  QT_PREFIX=~/Qt/6.x.x/gcc_64 ./deploy-linux.sh"
        exit 1
    fi
fi

if [ -n "$MISSING" ]; then
    echo ""
    echo "ERROR: Missing tools:$MISSING"
    echo "Install with:  sudo apt install cmake g++ ffmpeg"
    exit 1
fi

# -----------------------------------------------------------------------
# 2. Configure
# -----------------------------------------------------------------------
echo "[2/5] Configuring with CMake..."

cmake -S . -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE=Release \
    ${CMAKE_PREFIX_ARG}

# -----------------------------------------------------------------------
# 3. Build
# -----------------------------------------------------------------------
echo "[3/5] Building..."

JOBS=$(nproc 2>/dev/null || echo 4)
cmake --build "$BUILD_DIR" --config Release --parallel "$JOBS"

# -----------------------------------------------------------------------
# 4. Generate .deb
# -----------------------------------------------------------------------
echo "[4/5] Generating .deb package..."

cd "$BUILD_DIR"
# Make sure the deb scripts are executable
chmod +x ../linux/deb/postinst ../linux/deb/postrm 2>/dev/null || true
cpack -G DEB
cd ..

DEB_FILE=$(ls "$BUILD_DIR"/ffmpeg-converter-*.deb 2>/dev/null | head -1)

if [ -z "$DEB_FILE" ]; then
    echo "WARNING: .deb file not found — CPack may have failed."
    echo "You can still install manually:"
    echo "  sudo cmake --install $BUILD_DIR"
    exit 0
fi

# -----------------------------------------------------------------------
# 5. Done
# -----------------------------------------------------------------------
echo "[5/5] Done!"
echo ""
echo "Package ready:  $DEB_FILE"
echo ""
echo "To install:"
echo "  sudo apt install ./$DEB_FILE"
echo ""
echo "This will:"
echo "  - Install the app to /usr/bin/FFmpegWrapper"
echo "  - Register it in your application launcher (Activities / dock)"
echo "  - Associate it with video and audio file types"
echo "  - Allow uninstall via:  sudo apt remove ffmpeg-converter"
echo ""
