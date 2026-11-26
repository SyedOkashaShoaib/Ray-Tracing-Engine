#!/usr/bin/env bash
# 18 - Pastel room: floating chrome, porcelain and softly glowing pastel spheres
# in a sage and peach room with soft, low-contrast light. A closed room, so it
# takes a while (about 1.5 hours at the defaults on a 4-core laptop).
#
# Settings (environment variables):
#   WIDTH   image width       (default 1920; height follows at 16:9)
#   SPP     samples per pixel (default 1024)
#   THREADS OpenMP threads    (default: all logical CPUs)
source "$(dirname "$0")/../common/env.sh"
require_imagemagick

HERE="$SHOWCASE_DIR/18_pastel_room"
OUT="$HERE/output"
BIN="$HERE/build/pastel_room"
WIDTH="${WIDTH:-1920}"
SPP="${SPP:-1024}"

mkdir -p "$OUT"
build "$HERE/pastel_room.cpp" "$BIN"

img="$OUT/pastel_room_${WIDTH}w_${SPP}spp.ppm"
run "$BIN" --width "$WIDTH" --spp "$SPP" --out "$img"
to_png "$img"
echo "Done: ${img%.ppm}.png"
