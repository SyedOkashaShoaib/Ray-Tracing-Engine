#!/usr/bin/env bash
# 09 - Coloured light mixing: red, green and blue lights over white spheres give
# cyan, magenta and yellow shadows where one of the lights is blocked.
# Small lights in the dark need many samples; preview with SPP=64 first.
#
# Settings (environment variables):
#   WIDTH   image width       (default 1920; height follows at 16:9)
#   SPP     samples per pixel (default 1024)
#   THREADS OpenMP threads    (default: all logical CPUs)
source "$(dirname "$0")/../common/env.sh"
require_imagemagick

HERE="$SHOWCASE_DIR/09_rgb_light_mixing"
OUT="$HERE/output"
BIN="$HERE/build/rgb_light_mixing"
WIDTH="${WIDTH:-1920}"
SPP="${SPP:-1024}"

mkdir -p "$OUT"
build "$HERE/rgb_light_mixing.cpp" "$BIN"

img="$OUT/rgb_light_mixing_${WIDTH}w_${SPP}spp.ppm"
run "$BIN" --width "$WIDTH" --spp "$SPP" --out "$img"
to_png "$img"
if [[ -f "${img%.ppm}_labels.txt" ]]; then
    annotate "${img%.ppm}.png" "${img%.ppm}_labels.txt"
fi
echo "Done: ${img%.ppm}.png"
