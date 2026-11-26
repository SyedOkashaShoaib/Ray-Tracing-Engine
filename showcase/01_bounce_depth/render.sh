#!/usr/bin/env bash
# 01 - Bounce depth: the same scene with max_depth = 1, 2, 3, 5, 10, 30.
# Shows recursive ray tracing building up: depth 1 shows only the lights and
# background, depth 2 adds direct lighting, and reflections, light passing
# through glass and indirect light appear as more bounces are allowed.
#
# Settings (environment variables):
#   SCENE   demo | A | B      (default demo)
#   WIDTH   image width       (default 960; height follows at 16:9)
#   SPP     samples per pixel (default 256)
#   DEPTHS  list of depths    (default "1 2 3 5 10 30")
#   THREADS OpenMP threads    (default: all logical CPUs)
source "$(dirname "$0")/../common/env.sh"
require_imagemagick

HERE="$SHOWCASE_DIR/01_bounce_depth"
OUT="$HERE/output"
BIN="$SHOWCASE_DIR/common/build/render_scene"
SCENE="${SCENE:-demo}"
WIDTH="${WIDTH:-960}"
SPP="${SPP:-256}"
DEPTHS="${DEPTHS:-1 2 3 5 10 30}"

mkdir -p "$OUT"
build "$SHOWCASE_DIR/common/render_scene.cpp" "$BIN"

tiles=()
for d in $DEPTHS; do
    img="$OUT/${SCENE}_depth_${d}.ppm"
    run "$BIN" --scene "$SCENE" --width "$WIDTH" --spp "$SPP" --depth "$d" --out "$img"
    to_png "$img"
    tiles+=(-label "max depth = $d" "${img%.ppm}.png")
done

montage "${tiles[@]}" -tile 3x -geometry +6+6 -pointsize 22 \
    "$OUT/${SCENE}_bounce_depth_sequence.png"
echo "Done: $OUT/${SCENE}_bounce_depth_sequence.png"
