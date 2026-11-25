
#!/usr/bin/env bash
set -euo pipefail

# Step 7.4.3 - Hybrid scaling pilot
# Benchmark A and B at the frozen formal workload.

CONFIGS=(
    "1 4"
    "2 2"
    "4 1"
    "1 8"
    "2 4"
    "4 2"
    "8 1"
)

for SCENE in A B; do

    echo "======================================="
    echo "Hybrid scaling pilot | Scene $SCENE"
    echo "======================================="

    for CONFIG in "${CONFIGS[@]}"; do

        read -r RANKS THREADS <<< "$CONFIG"

        echo
        echo "Scene=$SCENE | Ranks=$RANKS | Threads=$THREADS"

        python3 scripts/benchmark_runner.py \
            --profile formal \
            --scene "$SCENE" \
            --mode hybrid \
            --ranks "$RANKS" \
            --threads "$THREADS" \
            --trial-start 1

    done
done

echo
echo "=== Hybrid scaling pilot completed ==="
