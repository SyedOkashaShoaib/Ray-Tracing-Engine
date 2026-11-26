#!/usr/bin/env bash
# 17 - Apollonian sphere packing: a ball filled with mutually touching spheres down
# to a minimum size, with a wedge cut away, in polished metal.
#
# Settings (environment variables):
#   WIDTH   image width       (default 1920; height follows at 16:9)
#   SPP     samples per pixel (default 1024)
#   THREADS OpenMP threads    (default: all logical CPUs)
source "$(dirname "$0")/../common/env.sh"
require_imagemagick

HERE="$SHOWCASE_DIR/17_apollonian_packing"
OUT="$HERE/output"
BIN="$HERE/build/apollonian_packing"
WIDTH="${WIDTH:-1920}"
SPP="${SPP:-1024}"

mkdir -p "$OUT"
build "$HERE/apollonian_packing.cpp" "$BIN"

img="$OUT/apollonian_packing_${WIDTH}w_${SPP}spp.ppm"
run "$BIN" --width "$WIDTH" --spp "$SPP" --out "$img"
to_png "$img"
echo "Done: ${img%.ppm}.png"
