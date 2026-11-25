
#!/usr/bin/env bash
set -euo pipefail

# Step 7.4.4 — Formal hybrid scaling
# 2 scenes × 7 configurations × 5 trials = 70 runs

CONFIGS=(
    "1 4"
    "2 2"
    "4 1"
    "1 8"
    "2 4"
    "4 2"
    "8 1"
)

for TRIAL in 1 2 3 4 5; do

    echo "======================================="
    echo "Hybrid Scaling | Trial $TRIAL"
    echo "======================================="

    if (( TRIAL % 2 == 1 )); then
        SCENES=(A B)
        ORDER=("${CONFIGS[@]}")
    else
        SCENES=(B A)
        ORDER=()

        for (( i=${#CONFIGS[@]}-1; i>=0; i-- )); do
            ORDER+=("${CONFIGS[i]}")
        done
    fi

    for SCENE in "${SCENES[@]}"; do

        for CONFIG in "${ORDER[@]}"; do

            read -r RANKS THREADS <<< "$CONFIG"

            echo
            echo "Scene=$SCENE | Ranks=$RANKS | Threads=$THREADS | Trial=$TRIAL"

            python3 scripts/benchmark_runner.py \
                --profile formal \
                --scene "$SCENE" \
                --mode hybrid \
                --ranks "$RANKS" \
                --threads "$THREADS" \
                --trial-start "$TRIAL"

        done

    done

done

echo
echo "=== Formal hybrid scaling completed ==="
