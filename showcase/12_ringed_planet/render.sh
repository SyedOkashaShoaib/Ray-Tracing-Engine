#!/usr/bin/env bash
# 12 - Ringed planet: Earth-textured planet, a tilted ring of 20,000 small spheres, a
# moon and one bright star. Lit by a single light, so it needs many samples.
#
# Settings (environment variables):
#   WIDTH   image width       (default 1920; height follows at 16:9)
#   SPP     samples per pixel (default 2048)
#   THREADS OpenMP threads    (default: all logical CPUs)
source "$(dirname "$0")/../common/env.sh"
require_imagemagick

HERE="$SHOWCASE_DIR/12_ringed_planet"
OUT="$HERE/output"
BIN="$HERE/build/ringed_planet"
WIDTH="${WIDTH:-1920}"
SPP="${SPP:-2048}"

mkdir -p "$OUT"
build "$HERE/ringed_planet.cpp" "$BIN"

img="$OUT/ringed_planet_${WIDTH}w_${SPP}spp.ppm"
run "$BIN" --width "$WIDTH" --spp "$SPP" --out "$img"
to_png "$img"
echo "Done: ${img%.ppm}.png"
