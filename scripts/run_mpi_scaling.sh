#!/usr/bin/env bash
set -euo pipefail

for TRIAL in 1 2 3 4 5; do

    echo "=== MPI Scaling | Trial $TRIAL ==="

    if (( TRIAL % 2 == 1 )); then
        SCENES=(A B)
        RANK_COUNTS=(1 2 4 8)
    else
        SCENES=(B A)
        RANK_COUNTS=(8 4 2 1)
    fi

    for SCENE in "${SCENES[@]}"; do

        for RANKS in "${RANK_COUNTS[@]}"; do

            echo "Scene=$SCENE Ranks=$RANKS Trial=$TRIAL"

            python3 scripts/benchmark_runner.py \
                --profile formal \
                --scene "$SCENE" \
                --mode mpi \
                --ranks "$RANKS" \
                --trial-start "$TRIAL"

        done

    done

done

echo "=== MPI scaling completed ==="