#!/usr/bin/env bash
# 13 - Golden-spiral sunflower: 900 seed spheres placed at the golden angle (spiral
# arms show as bands), ringed by 26 petals of yellow spheres, seen from above.
#
# Settings (environment variables):
#   WIDTH   image width       (default 1920; height follows at 16:9)
#   SPP     samples per pixel (default 1024)
#   THREADS OpenMP threads    (default: all logical CPUs)
source "$(dirname "$0")/../common/env.sh"
require_imagemagick

HERE="$SHOWCASE_DIR/13_golden_spiral"
OUT="$HERE/output"
BIN="$HERE/build/golden_spiral"
WIDTH="${WIDTH:-1920}"
SPP="${SPP:-1024}"

mkdir -p "$OUT"
build "$HERE/golden_spiral.cpp" "$BIN"

img="$OUT/golden_spiral_${WIDTH}w_${SPP}spp.ppm"
run "$BIN" --width "$WIDTH" --spp "$SPP" --out "$img"
to_png "$img"
echo "Done: ${img%.ppm}.png"
