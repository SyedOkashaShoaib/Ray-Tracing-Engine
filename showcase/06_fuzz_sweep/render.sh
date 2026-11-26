#!/usr/bin/env bash
# 06 - Metal fuzz sweep: silver spheres with fuzz 0, 0.1, 0.2, 0.4, 0.7, 1.0,
# from a sharp mirror to rough metal. Each sphere is labelled with its fuzz.
#
# Settings (environment variables):
#   WIDTH   image width       (default 1920; height follows at 16:9)
#   SPP     samples per pixel (default 512)
#   THREADS OpenMP threads    (default: all logical CPUs)
source "$(dirname "$0")/../common/env.sh"
require_imagemagick

HERE="$SHOWCASE_DIR/06_fuzz_sweep"
OUT="$HERE/output"
BIN="$HERE/build/fuzz_sweep"
WIDTH="${WIDTH:-1920}"
SPP="${SPP:-512}"

mkdir -p "$OUT"
build "$HERE/fuzz_sweep.cpp" "$BIN"

img="$OUT/fuzz_sweep_${WIDTH}w_${SPP}spp.ppm"
run "$BIN" --width "$WIDTH" --spp "$SPP" --out "$img"
to_png "$img"
if [[ -f "${img%.ppm}_labels.txt" ]]; then
    annotate "${img%.ppm}.png" "${img%.ppm}_labels.txt"
fi
echo "Done: ${img%.ppm}.png"
