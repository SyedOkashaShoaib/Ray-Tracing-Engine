#!/usr/bin/env bash
# 08 - Colour bleeding: white spheres in a room with a red and a green wall pick up
# red and green tints from light bouncing off the walls. Lit by one lamp, so it
# needs many samples per pixel; preview with SPP=64 first. Bounce limit is 8 (see the .cpp).
#
# Settings (environment variables):
#   WIDTH   image width       (default 1920; height follows at 16:9)
#   SPP     samples per pixel (default 512)
#   THREADS OpenMP threads    (default: all logical CPUs)
source "$(dirname "$0")/../common/env.sh"
require_imagemagick

HERE="$SHOWCASE_DIR/08_colour_bleeding"
OUT="$HERE/output"
BIN="$HERE/build/colour_bleeding"
WIDTH="${WIDTH:-1920}"
SPP="${SPP:-512}"

mkdir -p "$OUT"
build "$HERE/colour_bleeding.cpp" "$BIN"

img="$OUT/colour_bleeding_${WIDTH}w_${SPP}spp.ppm"
run "$BIN" --width "$WIDTH" --spp "$SPP" --out "$img"
to_png "$img"
if [[ -f "${img%.ppm}_labels.txt" ]]; then
    annotate "${img%.ppm}.png" "${img%.ppm}_labels.txt"
fi
echo "Done: ${img%.ppm}.png"
