#!/usr/bin/env bash
# 14 - Glowing marbles: glass marbles with coloured glowing cores on a dark mirror
# floor, with depth of field.
#
# Settings (environment variables):
#   WIDTH   image width       (default 1920; height follows at 16:9)
#   SPP     samples per pixel (default 2048)
#   THREADS OpenMP threads    (default: all logical CPUs)
source "$(dirname "$0")/../common/env.sh"
require_imagemagick

HERE="$SHOWCASE_DIR/14_glowing_marbles"
OUT="$HERE/output"
BIN="$HERE/build/glowing_marbles"
WIDTH="${WIDTH:-1920}"
SPP="${SPP:-2048}"

mkdir -p "$OUT"
build "$HERE/glowing_marbles.cpp" "$BIN"

img="$OUT/glowing_marbles_${WIDTH}w_${SPP}spp.ppm"
run "$BIN" --width "$WIDTH" --spp "$SPP" --out "$img"
to_png "$img"
echo "Done: ${img%.ppm}.png"
