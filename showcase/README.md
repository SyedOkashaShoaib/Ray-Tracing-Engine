# Showcase renders

Extra images that show what the ray tracer can do. They use the engine in
`src/` unchanged: every program includes the engine headers and renders with
the engine's own camera, materials and row loop. The benchmark scenes A and B
are reused exactly as benchmarked, and their output is byte-identical to the
main program's.

| Folder | What it shows | Default settings |
|---|---|---|
| `01_bounce_depth` | Same scene at max depth 1, 2, 3, 5, 10, 30: recursion building the image | demo scene, 960 px, 256 spp |
| `02_spp_convergence` | Same scene at 1–1024 spp, plus 100% crops: Monte Carlo noise falling | Scene A, 960 px |
| `03_rank_partition` | MPI row blocks tinted per rank, plus each rank's own render time (CSV) | Scene A, 4 ranks, 1280 px, 64 spp |
| `04_cost_heatmap` | Rays traced per pixel and render time per row; work share per MPI block | Scene A, 1280 px, 32 spp |
| `05_refraction_sweep` | Glass with index 1.00, 1.33, 1.50, 1.90, 2.42 against a checkered wall | 1920 px, 512 spp |
| `06_fuzz_sweep` | Metal with fuzz 0 to 1, from mirror to rough | 1920 px, 512 spp |
| `07_mirror_corridor` | Two parallel mirrors: one copy per bounce | 1920 px, 512 spp |
| `08_colour_bleeding` | Red and green walls tinting white spheres (global illumination) | 1920 px, 512 spp, depth 8 |
| `09_rgb_light_mixing` | Red, green and blue lights giving cyan, magenta and yellow shadows | 1920 px, 1024 spp |

## Requirements

```bash
sudo apt install build-essential imagemagick python3-numpy python3-matplotlib
sudo apt install openmpi-bin libopenmpi-dev     # only for 03_rank_partition
```

## Running

Run any script from anywhere; it builds its program, renders into the
folder's `output/`, and converts to PNG.

```bash
bash showcase/05_refraction_sweep/render.sh
```

Settings are environment variables, listed at the top of each script:

```bash
WIDTH=1280 SPP=64 bash showcase/08_colour_bleeding/render.sh     # quick preview
SCENE=B bash showcase/04_cost_heatmap/render.sh                   # heatmap of Scene B
RANKS=8 bash showcase/03_rank_partition/render.sh                 # one rank per hardware thread
DEPTHS="2 4 8 30" bash showcase/07_mirror_corridor/render.sh      # corridor growing with depth
THREADS=4 bash showcase/06_fuzz_sweep/render.sh                   # use fewer threads (cooler laptop)
```

Each render is wrapped in `systemd-inhibit`, so the laptop will not suspend
while it runs (the screen can still lock). Preview at low settings first:
the room scenes (08, 09) need many samples and take the longest.

## How the programs reach the engine

`common/showcase.h` includes the engine headers with `private` temporarily
defined as `public`, so the programs can call `Camera::initialize()` and
`Camera::renderRowsInitialized()` without changing `src/`. To reuse scenes A
and B, it reads the two scene headers with `Camera` renamed to a subclass
whose `renderImage()` saves the camera and world instead of rendering. This
only affects the showcase programs, not the engine or the benchmark builds.

`03_rank_partition` and `04_cost_heatmap` measure on whatever machine they run
on. Their timings are a separate diagnostic, not part of the formal benchmark
results.
