
import argparse
import csv
import filecmp
import os
import subprocess
import time
from datetime import datetime, timezone
from pathlib import Path
from uuid import uuid4

ROOT = Path(__file__).resolve().parents[1]
RUN_DIR = ROOT / "builds" / "benchmark_runs"
REFERENCE = ROOT / "builds" / "serial.ppm"

EXECUTABLES = {
    "serial": "serial_timed",
    "openmp": "openmp_timed",
    "mpi": "mpi_timed",
    "hybrid": "hybrid_timed",
}

COLUMNS = [
    "run_id", "trial", "scene", "mode",
    "ranks", "threads", "workers",
    "width", "spp", "max_depth",
    "setup_s", "camera_init_s", "render_s",
    "gather_s", "output_s",
    "camera_aggregate", "scene_aggregate",
    "application_rank0_s", "application_rank_max_s",
    "external_wall_s",
    "image_path", "log_path",
]


def parse_timings(log_text, expected_ranks):
    components = {}
    application_times = {}

    for line in log_text.splitlines():
        if not line.startswith("RT_TIMING "):
            continue

        fields = dict(
            item.split("=", 1)
            for item in line.split()[1:]
        )

        component = fields["component"]

        if component == "application":
            rank = int(fields["rank"])
            if rank in application_times:
                raise ValueError(f"Duplicate application record: rank {rank}")
            application_times[rank] = float(fields["total_s"])

        elif component in ("camera", "scene"):
            if component in components:
                raise ValueError(f"Duplicate {component} record")
            components[component] = fields

    if "camera" not in components or "scene" not in components:
        raise ValueError("Missing camera or scene timing record")

    if set(application_times) != set(range(expected_ranks)):
        raise ValueError("Missing or unexpected MPI rank timing records")

    return components["camera"], components["scene"], application_times


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--mode", required=True, choices=EXECUTABLES)
    parser.add_argument("--ranks", type=int, default=1)
    parser.add_argument("--threads", type=int, default=1)
    parser.add_argument("--trial", type=int, default=1)
    args = parser.parse_args()

    if args.ranks < 1 or args.threads < 1 or args.trial < 1:
        parser.error("Ranks, threads and trial must be positive")

    if args.mode in ("serial", "openmp") and args.ranks != 1:
        parser.error("Serial/OpenMP must use one process")

    if args.mode in ("serial", "mpi") and args.threads != 1:
        parser.error("Serial/MPI must use one thread per process")

    executable = ROOT / "builds" / EXECUTABLES[args.mode]

    if not executable.is_file():
        parser.error(f"Executable not found: {executable}")

    if not REFERENCE.is_file():
        parser.error(f"Reference image not found: {REFERENCE}")

    RUN_DIR.mkdir(parents=True, exist_ok=True)

    run_id = (
        datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%S")
        + "_" + uuid4().hex[:8]
    )

    image_path = RUN_DIR / f"{run_id}.ppm"
    log_path = RUN_DIR / f"{run_id}.log"
    csv_path = RUN_DIR / "quick_results.csv"

    command = [str(executable)]

    if args.mode in ("mpi", "hybrid"):
        command = ["mpirun", "-np", str(args.ranks)] + command

    env = os.environ.copy()
    env["OMP_NUM_THREADS"] = str(args.threads)
    env["OMP_DYNAMIC"] = "FALSE"
    env["OMP_SCHEDULE"] = "static"

    start = time.perf_counter()

    with image_path.open("wb") as image_file:
        result = subprocess.run(
            command,
            cwd=ROOT,
            env=env,
            stdout=image_file,
            stderr=subprocess.PIPE,
            text=True,
        )

    external_wall_s = time.perf_counter() - start
    log_path.write_text(result.stderr, encoding="utf-8")

    if result.returncode != 0:
        raise RuntimeError(
            f"Renderer exited with code {result.returncode}. "
            f"Inspect {log_path}"
        )

    if not filecmp.cmp(REFERENCE, image_path, shallow=False):
        raise RuntimeError(
            f"Image differs from serial reference: {image_path}"
        )

    camera, scene, app_times = parse_timings(
        result.stderr, args.ranks
    )

    # These quick-test configurations are fixed in the C++ drivers.
    if scene["scene"] != "A":
        raise ValueError("Expected Benchmark A quick-test scene")

    record = {
        "run_id": run_id,
        "trial": args.trial,
        "scene": scene["scene"],
        "mode": args.mode,
        "ranks": args.ranks,
        "threads": args.threads,
        "workers": args.ranks * args.threads,
        "width": 160,
        "spp": 4,
        "max_depth": 30,
        "setup_s": float(scene["setup_s"]),
        "camera_init_s": float(camera["camera_init_s"]),
        "render_s": float(camera["render_s"]),
        "gather_s": float(camera["gather_s"]),
        "output_s": float(camera["output_s"]),
        "camera_aggregate": camera["aggregate"],
        "scene_aggregate": scene["aggregate"],
        "application_rank0_s": app_times[0],
        "application_rank_max_s": max(app_times.values()),
        "external_wall_s": external_wall_s,
        "image_path": str(image_path.relative_to(ROOT)),
        "log_path": str(log_path.relative_to(ROOT)),
    }

    new_file = not csv_path.exists()

    with csv_path.open("a", newline="", encoding="utf-8") as file:
        writer = csv.DictWriter(file, fieldnames=COLUMNS)
        if new_file:
            writer.writeheader()
        writer.writerow(record)

    print(f"Mode: {args.mode}")
    print("Image correctness: PASS")
    print(f"Rendering: {record['render_s']:.6f} s")
    print(f"External wall time: {external_wall_s:.6f} s")
    print(f"CSV: {csv_path}")


if __name__ == "__main__":
    main()
