#!/usr/bin/env python3
"""Validated, repeatable runner for the ray tracing engine benchmarks.
"""

import argparse
import csv
import hashlib
import math
import os
import shutil
import signal
import subprocess
import time
from datetime import datetime, timezone
from pathlib import Path
from uuid import uuid4

ROOT = Path(__file__).resolve().parents[1]
RESULTS = ROOT / "builds" / "benchmark_runs"
PROFILES = {
    "smoke": {
        "prefix": "smoke",
        "width": 160,
        "spp": 4,
        "timeout": 120,
    },
    "calibration": {
        "prefix": "calibration",
        "width": 640,
        "spp": 16,
        "timeout": 900,
    },
    "formal": {
        "prefix": "benchmark",
        "width": 1280,
        "spp": 128,
        "timeout": 7200,
    },
}
MODES = ("serial", "openmp", "mpi", "hybrid")
FIELDS = [
    "run_id", "profile", "trial", "scene", "mode", "ranks", "threads",
    "workers", "omp_schedule", "width", "height", "spp", "max_depth",
    "sphereflake_levels", "setup_s", "camera_init_s", "render_s", "gather_s",
    "output_s", "scene_aggregate", "camera_aggregate",
    "application_rank0_s", "application_rank_max_s", "external_wall_s",
    "image_sha256", "image_path", "log_path",
]


def require_positive(name, value):
    if value <= 0:
        raise ValueError(f"{name} must be positive")


def parse_timings(log_text, expected_scene, expected_ranks, mpi_mode):
    camera = None
    scene = None
    application = {}

    for line in log_text.splitlines():
        if not line.startswith("RT_TIMING "):
            continue
        fields = {}
        for token in line.split()[1:]:
            if token.count("=") != 1:
                raise ValueError(f"Malformed timing token: {token!r}")
            key, value = token.split("=", 1)
            if not key or not value or key in fields:
                raise ValueError(f"Invalid timing token: {token!r}")
            fields[key] = value

        component = fields.get("component")
        if component == "camera":
            if camera is not None:
                raise ValueError("Duplicate camera timing record")
            camera = fields
        elif component == "scene":
            if scene is not None:
                raise ValueError("Duplicate scene timing record")
            scene = fields
        elif component == "application":
            rank = int(fields["rank"])
            if rank in application:
                raise ValueError(f"Duplicate application timing for rank {rank}")
            application[rank] = float(fields["total_s"])
        else:
            raise ValueError(f"Unknown RT_TIMING component: {component!r}")

    if camera is None or scene is None:
        raise ValueError("Missing camera/scene timing record")
    if scene.get("scene") != expected_scene:
        raise ValueError(f"Unexpected scene label: {scene.get('scene')}")
    if set(application) != set(range(expected_ranks)):
        raise ValueError(f"Expected application records for ranks 0..{expected_ranks - 1}")

    expected_aggregate = "rank_max" if mpi_mode else "single_process"
    if camera.get("aggregate") != expected_aggregate or scene.get("aggregate") != expected_aggregate:
        raise ValueError("Unexpected timing aggregation metadata")

    for name in ("camera_init_s", "render_s", "gather_s", "output_s"):
        value = float(camera[name])
        if not math.isfinite(value) or value < 0:
            raise ValueError(f"Invalid {name}: {value}")
    setup = float(scene["setup_s"])
    if not math.isfinite(setup) or setup < 0:
        raise ValueError("Invalid scene setup measurement")
    if any(not math.isfinite(v) or v < 0 for v in application.values()):
        raise ValueError("Invalid application duration")

    return camera, scene, application


def validate_ppm(path, width, height):
    """Validate our exact P3 writer format, including every RGB component."""
    count = 0
    with path.open("rb") as file:
        if file.readline().strip() != b"P3":
            raise ValueError("PPM must begin with P3")
        if file.readline().strip() != f"{width} {height}".encode():
            raise ValueError("Unexpected PPM dimensions")
        if file.readline().strip() != b"255":
            raise ValueError("Unexpected PPM maximum channel value")

        for line in file:
            pixel = line.split()
            if len(pixel) != 3:
                raise ValueError(f"Malformed PPM at pixel line {count + 4}")
            if any(not component.isdigit() or int(component) > 255 for component in pixel):
                raise ValueError(f"Invalid PPM component at pixel line {count + 4}")
            count += 1

    if count != width * height:
        raise ValueError(f"PPM has {count} pixels, expected {width * height}")


def sha256_file(path):
    digest = hashlib.sha256()
    with path.open("rb") as file:
        for chunk in iter(lambda: file.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def run_with_timeout(command, env, stdout_path, log_path, timeout_seconds):
    # One subprocess group so an expired timeout also stops the MPI workers.
    start = time.perf_counter()
    with stdout_path.open("wb") as out, log_path.open("wb") as err:
        process = subprocess.Popen(
            command,
            cwd=ROOT,
            env=env,
            stdin=subprocess.DEVNULL,
            stdout=out,
            stderr=err,
            start_new_session=True,
        )
        try:
            returncode = process.wait(timeout=timeout_seconds)
        except subprocess.TimeoutExpired:
            os.killpg(process.pid, signal.SIGTERM)
            try:
                process.wait(timeout=5)
            except subprocess.TimeoutExpired:
                os.killpg(process.pid, signal.SIGKILL)
                process.wait()
            raise TimeoutError(f"Exceeded {timeout_seconds:g} seconds")
    return returncode, time.perf_counter() - start


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--profile", choices=PROFILES, default="smoke")
    parser.add_argument("--scene", required=True, choices=("A", "B"))
    parser.add_argument("--mode", required=True, choices=MODES)
    parser.add_argument("--ranks", type=int, default=1)
    parser.add_argument("--threads", type=int, default=1)
    parser.add_argument("--schedule", default="static", choices=("static", "dynamic,1", "guided"))
    parser.add_argument("--trials", type=int, default=1)
    parser.add_argument("--trial-start", type=int, default=1)
    parser.add_argument("--timeout-s", type=float, default=None)
    parser.add_argument("--keep-images", action="store_true")
    args = parser.parse_args()

    try:
        for name in ("ranks", "threads", "trials", "trial_start"):
            require_positive(name, getattr(args, name))
        if args.mode in ("serial", "openmp") and args.ranks != 1:
            parser.error("Serial/OpenMP use exactly one process")
        if args.mode in ("serial", "mpi") and args.threads != 1:
            parser.error("Serial/MPI use exactly one rendering thread per process")
    except ValueError as error:
        parser.error(str(error))

    profile = PROFILES[args.profile]
    timeout_s = args.timeout_s if args.timeout_s is not None else profile["timeout"]
    if not math.isfinite(timeout_s) or timeout_s <= 0:
        parser.error("Timeout must be a positive number of seconds")

    executable = ROOT / "builds" / f"{profile['prefix']}_{args.scene}_{args.mode}"
    if not executable.is_file():
        parser.error(f"Missing executable: {executable}")

    references = RESULTS / "references"
    reference = references / f"{args.profile}_{args.scene}.ppm"
    if args.mode != "serial" and not reference.is_file():
        parser.error(
            f"No serial reference at {reference}. "
            f"Run --profile {args.profile} --scene {args.scene} --mode serial first."
        )

    width, spp = profile["width"], profile["spp"]
    height = int(width / (16.0 / 9.0))
    command = [str(executable)]

    if args.mode in ("mpi", "hybrid"):
        mpi_options = []

        if args.mode == "hybrid":
            total_workers = args.ranks * args.threads

            if total_workers not in (4, 8):
                parser.error("Hybrid scaling requires exactly 4 or 8 total workers")

            if total_workers == 8:
                # Use the eight logical processors (SMT hardware threads).
                mpi_options.append("--use-hwthread-cpus")
                binding_target = "hwthread"
            else:
                # Use the four physical cores.
                binding_target = "core"

            mpi_options.extend([
                "--map-by", f"slot:PE={args.threads}",
                "--bind-to", binding_target,
            ])

        elif args.ranks > 4:
            # Preserve the MPI-only policy used in the completed scaling runs.
            mpi_options.append("--use-hwthread-cpus")

        command = [
            "mpirun",
            *mpi_options,
            "-np",
            str(args.ranks),
            *command,
        ]

    env = os.environ.copy()
    env["OMP_NUM_THREADS"] = str(args.threads)
    env["OMP_DYNAMIC"] = "FALSE"
    env["OMP_SCHEDULE"] = args.schedule

    if args.mode == "hybrid":
        env["OMP_PROC_BIND"] = "spread"
        env["OMP_PLACES"] = "cores" if total_workers == 4 else "threads"

    RESULTS.mkdir(parents=True, exist_ok=True)
    references.mkdir(exist_ok=True)
    runs = RESULTS / "runs"
    runs.mkdir(exist_ok=True)
    csv_path = RESULTS / "results.csv"

    for trial in range(args.trial_start, args.trial_start + args.trials):
        run_id = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%S") + "_" + uuid4().hex[:8]
        image_path = runs / f"{run_id}.ppm"
        log_path = runs / f"{run_id}.log"
        stdout_path = runs / f"{run_id}.stdout.log"
        rank_log_paths = [runs / f"{run_id}_rank{rank}.log" for rank in range(args.ranks)]
        run_env = env.copy()
        run_env["RT_IMAGE_PATH"] = str(image_path)
        run_env["RT_TIMING_PREFIX"] = str(runs / run_id)
        print(
            f"Running {args.profile} scene={args.scene} mode={args.mode} "
            f"ranks={args.ranks} threads={args.threads} trial={trial}",
            flush=True,
        )
        try:
            returncode, external_s = run_with_timeout(
                command, run_env, stdout_path, log_path, timeout_s
            )
            if returncode != 0:
                raise RuntimeError(f"Renderer exited with status {returncode}")

            # Structured logs are written directly by each rank, bypassing
            # stdout/stderr forwarding through the MPI launcher.
            # Keep the launcher's stderr in log_path for diagnosing failures.
            rank_log_texts = []
            for rank, rank_log_path in enumerate(rank_log_paths):
                if not rank_log_path.is_file():
                    raise ValueError(f"Missing direct timing log for rank {rank}: {rank_log_path}")
                rank_log_texts.append(rank_log_path.read_text(encoding="utf-8", errors="replace"))

            with log_path.open("a", encoding="utf-8") as combined_log:
                for rank, rank_log in enumerate(rank_log_texts):
                    combined_log.write(f"\n=== DIRECT TIMING RANK {rank} ===\n")
                    combined_log.write(rank_log)
                    if not rank_log.endswith("\n"):
                        combined_log.write("\n")

            if stdout_path.stat().st_size:
                raise ValueError(f"Unexpected renderer stdout; inspect {stdout_path}")

            log = "\n".join(rank_log_texts)
            camera, scene, application = parse_timings(
                log, args.scene, args.ranks, args.mode in ("mpi", "hybrid")
            )
            validate_ppm(image_path, width, height)
            image_hash = sha256_file(image_path)

            if reference.is_file():
                if image_hash != sha256_file(reference):
                    raise ValueError("Image bytes differ from the serial reference")
            elif args.mode == "serial":
                shutil.copyfile(image_path, reference)
            else:
                raise ValueError("Reference missing")

            record = {
                "run_id": run_id,
                "profile": args.profile,
                "trial": trial,
                "scene": args.scene,
                "mode": args.mode,
                "ranks": args.ranks,
                "threads": args.threads,
                "workers": args.ranks * args.threads,
                "omp_schedule": args.schedule if args.mode in ("openmp", "hybrid") else "n/a",
                "width": width,
                "height": height,
                "spp": spp,
                "max_depth": 30,
                "sphereflake_levels": 4 if args.scene == "B" else "n/a",
                "setup_s": float(scene["setup_s"]),
                "camera_init_s": float(camera["camera_init_s"]),
                "render_s": float(camera["render_s"]),
                "gather_s": float(camera["gather_s"]),
                "output_s": float(camera["output_s"]),
                "scene_aggregate": scene["aggregate"],
                "camera_aggregate": camera["aggregate"],
                "application_rank0_s": application[0],
                "application_rank_max_s": max(application.values()),
                "external_wall_s": external_s,
                "image_sha256": image_hash,
                "image_path": str(image_path.relative_to(ROOT)) if args.keep_images else "",
                "log_path": str(log_path.relative_to(ROOT)),
            }
            new_file = not csv_path.exists()
            with csv_path.open("a", newline="", encoding="utf-8") as out:
                writer = csv.DictWriter(out, fieldnames=FIELDS)
                if new_file:
                    writer.writeheader()
                writer.writerow(record)

            if not args.keep_images:
                image_path.unlink()
            # Keep the combined launcher+timing log; remove the per-rank
            # intermediate files and empty stdout capture after success.
            for path in rank_log_paths:
                path.unlink()
            stdout_path.unlink()
            print(
                f"PASS: image matched; render={record['render_s']:.4f}s "
                f"external={external_s:.4f}s; CSV={csv_path}",
                flush=True,
            )
        except Exception as error:
            raise SystemExit(
                f"FAILED: {error}\nArtifacts preserved for inspection:\n"
                f"  {image_path}\n  {log_path}\n"
                "No CSV row was appended for this failed trial."
            ) from error


if __name__ == "__main__":
    main()
