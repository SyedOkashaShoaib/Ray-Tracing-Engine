#!/usr/bin/env bash
# 16 - DNA double helix: gold strands joined by glowing rungs, tilted across the
# frame with depth of field.
#
# Settings (environment variables):
#   WIDTH   image width       (default 1920; height follows at 16:9)
#   SPP     samples per pixel (default 2048)
#   THREADS OpenMP threads    (default: all logical CPUs)
source "$(dirname "$0")/../common/env.sh"
require_imagemagick

HERE="$SHOWCASE_DIR/16_dna_helix"
OUT="$HERE/output"
BIN="$HERE/build/dna_helix"
WIDTH="${WIDTH:-1920}"
SPP="${SPP:-2048}"

mkdir -p "$OUT"
build "$HERE/dna_helix.cpp" "$BIN"

img="$OUT/dna_helix_${WIDTH}w_${SPP}spp.ppm"
run "$BIN" --width "$WIDTH" --spp "$SPP" --out "$img"
to_png "$img"
echo "Done: ${img%.ppm}.png"
