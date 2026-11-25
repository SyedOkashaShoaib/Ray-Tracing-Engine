#!/usr/bin/env bash
set -euo pipefail

for TRIAL in 1 2 3 4 5; do

    echo "=== OpenMP scaling | Trial $TRIAL ==="

    if (( TRIAL % 2 == 1 )); then
        THREAD_COUNTS=(1 2 4 8)
        SCENES=(A B)
    else
        THREAD_COUNTS=(8 4 2 1)
        SCENES=(B A)
    fi

    for SCENE in "${SCENES[@]}"; do

        for THREADS in "${THREAD_COUNTS[@]}"; do

            python3 scripts/benchmark_runner.py \
                --profile formal \
                --scene "$SCENE" \
                --mode openmp \
                --threads "$THREADS" \
                --schedule static \
                --trial-start "$TRIAL"

        done

    done

done