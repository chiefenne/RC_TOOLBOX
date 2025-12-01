#!/bin/bash
# simulator/build_sim_debug.sh – compile with debug symbols (macOS only)
set -euo pipefail

OUTPUT_DIR="binaries"
EXECUTABLE="$OUTPUT_DIR/lvgl_simulator_macOS_debug"

# Create the binaries folder if it doesn't exist
mkdir -p "$OUTPUT_DIR"

INCLUDES="-I. -Ilvgl -Igui -Igui/pages -Igui/fonts"
DEBUG_FLAGS="-g -O0"

# Pre-compile LVGL font .c files with plain clang (no C++ mangling)
FONT_OBJS=()
for src in gui/fonts/*.c; do
    obj="${src%.c}.o"
    clang $DEBUG_FLAGS -c "$src" $INCLUDES -o "$obj"
    FONT_OBJS+=("$obj")
done

# Build the simulator with debug symbols
clang++ $DEBUG_FLAGS simulator/main.cpp simulator/sim_state.cpp \
    gui/*.cpp gui/pages/*.cpp "${FONT_OBJS[@]}" \
    $INCLUDES \
    -std=c++17 \
    lvgl/liblvgl.a -lSDL2 \
    -o "$EXECUTABLE"

# Optional: remove temporary font objects
rm -f "${FONT_OBJS[@]}"

echo "Debug build successful: $EXECUTABLE"
