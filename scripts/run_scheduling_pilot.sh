
#!/usr/bin/env bash
set -euo pipefail

# Step 7.5 — OpenMP scheduling pilot
# 2 scenes x 2 thread counts x 3 schedules = 12 runs

for SCENE in A B; do
    for THREADS in 4 8; do

        if (( THREADS == 4 )); then
            export OMP_PLACES=cores
        else
            export OMP_PLACES=threads
        fi

        export OMP_PROC_BIND=spread

        for SCHEDULE in static dynamic,1 guided; do

            echo
            echo "Scene=$SCENE Threads=$THREADS Schedule=$SCHEDULE"

            python3 scripts/benchmark_runner.py \
                --profile formal \
                --scene "$SCENE" \
                --mode openmp \
                --threads "$THREADS" \
                --schedule "$SCHEDULE"

        done
    done
done

echo
echo "=== Scheduling pilot completed ==="
