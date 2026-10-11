#!/usr/bin/env bash
# Download and extract pinned EncDotNet.S100 s100 CLI (linux-x64) into tools/encdotnet-s100/.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
DEST="${ENCDOTNET_DEST:-$ROOT/tools/encdotnet-s100}"
VERSION="0.24.0"
ASSET="s100-${VERSION}-linux-x64.tar.gz"
URL="https://github.com/philliphoff/EncDotNet.S100/releases/download/v${VERSION}/${ASSET}"
SHA256="9ea3d351aa071d4ae2fcbb5d545e00d318128ade8551f8b016dde2302e5f3416"

mkdir -p "$DEST"
CACHE_DIR="${ENCDOTNET_CACHE:-$DEST}"
TARBALL="$CACHE_DIR/$ASSET"

need_fetch=1
if [[ -f "$TARBALL" ]]; then
  got="$(sha256sum "$TARBALL" | awk '{print $1}')"
  if [[ "$got" == "$SHA256" ]]; then
    need_fetch=0
    echo "cache hit: $TARBALL"
  else
    echo "cache checksum mismatch (got $got), re-fetching" >&2
    rm -f "$TARBALL"
  fi
fi

if [[ "$need_fetch" -eq 1 ]]; then
  echo "fetching $URL"
  tmp="$(mktemp "$CACHE_DIR/.${ASSET}.XXXXXX")"
  # Prefer curl; fall back to wget.
  if command -v curl >/dev/null 2>&1; then
    curl -fL --retry 3 -o "$tmp" "$URL"
  else
    wget -O "$tmp" "$URL"
  fi
  got="$(sha256sum "$tmp" | awk '{print $1}')"
  if [[ "$got" != "$SHA256" ]]; then
    rm -f "$tmp"
    echo "SHA-256 mismatch: expected $SHA256 got $got" >&2
    exit 1
  fi
  mv "$tmp" "$TARBALL"
  echo "verified $TARBALL"
fi

# Extract in place (tarball has s100 + DLLs at top level).
if [[ ! -x "$DEST/s100" ]]; then
  echo "extracting into $DEST"
  tar -xzf "$TARBALL" -C "$DEST"
fi

ver="$("$DEST/s100" --version 2>&1 | head -1 || true)"
echo "s100 ready: $DEST/s100 ($ver)"
echo "ENCDOTNET_S100=$DEST/s100"
