#!/usr/bin/env bash
# 15 - Snow globe: a water-filled glass globe (n = 1.33) packed with ~600 coloured
# spheres and snow, magnified and bent by the water.
#
# Settings (environment variables):
#   WIDTH   image width       (default 1920; height follows at 16:9)
#   SPP     samples per pixel (default 1024)
#   THREADS OpenMP threads    (default: all logical CPUs)
source "$(dirname "$0")/../common/env.sh"
require_imagemagick

HERE="$SHOWCASE_DIR/15_snow_globe"
OUT="$HERE/output"
BIN="$HERE/build/snow_globe"
WIDTH="${WIDTH:-1920}"
SPP="${SPP:-1024}"

mkdir -p "$OUT"
build "$HERE/snow_globe.cpp" "$BIN"

img="$OUT/snow_globe_${WIDTH}w_${SPP}spp.ppm"
run "$BIN" --width "$WIDTH" --spp "$SPP" --out "$img"
to_png "$img"
echo "Done: ${img%.ppm}.png"
