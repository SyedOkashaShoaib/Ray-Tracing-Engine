#!/usr/bin/env bash
# 04 - Cost heatmap: rays traced per pixel, plus each row's render time.
# Shows directly that some parts of the image cost far more than others,
# which is why fixed row blocks leave MPI ranks idle.
#
# Settings (environment variables):
#   SCENE   demo | A | B   (default A)
#   WIDTH   image width    (default 1280, the benchmark size)
#   SPP     samples/pixel  (default 32; the pattern does not depend on spp)
#   THREADS OpenMP threads (default: all logical CPUs)
# Needs: sudo apt install python3-numpy python3-matplotlib imagemagick
source "$(dirname "$0")/../common/env.sh"
require_imagemagick

HERE="$SHOWCASE_DIR/04_cost_heatmap"
OUT="$HERE/output"
BIN="$HERE/build/cost_heatmap"
SCENE="${SCENE:-A}"
WIDTH="${WIDTH:-1280}"
SPP="${SPP:-32}"

mkdir -p "$OUT"
build "$HERE/cost_heatmap.cpp" "$BIN"

run "$BIN" --scene "$SCENE" --width "$WIDTH" --spp "$SPP" --out "$OUT/$SCENE"
to_png "$OUT/$SCENE.ppm"
python3 "$HERE/make_heatmap.py" "$OUT/$SCENE"
