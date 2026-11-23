
import argparse
import os
import shutil
import subprocess
import sys
import tempfile
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "src"
BUILD = ROOT / "build"
EXE = BUILD / ("sphereflake.exe" if os.name == "nt" else "sphereflake")
STYLES = ["rainbow", "brass", "gold", "chrome", "glass"]


#Build

def find_compiler(user_choice):
    candidates = [user_choice] if user_choice else [os.environ.get("CXX"), "g++", "clang++"]
    for c in candidates:
        if c and shutil.which(c):
            return c
    sys.exit("No C++ compiler found. Install g++ (MinGW-w64 on Windows) or clang++, "
             "or pass --cxx path/to/compiler.")


def build(cxx, force=False):
    stb = SRC / "external" / "stb_image.h"
    if not stb.exists():
        sys.exit(f"Missing {stb}. The engine needs stb_image.h there "
                 "(get it from https://github.com/nothings/stb).")

    sources = list(SRC.rglob("*.h")) + [SRC / "sphereflake_main.cpp"]
    newest_source = max(p.stat().st_mtime for p in sources)
    if not force and EXE.exists() and EXE.stat().st_mtime > newest_source:
        return  

    BUILD.mkdir(exist_ok=True)
    cmd = [cxx, "-std=c++20", "-O3", "-I", str(SRC),
           str(SRC / "sphereflake_main.cpp"), "-o", str(EXE)]
    print("Compiling:", " ".join(cmd))
    t = time.time()
    if subprocess.run(cmd).returncode != 0:
        sys.exit("Compilation failed.")
    print(f"Built in {time.time() - t:.1f}s")



def read_ppm_p3(path):
    tokens = Path(path).read_text().split()
    if tokens[0] != "P3":
        raise ValueError(f"{path} is not a P3 PPM")
    width, height = int(tokens[1]), int(tokens[2])
    values = list(map(int, tokens[4:4 + width * height * 3]))
    return width, height, values


def srgb_to_linear(c):
    return c / 12.92 if c <= 0.04045 else ((c + 0.055) / 1.055) ** 2.4


def linear_to_srgb(c):
    c = min(max(c, 0.0), 1.0)
    return 12.92 * c if c <= 0.0031308 else 1.055 * c ** (1 / 2.4) - 0.055


# The engine writes byte = int(256 * srgb); decode each byte at the middle of its bucket.
DECODE = [srgb_to_linear((b + 0.5) / 256) for b in range(256)]


def merge_renders(paths, weights):
    """Weighted average of several renders, done in linear light."""
    total = sum(weights)
    width = height = None
    acc = None
    for path, w in zip(paths, weights):
        wd, ht, vals = read_ppm_p3(path)
        if acc is None:
            width, height, acc = wd, ht, [0.0] * len(vals)
        scale = w / total
        for i, v in enumerate(vals):
            acc[i] += DECODE[v] * scale
    out = bytes(min(255, int(256 * linear_to_srgb(v))) for v in acc)
    return width, height, out


def save_image(width, height, rgb_bytes, out_path):
    out_path = Path(out_path)
    out_path.parent.mkdir(parents=True, exist_ok=True)
    if out_path.suffix.lower() != ".ppm":
        try:
            from PIL import Image
            Image.frombytes("RGB", (width, height), rgb_bytes).save(out_path)
            return out_path
        except ImportError:
            out_path = out_path.with_suffix(".ppm")
            print("Pillow not installed (pip install pillow) - saving PPM instead.")
    with open(out_path, "wb") as f:
        f.write(f"P6\n{width} {height}\n255\n".encode())
        f.write(rgb_bytes)
    return out_path



# Render

def render(args):
    jobs = max(1, min(args.jobs, args.spp))
  
    shares = [args.spp // jobs + (1 if i < args.spp % jobs else 0) for i in range(jobs)]

    spheres = sum(9 ** k for k in range(args.levels + 1))
    print(f"Rendering {spheres:,} spheres at {args.width}px, {args.spp} spp "
          f"on {jobs} process(es)...")

    tmp = Path(tempfile.mkdtemp(prefix="sphereflake_"))
    procs, files = [], []
    t = time.time()
    try:
        for i, spp in enumerate(shares):
            cmd = [str(EXE), "--levels", str(args.levels), "--scale", str(args.scale),
                   "--style", args.style, "--width", str(args.width), "--spp", str(spp),
                   "--depth", str(args.depth), "--seed", str(1000 + 7919 * i)]
            if args.plain_floor:
                cmd.append("--plain-floor")
            ppm = tmp / f"part_{i}.ppm"
            files.append(ppm)
            out = open(ppm, "w")
            # Only the first process shows its progress, otherwise the lines get mixed up.
            err = None if i == 0 else subprocess.DEVNULL
            procs.append((subprocess.Popen(cmd, stdout=out, stderr=err, cwd=ROOT), out))

        failed = False
        for p, out in procs:
            failed |= p.wait() != 0
            out.close()
        if failed:
            sys.exit("A render process failed.")
        print(f"Rendered in {time.time() - t:.1f}s, merging...")

        width, height, rgb = merge_renders(files, shares)
        saved = save_image(width, height, rgb, args.out)
        print(f"Saved {saved}")
    finally:
        shutil.rmtree(tmp, ignore_errors=True)


def main():
    ap = argparse.ArgumentParser(description="Render a sphereflake with the ray tracing engine.")
    ap.add_argument("--levels", type=int, default=3, help="recursion depth 0-6 (default 3)")
    ap.add_argument("--scale", type=float, default=1 / 3, help="child/parent radius ratio (default 1/3)")
    ap.add_argument("--style", choices=STYLES, default="rainbow", help="material style")
    ap.add_argument("--width", type=int, default=800, help="image width in pixels (16:9)")
    ap.add_argument("--spp", type=int, default=64, help="total samples per pixel")
    ap.add_argument("--depth", type=int, default=30, help="max ray bounces")
    ap.add_argument("--plain-floor", action="store_true", help="dark floor instead of checkerboard")
    ap.add_argument("--jobs", type=int, default=os.cpu_count() or 1, help="parallel render processes")
    ap.add_argument("--out", default=None, help="output file (.png or .ppm)")
    ap.add_argument("--cxx", default=None, help="C++ compiler to use")
    ap.add_argument("--rebuild", action="store_true", help="force recompiling")
    ap.add_argument("--preview", action="store_true", help="quick low-quality render")
    args = ap.parse_args()

    if args.preview:
        args.width, args.spp, args.depth = 400, 16, 10
    if not 0 <= args.levels <= 6:
        ap.error("--levels must be between 0 and 6")
    if args.levels >= 5:
        print("Heads up: level 5+ means 66k+ spheres - expect long build/render times.")
    if args.out is None:
        args.out = f"renders/sphereflake_{args.style}_L{args.levels}.png"
    args.out = str((Path.cwd() / args.out).resolve())

    build(find_compiler(args.cxx), force=args.rebuild)
    render(args)


if __name__ == "__main__":
    main()
