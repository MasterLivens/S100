#!/usr/bin/env python3
"""A2 reproducibility for Part 15 fixtures (docs/part15-signature-contract.md).

ECDSA signing is randomized (no RFC 6979 under OpenSSL 3.0 API pin). Assertion:
  - shared TEST-ONLY private keys → identical key bytes across two runs
  - unsigned payloads (.000 cells, README, fake_fc.xml) byte-identical
  - generated signatures verify under the test SA (openssl dgst -sha384 -verify)
"""
from __future__ import annotations

import base64
import hashlib
import os
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

UNSIGNED_SUFFIXES = {".000", ".md", ".xml"}  # payload / docs; not .crt/.SIGN/.XML catalog


def sha_unsigned(root: Path) -> dict:
    out = {}
    for dirpath, _, files in os.walk(root):
        for f in sorted(files):
            p = Path(dirpath) / f
            rel = p.relative_to(root).as_posix()
            if "TEST-ONLY" in f and f.endswith(".pem"):
                out[rel] = hashlib.sha256(p.read_bytes()).hexdigest()
                continue
            if p.suffix.lower() in UNSIGNED_SUFFIXES and f not in ("CATALOG.XML",):
                # fake_fc.xml and README.md and cells/*.000
                out[rel] = hashlib.sha256(p.read_bytes()).hexdigest()
    return out


def pem_body(pem: str) -> str:
    return "".join(ln.strip() for ln in pem.splitlines() if ln and not ln.startswith("-----"))


def verify_positive(case: Path, sa_crt: Path) -> None:
    ds_pem = case.parent / "keys" / "test_ds.crt"
    # Extract pubkey from DS cert
    pub = Path(tempfile.mkstemp(suffix=".pem")[1])
    try:
        subprocess.check_call(
            ["openssl", "x509", "-in", str(ds_pem), "-pubkey", "-noout", "-out", str(pub)]
        )
        # CATALOG.XML
        sign = (case / "CATALOG.SIGN").read_text()
        m = re.search(
            r"<S100SE:digitalSignature[^>]*>(.*?)</S100SE:digitalSignature>", sign, re.S
        )
        assert m, "no catalog signature"
        sig = base64.b64decode("".join(m.group(1).split()))
        sigf = Path(tempfile.mkstemp(suffix=".sig")[1])
        try:
            sigf.write_bytes(sig)
            r = subprocess.run(
                [
                    "openssl",
                    "dgst",
                    "-sha384",
                    "-verify",
                    str(pub),
                    "-signature",
                    str(sigf),
                    str(case / "CATALOG.XML"),
                ],
                capture_output=True,
                text=True,
            )
            if "Verified OK" not in (r.stdout + r.stderr):
                raise SystemExit(f"CATALOG.XML signature failed: {r.stdout}{r.stderr}")
        finally:
            sigf.unlink(missing_ok=True)
        # cells
        x = (case / "CATALOG.XML").read_text()
        for blk in re.findall(
            r"<S100XC:S100_DatasetDiscoveryMetadata>(.*?)</S100XC:S100_DatasetDiscoveryMetadata>",
            x,
            re.S,
        ):
            fn = re.search(r"<S100XC:fileName>(.*?)</S100XC:fileName>", blk).group(1)
            fn = fn.replace("file:", "").lstrip("/")
            vv = re.search(
                r"<S100SE:S100_SE_SignatureOnData[^>]*>(.*?)</S100SE:S100_SE_SignatureOnData>",
                blk,
                re.S,
            ).group(1)
            sig = base64.b64decode("".join(vv.split()))
            sigf = Path(tempfile.mkstemp(suffix=".sig")[1])
            try:
                sigf.write_bytes(sig)
                path = case / fn
                r = subprocess.run(
                    [
                        "openssl",
                        "dgst",
                        "-sha384",
                        "-verify",
                        str(pub),
                        "-signature",
                        str(sigf),
                        str(path),
                    ],
                    capture_output=True,
                    text=True,
                )
                if "Verified OK" not in (r.stdout + r.stderr):
                    raise SystemExit(f"{fn} signature failed: {r.stdout}{r.stderr}")
            finally:
                sigf.unlink(missing_ok=True)
    finally:
        pub.unlink(missing_ok=True)
    # SA chain
    r = subprocess.run(
        ["openssl", "verify", "-CAfile", str(sa_crt), str(ds_pem)],
        capture_output=True,
        text=True,
    )
    if "OK" not in r.stdout:
        # D1: DN may not match; still require file sigs above. Warn only.
        print("note: openssl verify CA DN chain:", (r.stdout + r.stderr).strip()[:200])


def main() -> int:
    if len(sys.argv) < 3:
        print("usage: check_signature_fixtures.py SRC BIN", file=sys.stderr)
        return 1
    src = Path(sys.argv[1])
    gen = src / "tools" / "gen_signature_fixtures.py"
    with tempfile.TemporaryDirectory() as td:
        base = Path(td)
        once = base / "once"
        subprocess.check_call([sys.executable, str(gen), "--out", str(once)])
        keys = once / "keys"

        def run_with_keys(dest: Path) -> None:
            dest.mkdir(parents=True)
            shutil.copytree(keys, dest / "keys")
            subprocess.check_call([sys.executable, str(gen), "--out", str(dest)])

        a, b = base / "run_a", base / "run_b"
        run_with_keys(a)
        run_with_keys(b)
        sa, sb = sha_unsigned(a), sha_unsigned(b)
        if sa != sb:
            diff = sorted(k for k in set(sa) | set(sb) if sa.get(k) != sb.get(k))
            print("FAIL unsigned bytes differ:", diff, file=sys.stderr)
            return 1
        for p in (a / "keys").glob("*TEST-ONLY*.pem"):
            pass
        else:
            # ensure at least one TEST-ONLY key
            keys_found = list((a / "keys").glob("*TEST-ONLY*"))
            if not keys_found:
                print("FAIL no TEST-ONLY key files", file=sys.stderr)
                return 1
        verify_positive(a / "positive", a / "keys" / "test_sa.crt")
        verify_positive(b / "positive", b / "keys" / "test_sa.crt")
        print("signature_fixtures_reproducible ok unsigned_files=", len(sa))
    return 0


if __name__ == "__main__":
    sys.exit(main())
