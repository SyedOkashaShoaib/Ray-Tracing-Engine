#!/usr/bin/env bash
# 07 - Mirror corridor: two parallel mirror walls reflect each other, so the
# objects between them repeat down an endless corridor. Each copy is one more
# bounce; rendering several depths shows the corridor growing.
#
# Settings (environment variables):
#   WIDTH   image width       (default 1920)
#   SPP     samples per pixel (default 512)
#   DEPTHS  bounce limits     (default "30"; e.g. "2 4 8 30" for a sequence)
#   THREADS OpenMP threads    (default: all logical CPUs)
source "$(dirname "$0")/../common/env.sh"
require_imagemagick

HERE="$SHOWCASE_DIR/07_mirror_corridor"
OUT="$HERE/output"
BIN="$HERE/build/mirror_corridor"
WIDTH="${WIDTH:-1920}"
SPP="${SPP:-512}"
DEPTHS="${DEPTHS:-30}"

mkdir -p "$OUT"
build "$HERE/mirror_corridor.cpp" "$BIN"

tiles=()
for d in $DEPTHS; do
    img="$OUT/mirror_corridor_depth_${d}.ppm"
    run "$BIN" --width "$WIDTH" --spp "$SPP" --depth "$d" --out "$img"
    to_png "$img"
    tiles+=(-label "max depth = $d" "${img%.ppm}.png")
done
if (( ${#tiles[@]} > 3 )); then
    montage "${tiles[@]}" -tile 2x -geometry +6+6 -pointsize 22 "$OUT/mirror_corridor_sequence.png"
    echo "Done: $OUT/mirror_corridor_sequence.png"
else
    echo "Done: ${img%.ppm}.png"
fi
