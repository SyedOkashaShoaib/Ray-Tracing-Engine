#!/usr/bin/env bash
# Shared settings for the showcase scripts. Sourced, not run directly.
set -euo pipefail

SHOWCASE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ROOT="$(cd "$SHOWCASE_DIR/.." && pwd)"

THREADS="${THREADS:-$(nproc)}"
export OMP_NUM_THREADS="$THREADS"
export OMP_SCHEDULE="${OMP_SCHEDULE:-dynamic,1}"
export OMP_DYNAMIC=FALSE

CXXFLAGS=(-std=c++20 -O3 -DNDEBUG -fopenmp -DRT_USE_OPENMP -I "$ROOT/src" -I "$SHOWCASE_DIR/common")

# build <source.cpp> <output-binary> [extra flags...]
build() {
    local src="$1" out="$2"
    shift 2
    mkdir -p "$(dirname "$out")"
    echo "Building $(basename "$out")"
    "${CXX:-g++}" "${CXXFLAGS[@]}" "$@" "$src" -o "$out"
}

# Keeps the laptop from suspending while a render runs (the screen can still lock).
# Falls back to a plain run where systemd-inhibit is missing or not usable.
if command -v systemd-inhibit >/dev/null 2>&1 \
    && systemd-inhibit --what=idle:sleep --why="check" true >/dev/null 2>&1; then
    INHIBIT=(systemd-inhibit --what=idle:sleep --why="Ray tracing showcase")
else
    INHIBIT=()
fi
run() {
    "${INHIBIT[@]}" "$@"
}

# to_png <file.ppm>  ->  file.png next to it
to_png() {
    convert "$1" "${1%.ppm}.png"
}

require_imagemagick() {
    if ! command -v convert >/dev/null 2>&1 || ! command -v montage >/dev/null 2>&1; then
        echo "ImageMagick is needed: sudo apt install imagemagick" >&2
        exit 1
    fi
}

# Scene A loads src/images/input/globe.png by a relative path, so every
# render runs from the repository root.
cd "$ROOT"

# annotate <image.png> <labels.txt>
# Draws each "x y text" line centred just below pixel (x, y), in place.
annotate() {
    local png="$1" labels="$2" size
    local w
    w=$(identify -format '%w' "$png")
    size=$(( w / 45 ))
    local args=()
    while read -r x y text; do
        [[ -z "${x:-}" ]] && continue
        local half=$(( size * ${#text} * 3 / 10 ))
        args+=(-annotate "+$(( x - half ))+$(( y + size / 3 ))" "$text")
    done < "$labels"
    convert "$png" -font DejaVu-Sans -pointsize "$size" -fill white -stroke black -strokewidth 1 \
        -gravity NorthWest "${args[@]}" "$png"
}
