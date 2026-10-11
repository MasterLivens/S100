#!/usr/bin/env bash
# Render EncDotNet Day-palette oracle PNGs (1280x800) for soft-compare.
# Outputs under Testing/encdotnet-oracle/ (gitignored). Warn-only path — not CI golden.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
OUT="${ORACLE_OUT:-$ROOT/Testing/encdotnet-oracle}"
W="${ORACLE_WIDTH:-1280}"
H="${ORACLE_HEIGHT:-800}"
PALETTE="${ORACLE_PALETTE:-day}"

S100_BIN="${ENCDOTNET_S100:-}"
if [[ -z "$S100_BIN" ]]; then
  if [[ -x "$ROOT/tools/encdotnet-s100/s100" ]]; then
    S100_BIN="$ROOT/tools/encdotnet-s100/s100"
  elif [[ -x /workspace/vendor/encdotnet-s100/s100 ]]; then
    S100_BIN=/workspace/vendor/encdotnet-s100/s100
  fi
fi
if [[ -z "${S100_BIN:-}" || ! -x "$S100_BIN" ]]; then
  echo "s100 CLI not found. Run: ./scripts/fetch_encdotnet_s100.sh" >&2
  exit 1
fi

mkdir -p "$OUT"
echo "using $S100_BIN ($("$S100_BIN" --version 2>&1 | head -1))"
echo "output $OUT (${W}x${H}, palette=$PALETTE)"

render_one() {
  local label="$1" src="$2" dest="$3"
  if [[ ! -f "$src" ]]; then
    echo "SKIP $label (missing $src)"
    return 0
  fi
  echo "RENDER $label -> $dest"
  "$S100_BIN" render "$src" "$dest" --palette "$PALETTE" -w "$W" -h "$H"
}

IHO="$ROOT/testdata/s101/101AA00DS0008.000"
CHS="$ROOT/testdata/chs/COMBINED/S100_ROOT/S-101/DATASET_FILES/CA00/101CA00P468N0712W/2/101CA00P468N0712W.000"
S102="$ROOT/testdata/s102/102US004AK5HQ262267.h5"

render_one "IHO 0008" "$IHO" "$OUT/enc_iho_0008_day_${W}x${H}.png"
render_one "CHS 101CA00P468N0712W" "$CHS" "$OUT/enc_chs_101CA00P468N0712W_day_${W}x${H}.png"
render_one "NOAA S-102" "$S102" "$OUT/enc_s102_102US004AK5HQ262267_day_${W}x${H}.png"

echo "done. PNGs:"
ls -la "$OUT"/*.png 2>/dev/null || echo "(none)"
