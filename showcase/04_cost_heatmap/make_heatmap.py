#!/usr/bin/env python3
"""Turn the cost_heatmap output into figures.

Usage: python3 make_heatmap.py output/A

Reads   output/A.png (the render), output/A_rays.u32, output/A_meta.txt, output/A_rows.csv
Writes  output/A_heatmap.png        rays traced per pixel, with a colour bar
        output/A_side_by_side.png   render next to the heatmap
        output/A_row_blocks.png     share of the work in each MPI row block (2, 4, 8 ranks)
        output/A_row_blocks.csv     the numbers behind that chart
"""
import csv
import sys
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.colors import LogNorm
from matplotlib.ticker import FixedLocator, ScalarFormatter

plt.rcParams.update({
    "font.family": "serif",
    "font.serif": ["Times New Roman", "Liberation Serif", "DejaVu Serif"],
    "font.size": 10,
    "savefig.dpi": 200,
    "savefig.bbox": "tight",
})

BLUE, ORANGE = "#257de0", "#f47230"


def blocks(n_rows, ranks):
    """Row blocks exactly as Camera::renderImageMPI assigns them."""
    base, rem = divmod(n_rows, ranks)
    out = []
    for r in range(ranks):
        start = r * base + min(r, rem)
        out.append((start, start + base + (1 if r < rem else 0)))
    return out


def nice_ticks(bar, norm):
    """Plain-number ticks (1, 2, 5, 10, ...) instead of 2x10^0 style labels."""
    ticks = [t for t in (1, 1.5, 2, 3, 5, 7, 10, 15, 20, 30, 50, 100) if norm.vmin <= t <= norm.vmax]
    bar.ax.yaxis.set_major_locator(FixedLocator(ticks))
    bar.ax.yaxis.set_major_formatter(ScalarFormatter())
    bar.ax.yaxis.set_minor_locator(FixedLocator([]))


def main():
    if len(sys.argv) != 2:
        sys.exit(__doc__)
    stem = Path(sys.argv[1])
    width, height, spp = (int(v) for v in (stem.parent / f"{stem.name}_meta.txt").read_text().split())
    rays = np.fromfile(stem.parent / f"{stem.name}_rays.u32", dtype=np.uint32).reshape(height, width)
    rows = list(csv.DictReader(open(stem.parent / f"{stem.name}_rows.csv")))
    row_time = np.array([float(r["render_s"]) for r in rows])
    row_rays = np.array([int(r["rays"]) for r in rows], dtype=float)

    # Heatmap of rays per pixel, shown per sample so it reads the same at any spp.
    per_sample = rays / spp
    fig, ax = plt.subplots(figsize=(7.5, 4.6))
    norm = LogNorm(vmin=1.0, vmax=max(2.0, float(np.percentile(per_sample, 99.5))))
    im = ax.imshow(per_sample, cmap="inferno", norm=norm, interpolation="nearest")
    ax.set_axis_off()
    bar = fig.colorbar(im, ax=ax, fraction=0.03, pad=0.02)
    bar.set_label("Rays traced per camera sample (log scale)")
    nice_ticks(bar, norm)
    fig.savefig(stem.parent / f"{stem.name}_heatmap.png")
    plt.close(fig)

    render_png = stem.parent / f"{stem.name}.png"
    if render_png.exists():
        fig, axes = plt.subplots(1, 2, figsize=(11, 3.4))
        axes[0].imshow(plt.imread(render_png))
        axes[0].set_title("Rendered image")
        im = axes[1].imshow(per_sample, cmap="inferno", norm=norm, interpolation="nearest")
        axes[1].set_title("Rays traced per camera sample (log scale)")
        for a in axes:
            a.set_axis_off()
        nice_ticks(fig.colorbar(im, ax=axes[1], fraction=0.03, pad=0.02), norm)
        fig.savefig(stem.parent / f"{stem.name}_side_by_side.png")
        plt.close(fig)

    # Share of the work in each fixed row block, as MPI would assign them.
    table = []
    fig, axes = plt.subplots(1, 3, figsize=(11, 3.2), sharey=True)
    for ax, ranks in zip(axes, (2, 4, 8)):
        b = blocks(height, ranks)
        t_share = np.array([row_time[s:e].sum() for s, e in b]) / row_time.sum() * 100
        r_share = np.array([row_rays[s:e].sum() for s, e in b]) / row_rays.sum() * 100
        x = np.arange(ranks)
        ax.bar(x - 0.2, t_share, 0.38, color=BLUE, label="Share of render time")
        ax.bar(x + 0.2, r_share, 0.38, color=ORANGE, hatch="////", edgecolor="white", label="Share of rays")
        ax.axhline(100 / ranks, ls="--", color="#777777", lw=1, label="Equal share")
        ax.set_xticks(x, [str(i) for i in x])
        ax.set_xlabel(f"Rank ({ranks} ranks)")
        cap = 100 / t_share.max()
        ax.set_title(f"{ranks} ranks: best possible speedup {cap:.2f}×")
        for i, (s, e) in enumerate(b):
            table.append({
                "ranks": ranks, "rank": i, "first_row": s, "last_row": e - 1,
                "time_share_pct": round(t_share[i], 2), "ray_share_pct": round(r_share[i], 2),
            })
    axes[0].set_ylabel("Share of total (%)")
    axes[0].legend(frameon=False, fontsize=8)
    fig.savefig(stem.parent / f"{stem.name}_row_blocks.png")
    plt.close(fig)

    with open(stem.parent / f"{stem.name}_row_blocks.csv", "w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=list(table[0]))
        w.writeheader()
        w.writerows(table)

    print(f"Rays per camera sample: min {per_sample.min():.2f}, mean {per_sample.mean():.2f}, max {per_sample.max():.2f}")
    for ranks in (2, 4, 8):
        b = blocks(height, ranks)
        shares = [row_time[s:e].sum() / row_time.sum() for s, e in b]
        print(f"{ranks} ranks: slowest block has {max(shares) * 100:.1f}% of the work "
              f"-> best possible speedup with fixed blocks {1 / max(shares):.2f}x")
    print(f"Figures written next to {stem}")


if __name__ == "__main__":
    main()
