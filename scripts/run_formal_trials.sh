#!/usr/bin/env bash
set -euo pipefail

for SCENE in A B; do
    for TRIAL in 1 2 3 4 5; do

        echo "=== Scene $SCENE | Trial $TRIAL ==="

        if (( TRIAL % 2 == 1 )); then
            MODES=(serial openmp mpi hybrid)
        else
            MODES=(hybrid mpi openmp serial)
        fi

        for MODE in "${MODES[@]}"; do

            COMMON=(
                --profile formal
                --scene "$SCENE"
                --mode "$MODE"
                --trial-start "$TRIAL"
            )

            case "$MODE" in
                serial)
                    python3 scripts/benchmark_runner.py \
                        "${COMMON[@]}"
                    ;;

                openmp)
                    python3 scripts/benchmark_runner.py \
                        "${COMMON[@]}" \
                        --threads 4 \
                        --schedule static
                    ;;

                mpi)
                    python3 scripts/benchmark_runner.py \
                        "${COMMON[@]}" \
                        --ranks 4
                    ;;

                hybrid)
                    python3 scripts/benchmark_runner.py \
                        "${COMMON[@]}" \
                        --ranks 2 \
                        --threads 2 \
                        --schedule static
                    ;;
            esac
        done
    done
done