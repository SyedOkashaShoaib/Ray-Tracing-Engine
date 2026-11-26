#!/usr/bin/env bash
# 02 - Samples per pixel: the same scene at 1, 4, 16, 64, 256, 1024 spp.
# Shows Monte Carlo noise falling with the square root of the sample count
# (each 4x step halves the noise). Also saves a 100% crop of each image,
# because noise is hard to see once full images are shrunk into a grid.
#
# Settings (environment variables):
#   SCENE   demo | A | B      (default A, the Material Garden)
#   WIDTH   image width       (default 960)
#   SPPS    list of spp       (default "1 4 16 64 256 1024")
#   CROP    crop geometry WxH+X+Y for the close-ups (default 320x180+320+220)
#   THREADS OpenMP threads    (default: all logical CPUs)
source "$(dirname "$0")/../common/env.sh"
require_imagemagick

HERE="$SHOWCASE_DIR/02_spp_convergence"
OUT="$HERE/output"
BIN="$SHOWCASE_DIR/common/build/render_scene"
SCENE="${SCENE:-A}"
WIDTH="${WIDTH:-960}"
SPPS="${SPPS:-1 4 16 64 256 1024}"
CROP="${CROP:-320x180+320+220}"

mkdir -p "$OUT"
build "$SHOWCASE_DIR/common/render_scene.cpp" "$BIN"

tiles=()
crops=()
for s in $SPPS; do
    img="$OUT/${SCENE}_spp_${s}.ppm"
    run "$BIN" --scene "$SCENE" --width "$WIDTH" --spp "$s" --out "$img"
    to_png "$img"
    convert "${img%.ppm}.png" -crop "$CROP" +repage -filter point -resize 200% "$OUT/${SCENE}_spp_${s}_crop.png"
    tiles+=(-label "$s spp" "${img%.ppm}.png")
    crops+=(-label "$s spp" "$OUT/${SCENE}_spp_${s}_crop.png")
done

montage "${tiles[@]}" -tile 3x -geometry +6+6 -pointsize 22 "$OUT/${SCENE}_spp_sequence.png"
montage "${crops[@]}" -tile 3x -geometry +6+6 -pointsize 22 "$OUT/${SCENE}_spp_crops.png"
echo "Done: $OUT/${SCENE}_spp_sequence.png and $OUT/${SCENE}_spp_crops.png"
