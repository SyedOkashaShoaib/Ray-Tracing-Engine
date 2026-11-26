#!/usr/bin/env bash
# Renders the showcase scenes one after another, unattended.
#
# Between renders it pauses for COOLDOWN seconds and then waits until the CPU
# package temperature is below RESUME_TEMP (when it can read it), so the
# laptop never starts a long render while already hot. Each scene's output
# goes to its own output/ folder; progress is logged to showcase/overnight.log.
# If one scene fails, the rest still run.
#
# Settings (environment variables):
#   SCENES       folders to render (default: the eight "wow" scenes, 10 to 17)
#   COOLDOWN     pause between renders in seconds (default 120)
#   RESUME_TEMP  wait until the CPU is at or below this many degrees C (default 60)
#   THREADS      OpenMP threads (default: all logical CPUs); 6 runs cooler
#   WIDTH / SPP  if set, override every scene's own defaults
#
# Example:  THREADS=6 bash showcase/render_overnight.sh
set -uo pipefail

SHOWCASE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LOG="$SHOWCASE_DIR/overnight.log"
SCENES="${SCENES:-10_galaxy 16_dna_helix 14_glowing_marbles 11_chrome_ocean 15_snow_globe 13_golden_spiral 12_ringed_planet 17_apollonian_packing}"
COOLDOWN="${COOLDOWN:-120}"
RESUME_TEMP="${RESUME_TEMP:-60}"

log() { echo "[$(date '+%H:%M:%S')] $*" | tee -a "$LOG"; }

# CPU package temperature in whole degrees C, or empty if it cannot be read.
cpu_temp() {
    local zone
    for zone in /sys/class/thermal/thermal_zone*; do
        if [[ "$(cat "$zone/type" 2>/dev/null)" == "x86_pkg_temp" ]]; then
            echo $(( $(cat "$zone/temp") / 1000 ))
            return
        fi
    done
    if command -v sensors >/dev/null 2>&1; then
        sensors 2>/dev/null | awk '/^Package id 0:/ { gsub(/[+°C]/, "", $4); printf "%d\n", $4; exit }'
    fi
}

wait_until_cool() {
    local t
    t="$(cpu_temp)"
    if [[ -z "$t" ]]; then
        log "CPU temperature not readable; cooling down for ${COOLDOWN}s only"
        sleep "$COOLDOWN"
        return
    fi
    log "Cooling down for ${COOLDOWN}s (CPU ${t}°C)"
    sleep "$COOLDOWN"
    while t="$(cpu_temp)"; [[ -n "$t" && "$t" -gt "$RESUME_TEMP" ]]; do
        log "CPU at ${t}°C, waiting until ≤ ${RESUME_TEMP}°C"
        sleep 30
    done
}

log "=== Overnight showcase run: $SCENES"
first=1
for scene in $SCENES; do
    script="$SHOWCASE_DIR/$scene/render.sh"
    if [[ ! -f "$script" ]]; then
        log "Skipping $scene (no render.sh)"
        continue
    fi
    (( first )) || wait_until_cool
    first=0
    log "Starting $scene"
    start=$(date +%s)
    if bash "$script" >>"$LOG" 2>&1; then
        log "Finished $scene in $(( ($(date +%s) - start) / 60 )) min"
    else
        log "FAILED $scene after $(( ($(date +%s) - start) / 60 )) min (see log above); continuing"
    fi
done
log "=== All done"
