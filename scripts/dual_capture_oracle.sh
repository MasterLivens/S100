#!/usr/bin/env bash
# Dual capture: our s100_viewer --snapshot + EncDotNet render, then warn-only soft compare.
# Does not touch CI golden / S-164 refs. Failures in soft-compare are WARN-only.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
OUT="${ORACLE_OUT:-$ROOT/Testing/encdotnet-oracle}"
W="${ORACLE_WIDTH:-1280}"
H="${ORACLE_HEIGHT:-800}"
VIEWER="${S100_VIEWER:-$ROOT/build/s100_viewer}"
THRESH="${ORACLE_THRESHOLD:-0.90}"

mkdir -p "$OUT"

echo "== EncDotNet oracle renders =="
"$ROOT/scripts/render_encdotnet_oracle.sh"

if [[ ! -x "$VIEWER" ]]; then
  echo "WARN: s100_viewer not found at $VIEWER — skip dual capture / soft compare"
  exit 0
fi

IHO="$ROOT/testdata/s101/101AA00DS0008.000"
OURS_IHO="$OUT/ours_iho_0008_day_${W}x${H}.png"
ENC_IHO="$OUT/enc_iho_0008_day_${W}x${H}.png"

echo "== our viewer snapshot (IHO 0008 Day) =="
QT_QPA_PLATFORM=offscreen QT_OPENGL=software \
  "$VIEWER" "$IHO" --palette Day --snapshot "$OURS_IHO"

# Optional CHS / S-102 if present
CHS="$ROOT/testdata/chs/COMBINED/S100_ROOT/S-101/DATASET_FILES/CA00/101CA00P468N0712W/2/101CA00P468N0712W.000"
if [[ -f "$CHS" ]]; then
  OURS_CHS="$OUT/ours_chs_101CA00P468N0712W_day_${W}x${H}.png"
  echo "== our viewer snapshot (CHS) =="
  QT_QPA_PLATFORM=offscreen QT_OPENGL=software \
    "$VIEWER" "$CHS" --palette Day --snapshot "$OURS_CHS" || echo "WARN: CHS viewer snapshot failed"
fi

S102="$ROOT/testdata/s102/102US004AK5HQ262267.h5"
if [[ -f "$S102" ]]; then
  OURS_S102="$OUT/ours_s102_102US004AK5HQ262267_day_${W}x${H}.png"
  echo "== our viewer snapshot (S-102) =="
  QT_QPA_PLATFORM=offscreen QT_OPENGL=software \
    "$VIEWER" "$S102" --palette Day --snapshot "$OURS_S102" || \
    QT_QPA_PLATFORM=offscreen QT_OPENGL=software \
      "$VIEWER" "$IHO" --s102 "$S102" --palette Day --snapshot "$OURS_S102" || \
    echo "WARN: S-102 viewer snapshot failed (viewer may need --s102 with an S-101 base)"
fi

echo "== soft compare (warn-only) =="
if [[ -f "$OURS_IHO" && -f "$ENC_IHO" ]]; then
  python3 "$ROOT/scripts/soft_compare_oracle.py" "$OURS_IHO" "$ENC_IHO" --threshold "$THRESH" || true
else
  echo "WARN: missing IHO pair for soft compare"
fi

if [[ -f "${OURS_CHS:-}" && -f "$OUT/enc_chs_101CA00P468N0712W_day_${W}x${H}.png" ]]; then
  python3 "$ROOT/scripts/soft_compare_oracle.py" "$OURS_CHS" \
    "$OUT/enc_chs_101CA00P468N0712W_day_${W}x${H}.png" --threshold "$THRESH" || true
fi

echo "artifacts in $OUT"
ls -la "$OUT"
