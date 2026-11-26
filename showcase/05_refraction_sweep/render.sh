#!/usr/bin/env bash
# 05 - Refraction index sweep: glass spheres with n = 1.00, 1.33, 1.50, 1.90, 2.42
# in front of a checkered wall. Each sphere is labelled with its index.
#
# Settings (environment variables):
#   WIDTH   image width       (default 1920; height follows at 16:9)
#   SPP     samples per pixel (default 512)
#   THREADS OpenMP threads    (default: all logical CPUs)
source "$(dirname "$0")/../common/env.sh"
require_imagemagick

HERE="$SHOWCASE_DIR/05_refraction_sweep"
OUT="$HERE/output"
BIN="$HERE/build/refraction_sweep"
WIDTH="${WIDTH:-1920}"
SPP="${SPP:-512}"

mkdir -p "$OUT"
build "$HERE/refraction_sweep.cpp" "$BIN"

img="$OUT/refraction_sweep_${WIDTH}w_${SPP}spp.ppm"
run "$BIN" --width "$WIDTH" --spp "$SPP" --out "$img"
to_png "$img"
if [[ -f "${img%.ppm}_labels.txt" ]]; then
    annotate "${img%.ppm}.png" "${img%.ppm}_labels.txt"
fi
echo "Done: ${img%.ppm}.png"
