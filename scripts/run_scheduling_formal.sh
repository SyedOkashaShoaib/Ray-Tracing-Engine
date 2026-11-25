
#!/usr/bin/env bash
set -euo pipefail

# Step 7.5 — Formal OpenMP scheduling experiment
# 2 scenes × 2 thread counts × 3 schedules × 5 trials

SCHEDULES=(static dynamic,1 guided)

for TRIAL in 1 2 3 4 5; do

    echo "===================================="
    echo "Scheduling experiment | Trial $TRIAL"
    echo "===================================="

    if (( TRIAL % 2 == 1 )); then
        SCENES=(A B)
        THREAD_COUNTS=(4 8)
    else
        SCENES=(B A)
        THREAD_COUNTS=(8 4)
    fi

    # Rotate schedule order between trials
    OFFSET=$(( (TRIAL - 1) % 3 ))

    for SCENE in "${SCENES[@]}"; do

        for THREADS in "${THREAD_COUNTS[@]}"; do

            if (( THREADS == 4 )); then
                export OMP_PLACES=cores
            else
                export OMP_PLACES=threads
            fi

            export OMP_PROC_BIND=spread

            for INDEX in 0 1 2; do

                POSITION=$(( (INDEX + OFFSET) % 3 ))
                SCHEDULE="${SCHEDULES[$POSITION]}"

                echo
                echo "Scene=$SCENE Threads=$THREADS Schedule=$SCHEDULE Trial=$TRIAL"

                python3 scripts/benchmark_runner.py \
                    --profile formal \
                    --scene "$SCENE" \
                    --mode openmp \
                    --threads "$THREADS" \
                    --schedule "$SCHEDULE" \
                    --trial-start "$TRIAL"

            done
        done
    done
done

echo
echo "=== Formal scheduling experiment completed ==="
