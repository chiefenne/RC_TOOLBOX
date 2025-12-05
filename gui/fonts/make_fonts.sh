#!/bin/zsh

# Usage:
#   ./make_fonts_batch.sh <config_file>
#
# <config_file> format (one entry per line):
#   /path/to/font.ttf;size1,size2,size3
#
# Example Config Line:
#   /System/Library/Fonts/Supplemental/Courier New Bold.ttf;12,14,20

set -e

CONFIG_FILE="$1"

if [[ -z "$CONFIG_FILE" ]]; then
    echo "Usage: $0 <config_file>"
    echo "  Config file format per line: /path/to/font.ttf;12,14,16"
    exit 1
fi

if [[ ! -f "$CONFIG_FILE" ]]; then
    echo "Error: Config file not found: $CONFIG_FILE"
    exit 1
fi

# Read the file line by line
while IFS= read -r line || [[ -n "$line" ]]; do
    # 1. Trim leading/trailing whitespace
    line="${line#"${line%%[![:space:]]*}"}"
    line="${line%"${line##*[![:space:]]}"}"

    # 2. Skip empty lines or lines starting with # (comments)
    if [[ -z "$line" || "$line" == \#* ]]; then
        continue
    fi

    # 3. Parse the line using semicolon delimiter
    # format: path/to/file;sizes
    
    # Get everything before the last semicolon
    FONT_FILE="${line%;*}"
    # Get everything after the last semicolon
    FONT_SIZES_RAW="${line##*;}"

    # Remove any accidental spaces from the sizes string
    FONT_SIZES_CLEAN="${FONT_SIZES_RAW// /}"

    if [[ ! -f "$FONT_FILE" ]]; then
        echo "⚠️  Skipping: Font file not found: $FONT_FILE"
        echo "-----------------------------"
        continue
    fi

    if [[ -z "$FONT_SIZES_CLEAN" ]]; then
        echo "⚠️  Skipping: No sizes defined for $FONT_FILE"
        echo "-----------------------------"
        continue
    fi

    # Split comma-separated sizes into a zsh array
    SIZES=(${(s:,:)FONT_SIZES_CLEAN})

    # Get base name without extension for naming
    BASENAME=${FONT_FILE:t}
    NAME_NO_EXT=${BASENAME%.*}

    # Replace spaces with underscores and convert to lowercase
    SAFE_NAME="${NAME_NO_EXT// /_}"
    SAFE_NAME="${SAFE_NAME:l}"

    # Iterate through sizes for this specific font
    for SIZE in $SIZES; do
        OUTPUT="${SAFE_NAME}_${SIZE}.c"

        echo "Processing: $BASENAME @ ${SIZE}px"
        
        # Run conversion
        # The range includes ASCII, German Umlauts, Micro sign, Greek small mu
        # * Opts from online LVGL font converter: --bpp 4 --size 14 --no-compress --stride 1 --align 1 --font Arial.ttf --symbols ÄÖÜ@&%!?μ --range 32-127,196,214,220,223,228,246,252 --format lvgl -o arial_14_online.c

        npx github:lvgl/lv_font_conv \
            --size "$SIZE" \
            --bpp 4 \
            --no-compress \
            --stride 1 \
            --align 1 \
            --font "$FONT_FILE" \
            --format lvgl \
            --range 0x20-0x7F,0xC4,0xD6,0xDC,0xDF,0xE4,0xF6,0xFC,0xB5,0x3BC,0xA9,0x3C,0x3E,0x25BA,0x25C4,0x25B2,0x25BC \
            --output "$OUTPUT"
    done
    
    echo "✅ Finished processing $BASENAME"
    echo "-----------------------------"

done < "$CONFIG_FILE"

echo "All fonts processed."
