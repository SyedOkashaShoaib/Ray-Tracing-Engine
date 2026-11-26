#!/usr/bin/env bash
# 10 - Spiral galaxy of ~34,000 glowing spheres. Nearly noise-free and the quickest
# of the showcase scenes.
#
# Settings (environment variables):
#   WIDTH   image width       (default 1920; height follows at 16:9)
#   SPP     samples per pixel (default 256)
#   THREADS OpenMP threads    (default: all logical CPUs)
source "$(dirname "$0")/../common/env.sh"
require_imagemagick

HERE="$SHOWCASE_DIR/10_galaxy"
OUT="$HERE/output"
BIN="$HERE/build/galaxy"
WIDTH="${WIDTH:-1920}"
SPP="${SPP:-256}"

mkdir -p "$OUT"
build "$HERE/galaxy.cpp" "$BIN"

img="$OUT/galaxy_${WIDTH}w_${SPP}spp.ppm"
run "$BIN" --width "$WIDTH" --spp "$SPP" --out "$img"
to_png "$img"
echo "Done: ${img%.ppm}.png"
