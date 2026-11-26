#!/usr/bin/env bash
# 03 - Which MPI rank rendered which rows.
# Renders the scene with MPI, tints each rank's block of rows in its own
# colour, and prints each rank's own render time (also saved as a CSV).
#
# Settings (environment variables):
#   SCENE   demo | A | B   (default A)
#   WIDTH   image width    (default 1280)
#   SPP     samples/pixel  (default 64)
#   RANKS   MPI processes  (default 4; use 8 for one rank per hardware thread)
#   THREADS_PER_RANK OpenMP threads per rank (default 1, i.e. pure MPI)
source "$(dirname "$0")/../common/env.sh"
require_imagemagick

HERE="$SHOWCASE_DIR/03_rank_partition"
OUT="$HERE/output"
BIN="$HERE/build/rank_partition"
SCENE="${SCENE:-A}"
WIDTH="${WIDTH:-1280}"
SPP="${SPP:-64}"
RANKS="${RANKS:-4}"
export OMP_NUM_THREADS="${THREADS_PER_RANK:-1}"

mkdir -p "$OUT"
CXX=mpic++ build "$HERE/rank_partition.cpp" "$BIN" -DRT_USE_MPI

MPI_OPTS=()
if (( RANKS > 4 )); then
    MPI_OPTS+=(--use-hwthread-cpus)   # 8 ranks need the 8 hardware threads
fi

name="${SCENE}_${RANKS}ranks"
run mpirun "${MPI_OPTS[@]}" -np "$RANKS" "$BIN" --scene "$SCENE" --width "$WIDTH" --spp "$SPP" \
    --out "$OUT/$name.ppm"
to_png "$OUT/$name.ppm"
echo "Done: $OUT/$name.png and $OUT/${name}_rank_times.csv"
