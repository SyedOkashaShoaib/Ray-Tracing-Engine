#!/usr/bin/env bash
# 11 - Chrome sphere ocean: 10,000 mirror spheres on two overlapping waves, with
# synthwave sunset reflected across the crests.
#
# Settings (environment variables):
#   WIDTH   image width       (default 1920; height follows at 16:9)
#   SPP     samples per pixel (default 1024)
#   THREADS OpenMP threads    (default: all logical CPUs)
source "$(dirname "$0")/../common/env.sh"
require_imagemagick

HERE="$SHOWCASE_DIR/11_chrome_ocean"
OUT="$HERE/output"
BIN="$HERE/build/chrome_ocean"
WIDTH="${WIDTH:-1920}"
SPP="${SPP:-1024}"

mkdir -p "$OUT"
build "$HERE/chrome_ocean.cpp" "$BIN"

img="$OUT/chrome_ocean_${WIDTH}w_${SPP}spp.ppm"
run "$BIN" --width "$WIDTH" --spp "$SPP" --out "$img"
to_png "$img"
echo "Done: ${img%.ppm}.png"
