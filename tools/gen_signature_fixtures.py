#!/usr/bin/env python3
"""Generate Part 15 signature fixtures (docs/part15-signature-contract.md).

Python 3 stdlib + openssl CLI only (no pip crypto). TEST-ONLY private keys.
Reproducibility (A2): unsigned payload bytes + scripts are byte-identical across runs;
signatures are randomized ECDSA and only required to verify.

    python3 tools/gen_signature_fixtures.py
    python3 tools/gen_signature_fixtures.py --out DIR
"""
from __future__ import annotations

import argparse
import base64
import hashlib
import os
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

REPO = Path(__file__).resolve().parents[1]
SRC_CELLS = [
    REPO / "testdata/s101/101AA00DS0008.000",
    REPO / "testdata/s101/101AA00DS0003.000",
]
VERIFY_TIME = "2026-01-15T00:00:00Z"
# Fixed cert validity windows. OpenSSL 3.0.x (Ubuntu CI) rejects
# `req`/`x509` `-not_before`/`-not_after` (added later); use `openssl ca`
# `-startdate`/`-enddate` which exist on 1.0+/3.0/3.5.
NB = "250101000000Z"       # 2025-01-01
NA = "300101000000Z"       # 2030-01-01
NA_EXPIRED = "251201000000Z"  # 2025-12-01 (before verify_time)

SA_ID = "TESTSA"
DS_ID = "urn:mrn:iho:99ZZ:90001"
SA_CN = "urn:mrn:iho:99ZZ:01810"
DS_CN = "urn:mrn:iho:99ZZ:90001"
ROGUE_ID = "ROGUE"
ROGUE_CN = "urn:mrn:iho:99ZZ:rogue"

CELL_EDITION = {
    "101AA00DS0008.000": 8,
    "101AA00DS0003.000": 9,
}


def run(cmd, **kw):
    r = subprocess.run(cmd, capture_output=True, text=True, **kw)
    if r.returncode != 0:
        sys.stderr.write(f"CMD FAIL: {' '.join(cmd)}\n{r.stdout}\n{r.stderr}\n")
        raise SystemExit(r.returncode)
    return r


def write_bytes(path: Path, data: bytes):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(data)


def write_text(path: Path, text: str):
    # LF only for reproducibility of unsigned XML we control
    write_bytes(path, text.encode("utf-8"))


def pem_body(pem: str) -> str:
    lines = [ln.strip() for ln in pem.splitlines() if ln and not ln.startswith("-----")]
    return "".join(lines)


def sign_file(key_pem: Path, data_path: Path) -> str:
    sig = Path(tempfile.mkstemp(suffix=".sig")[1])
    try:
        run(["openssl", "dgst", "-sha384", "-sign", str(key_pem), "-out", str(sig), str(data_path)])
        return base64.b64encode(sig.read_bytes()).decode("ascii")
    finally:
        sig.unlink(missing_ok=True)


def gen_ec_key(path: Path):
    run(["openssl", "ecparam", "-name", "secp384r1", "-genkey", "-noout", "-out", str(path)])


def _write_ca_config(td: Path, extensions: str | None) -> Path:
    """Minimal openssl.cnf for `openssl ca` (OpenSSL 3.0-compatible dates)."""
    (td / "index.txt").write_text("")
    (td / "serial").write_text("01\n")
    (td / "newcerts").mkdir()
    ext_line = ""
    ext_section = ""
    if extensions:
        ext_line = "x509_extensions = cert_ext\n"
        ext_section = f"\n[cert_ext]\n{extensions}\n"
    # Forward slashes: openssl.cnf treats backslash as line continuation.
    def p(x: Path) -> str:
        return x.resolve().as_posix()

    cnf = td / "ca.cnf"
    cnf.write_text(
        "[ca]\n"
        "default_ca = CA_default\n"
        "\n"
        "[CA_default]\n"
        f"database = {p(td / 'index.txt')}\n"
        f"serial = {p(td / 'serial')}\n"
        f"new_certs_dir = {p(td / 'newcerts')}\n"
        "default_md = sha384\n"
        "policy = policy_any\n"
        "unique_subject = no\n"
        f"{ext_line}"
        "\n"
        "[policy_any]\n"
        "countryName = optional\n"
        "stateOrProvinceName = optional\n"
        "organizationName = optional\n"
        "commonName = supplied\n"
        f"{ext_section}"
    )
    return cnf


def gen_self_signed(key: Path, out_crt: Path, subj: str, not_after: str = NA, sha: str = "sha384"):
    # `openssl ca -selfsign -startdate/-enddate` works on OpenSSL 3.0 and 3.5.
    # (`req`/`x509` `-not_before`/`-not_after` are rejected on Ubuntu 3.0.13.)
    with tempfile.TemporaryDirectory() as td:
        td_path = Path(td)
        cnf = _write_ca_config(td_path, "basicConstraints = critical,CA:TRUE\n")
        csr = td_path / "req.csr"
        run(["openssl", "req", "-new", "-key", str(key), "-out", str(csr), "-subj", subj])
        run([
            "openssl", "ca", "-batch", "-notext", "-config", str(cnf),
            "-in", str(csr), "-out", str(out_crt),
            "-keyfile", str(key), "-selfsign",
            "-startdate", NB, "-enddate", not_after,
            "-md", sha,
        ])


def gen_signed_cert(key: Path, ca_crt: Path, ca_key: Path, out_crt: Path, subj: str,
                    not_after: str = NA, sha: str = "sha384"):
    with tempfile.TemporaryDirectory() as td:
        td_path = Path(td)
        cnf = _write_ca_config(td_path, None)
        csr = td_path / "req.csr"
        run(["openssl", "req", "-new", "-key", str(key), "-out", str(csr), "-subj", subj])
        run([
            "openssl", "ca", "-batch", "-notext", "-config", str(cnf),
            "-in", str(csr), "-out", str(out_crt),
            "-cert", str(ca_crt), "-keyfile", str(ca_key),
            "-startdate", NB, "-enddate", not_after,
            "-md", sha,
        ])


def catalog_xml(cells: list[tuple], certificates_xml: str,
                extra_meta: str = "") -> str:
    """cells: list of (fileName, sig_element_inner_or_empty, alg[, edition])
    sig_element_inner is full S100SE child XML or empty for missing.
    """
    parts = []
    for item in cells:
        fn, sig_xml, alg = item[0], item[1], item[2]
        edition = item[3] if len(item) > 3 else 1
        block = f"""    <S100XC:S100_DatasetDiscoveryMetadata>
      <S100XC:fileName>{fn}</S100XC:fileName>
      <S100XC:purpose>newEdition</S100XC:purpose>
      <S100XC:editionNumber>{edition}</S100XC:editionNumber>
      <S100XC:updateNumber>0</S100XC:updateNumber>
      <S100XC:productSpecification>
        <S100XC:name>S-101</S100XC:name>
        <S100XC:version>2.0.0</S100XC:version>
        <S100XC:productIdentifier>S-101</S100XC:productIdentifier>
      </S100XC:productSpecification>
"""
        if alg is not None:
            block += f"      <S100XC:digitalSignatureReference>{alg}</S100XC:digitalSignatureReference>\n"
        if sig_xml is not None:
            block += "      <S100XC:digitalSignatureValue>\n"
            block += sig_xml
            block += "      </S100XC:digitalSignatureValue>\n"
        block += "    </S100XC:S100_DatasetDiscoveryMetadata>\n"
        parts.append(block)
    return (
        '<?xml version="1.0" encoding="UTF-8"?>\n'
        '<S100XC:S100_ExchangeCatalogue xmlns:S100XC="http://www.iho.int/s100/xc/5.2"\n'
        '  xmlns:S100SE="http://www.iho.int/s100/se/5.2"\n'
        '  xmlns:gco="http://standards.iso.org/iso/19115/-3/gco/1.0">\n'
        "  <S100XC:identifier>\n"
        "    <S100XC:identifier>s100-chart signature fixtures</S100XC:identifier>\n"
        f"    <S100XC:dateTime>{VERIFY_TIME}</S100XC:dateTime>\n"
        "  </S100XC:identifier>\n"
        f"  <S100XC:certificates>\n{certificates_xml}  </S100XC:certificates>\n"
        "  <S100XC:datasetDiscoveryMetadata>\n"
        + "".join(parts)
        + "  </S100XC:datasetDiscoveryMetadata>\n"
        + extra_meta
        + "</S100XC:S100_ExchangeCatalogue>\n"
    )


def cert_xml(cert_path: Path, cid: str, issuer: str) -> str:
    body = pem_body(cert_path.read_text())
    return f'    <S100SE:certificate id="{cid}" issuer="{issuer}">{body}</S100SE:certificate>\n'


def sa_block(sa_crt: Path) -> str:
    return f'    <S100SE:schemeAdministrator id="{SA_ID}"/>\n' + cert_xml(sa_crt, DS_ID, SA_ID)


def standalone_sign(filename: str, certificates_xml: str, sig_b64: str, cert_ref: str) -> str:
    return (
        '<?xml version="1.0" encoding="UTF-8"?>\n'
        '<S100SE:StandaloneDigitalSignature xmlns:S100SE="http://www.iho.int/s100/se/5.2">\n'
        f"  <S100SE:filename>{filename}</S100SE:filename>\n"
        f"  <S100SE:certificates>\n{certificates_xml}  </S100SE:certificates>\n"
        f'  <S100SE:digitalSignature id="catalog" certificateRef="{cert_ref}">{sig_b64}</S100SE:digitalSignature>\n'
        "</S100SE:StandaloneDigitalSignature>\n"
    )


def sig_on_data(sig_b64: str, cert_ref: str, sid: str) -> str:
    return (
        f'        <S100SE:S100_SE_SignatureOnData dataStatus="unencrypted" id="{sid}" '
        f'certificateRef="{cert_ref}">{sig_b64}</S100SE:S100_SE_SignatureOnData>\n'
    )


def copy_cells(case_dir: Path) -> list[Path]:
    cells_dir = case_dir / "cells"
    cells_dir.mkdir(parents=True, exist_ok=True)
    out = []
    for src in SRC_CELLS:
        dst = cells_dir / src.name
        shutil.copy2(src, dst)
        out.append(dst)
    return out


def build_positive(out_root: Path, keys: dict, case: str = "positive"):
    case_dir = out_root / case
    if case_dir.exists():
        shutil.rmtree(case_dir)
    cells = copy_cells(case_dir)
    ds_key, ds_crt, sa_crt = keys["ds_key"], keys["ds_crt"], keys["sa_crt"]

    cell_sigs = []
    for c in cells:
        b64 = sign_file(ds_key, c)
        rel = f"file:/cells/{c.name}"
        cell_sigs.append((rel, sig_on_data(b64, DS_ID, c.stem), "ECDSA-384-SHA2",
                           CELL_EDITION.get(c.name, 1)))

    cxml = catalog_xml(cell_sigs, sa_block(ds_crt))
    cat_path = case_dir / "CATALOG.XML"
    write_text(cat_path, cxml)
    cat_sig = sign_file(ds_key, cat_path)
    write_text(case_dir / "CATALOG.SIGN", standalone_sign("CATALOG.XML", sa_block(ds_crt), cat_sig, DS_ID))
    return case_dir


def mutate_copy(src_case: Path, dst_name: str, out_root: Path, mutator):
    dst = out_root / dst_name
    if dst.exists():
        shutil.rmtree(dst)
    shutil.copytree(src_case, dst)
    mutator(dst)
    return dst


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--out", type=Path, default=REPO / "testdata/s101/signatures")
    args = ap.parse_args()
    out: Path = args.out
    out.mkdir(parents=True, exist_ok=True)
    keys_dir = out / "keys"
    keys_dir.mkdir(parents=True, exist_ok=True)

    sa_key = keys_dir / "test_sa_TEST-ONLY.pem"
    ds_key = keys_dir / "test_ds_TEST-ONLY.pem"
    rogue_key = keys_dir / "rogue_sa_TEST-ONLY.pem"
    sa_crt = keys_dir / "test_sa.crt"
    ds_crt = keys_dir / "test_ds.crt"
    ds_expired = keys_dir / "test_ds_expired.crt"
    rogue_crt = keys_dir / "rogue_sa.crt"

    # Regenerate keys only if missing (keep stable keys across regen for less churn);
    # always regenerate certs with fixed dates.
    if not sa_key.exists():
        gen_ec_key(sa_key)
    if not ds_key.exists():
        gen_ec_key(ds_key)
    if not rogue_key.exists():
        gen_ec_key(rogue_key)

    gen_self_signed(sa_key, sa_crt, f"/C=MC/ST=SCHEME_ADMINISTRATOR/O=TestSA/CN={SA_CN}")
    gen_signed_cert(ds_key, sa_crt, sa_key, ds_crt,
                    f"/C=MC/ST=DATA_PRODUCER/O=TestDS/CN={DS_CN}")
    gen_signed_cert(ds_key, sa_crt, sa_key, ds_expired,
                    f"/C=MC/ST=DATA_PRODUCER/O=TestDS/CN={DS_CN}", not_after=NA_EXPIRED)
    gen_self_signed(rogue_key, rogue_crt, f"/C=MC/ST=SCHEME_ADMINISTRATOR/O=Rogue/CN={ROGUE_CN}")

    keys = {"ds_key": ds_key, "ds_crt": ds_crt, "sa_crt": sa_crt, "sa_key": sa_key,
            "ds_expired": ds_expired, "rogue_key": rogue_key, "rogue_crt": rogue_crt}

    positive = build_positive(out, keys, "positive")

    # 2a missing CATALOG.SIGN
    mutate_copy(positive, "missing_sign", out, lambda d: (d / "CATALOG.SIGN").unlink())

    # 2b invalid CATALOG.SIGN (flip a byte in the signature text)
    def bad_sign(d: Path):
        p = d / "CATALOG.SIGN"
        t = p.read_text()
        # flip a character inside the signature base64
        i = t.find('certificateRef="')
        j = t.find(">", i) + 1
        k = t.find("</S100SE:digitalSignature>", j)
        body = list(t[j:k])
        body[0] = "A" if body[0] != "A" else "B"
        p.write_text(t[:j] + "".join(body) + t[k:])
    mutate_copy(positive, "bad_catalog_sign", out, bad_sign)

    # 2c unparseable CATALOG.SIGN
    mutate_copy(positive, "unparseable_sign", out,
                lambda d: write_text(d / "CATALOG.SIGN", "NOT XML {{{"))

    # 2d one dataset byte flipped
    def tamper_cell(d: Path):
        cell = d / "cells" / "101AA00DS0008.000"
        b = bytearray(cell.read_bytes())
        b[-1] = (b[-1] + 1) % 256
        cell.write_bytes(bytes(b))
    mutate_copy(positive, "tampered_dataset", out, tamper_cell)

    # 2e issuer not SA — sign with rogue self-signed as the only cert, issuer=ROGUE
    def non_sa(d: Path):
        cells = list((d / "cells").glob("*.000"))
        cell_sigs = []
        for c in cells:
            b64 = sign_file(rogue_key, c)
            cell_sigs.append((f"file:/cells/{c.name}",
                              sig_on_data(b64, ROGUE_ID, c.stem), "ECDSA-384-SHA2",
                              CELL_EDITION.get(c.name, 1)))
        certs = f'    <S100SE:schemeAdministrator id="{ROGUE_ID}"/>\n' + cert_xml(
            rogue_crt, ROGUE_ID, ROGUE_ID)
        cxml = catalog_xml(cell_sigs, certs)
        cat = d / "CATALOG.XML"
        write_text(cat, cxml)
        cat_sig = sign_file(rogue_key, cat)
        write_text(d / "CATALOG.SIGN", standalone_sign("CATALOG.XML", certs, cat_sig, ROGUE_ID))
    mutate_copy(positive, "issuer_not_sa", out, non_sa)

    # 2g expired DS cert at verify_time
    def expired(d: Path):
        cells = list((d / "cells").glob("*.000"))
        cell_sigs = []
        for c in cells:
            b64 = sign_file(ds_key, c)
            cell_sigs.append((f"file:/cells/{c.name}",
                              sig_on_data(b64, DS_ID, c.stem), "ECDSA-384-SHA2",
                              CELL_EDITION.get(c.name, 1)))
        certs = f'    <S100SE:schemeAdministrator id="{SA_ID}"/>\n' + cert_xml(
            ds_expired, DS_ID, SA_ID)
        cxml = catalog_xml(cell_sigs, certs)
        cat = d / "CATALOG.XML"
        write_text(cat, cxml)
        cat_sig = sign_file(ds_key, cat)
        write_text(d / "CATALOG.SIGN", standalone_sign("CATALOG.XML", certs, cat_sig, DS_ID))
    mutate_copy(positive, "expired_cert", out, expired)

    # 2h FC/PC not SA-direct: add catalogueDiscoveryMetadata signed by DS (DS is SA-signed,
    # but not SA itself → 129). Use a tiny placeholder file.
    def fc_not_direct(d: Path):
        fc = d / "cells" / "fake_fc.xml"
        write_text(fc, "<FeatureCatalogue/>\n")
        b64 = sign_file(ds_key, fc)
        # rebuild catalog with extra catalogue meta
        cells = list((d / "cells").glob("*.000"))
        cell_sigs = []
        for c in cells:
            sb = sign_file(ds_key, c)
            cell_sigs.append((f"file:/cells/{c.name}",
                              sig_on_data(sb, DS_ID, c.stem), "ECDSA-384-SHA2",
                              CELL_EDITION.get(c.name, 1)))
        extra = (
            "  <S100XC:catalogueDiscoveryMetadata>\n"
            "    <S100XC:S100_CatalogueDiscoveryMetadata>\n"
            "      <S100XC:fileName>file:/cells/fake_fc.xml</S100XC:fileName>\n"
            "      <S100XC:scope>featureCatalogue</S100XC:scope>\n"
            "      <S100XC:digitalSignatureReference>ECDSA-384-SHA2</S100XC:digitalSignatureReference>\n"
            "      <S100XC:digitalSignatureValue>\n"
            + sig_on_data(b64, DS_ID, "fake_fc")
            + "      </S100XC:digitalSignatureValue>\n"
            "    </S100XC:S100_CatalogueDiscoveryMetadata>\n"
            "  </S100XC:catalogueDiscoveryMetadata>\n"
        )
        cxml = catalog_xml(cell_sigs, sa_block(ds_crt), extra_meta=extra)
        cat = d / "CATALOG.XML"
        write_text(cat, cxml)
        cat_sig = sign_file(ds_key, cat)
        write_text(d / "CATALOG.SIGN", standalone_sign("CATALOG.XML", sa_block(ds_crt), cat_sig, DS_ID))
    mutate_copy(positive, "fc_not_sa_direct", out, fc_not_direct)

    # 2i wrong algorithm string
    def bad_alg(d: Path):
        t = (d / "CATALOG.XML").read_text()
        t = t.replace("ECDSA-384-SHA2", "RSA-SHA256", 1)  # first dataset only
        write_text(d / "CATALOG.XML", t)
        # re-sign catalog (dataset sig still valid for untouched file; alg check fails first)
        cat_sig = sign_file(ds_key, d / "CATALOG.XML")
        write_text(d / "CATALOG.SIGN",
                   standalone_sign("CATALOG.XML", sa_block(ds_crt), cat_sig, DS_ID))
    mutate_copy(positive, "bad_algorithm", out, bad_alg)

    # 2j bad Base64
    def bad_b64(d: Path):
        t = (d / "CATALOG.XML").read_text()
        # corrupt first SignatureOnData body
        a = t.find("<S100SE:S100_SE_SignatureOnData")
        b = t.find(">", a) + 1
        c = t.find("</S100SE:S100_SE_SignatureOnData>", b)
        write_text(d / "CATALOG.XML", t[:b] + "!!!not-base64!!!" + t[c:])
        cat_sig = sign_file(ds_key, d / "CATALOG.XML")
        write_text(d / "CATALOG.SIGN",
                   standalone_sign("CATALOG.XML", sa_block(ds_crt), cat_sig, DS_ID))
    mutate_copy(positive, "bad_base64", out, bad_b64)

    # 2k signature belonging to a different file (swap sigs between two cells)
    def wrong_file_sig(d: Path):
        cells = sorted((d / "cells").glob("*.000"))
        assert len(cells) >= 2
        # sign cell0 with bytes of cell0 but put that sig on cell1's metadata entry (and vice versa)
        s0 = sign_file(ds_key, cells[0])
        s1 = sign_file(ds_key, cells[1])
        # put s0 on cells[1] entry and s1 on cells[0]
        cell_sigs = [
            (f"file:/cells/{cells[0].name}", sig_on_data(s1, DS_ID, cells[0].stem), "ECDSA-384-SHA2",
             CELL_EDITION.get(cells[0].name, 1)),
            (f"file:/cells/{cells[1].name}", sig_on_data(s0, DS_ID, cells[1].stem), "ECDSA-384-SHA2",
             CELL_EDITION.get(cells[1].name, 1)),
        ]
        cxml = catalog_xml(cell_sigs, sa_block(ds_crt))
        write_text(d / "CATALOG.XML", cxml)
        cat_sig = sign_file(ds_key, d / "CATALOG.XML")
        write_text(d / "CATALOG.SIGN",
                   standalone_sign("CATALOG.XML", sa_block(ds_crt), cat_sig, DS_ID))
    mutate_copy(positive, "wrong_file_sig", out, wrong_file_sig)

    # 3b self-signed root shipped inside the set (and used as issuer) — not in trust store
    # (same as issuer_not_sa with rogue; keep dedicated name)
    mutate_copy(out / "issuer_not_sa", "self_signed_in_set", out, lambda d: None)

    # missing dataset signature value
    def missing_dsig(d: Path):
        cells = list((d / "cells").glob("*.000"))
        cell_sigs = []
        for i, c in enumerate(cells):
            if i == 0:
                cell_sigs.append((f"file:/cells/{c.name}", None, "ECDSA-384-SHA2",
                                  CELL_EDITION.get(c.name, 1)))
            else:
                b64 = sign_file(ds_key, c)
                cell_sigs.append((f"file:/cells/{c.name}",
                                  sig_on_data(b64, DS_ID, c.stem), "ECDSA-384-SHA2",
                                  CELL_EDITION.get(c.name, 1)))
        cxml = catalog_xml(cell_sigs, sa_block(ds_crt))
        write_text(d / "CATALOG.XML", cxml)
        cat_sig = sign_file(ds_key, d / "CATALOG.XML")
        write_text(d / "CATALOG.SIGN",
                   standalone_sign("CATALOG.XML", sa_block(ds_crt), cat_sig, DS_ID))
    mutate_copy(positive, "missing_dataset_sig", out, missing_dsig)

    # README
    write_text(out / "README.md",
               "# Part 15 signature fixtures (IHO-S164-SIG)\n\n"
               "Generated by `tools/gen_signature_fixtures.py`. "
               "Private keys contain `TEST-ONLY` in the filename.\n"
               f"Fixed verify_time: `{VERIFY_TIME}`.\n")

    # Manifest of unsigned paths for reproducibility check (payload cells under positive)
    lines = []
    for dirpath, _, files in os.walk(out):
        for f in sorted(files):
            p = Path(dirpath) / f
            rel = p.relative_to(out).as_posix()
            # skip signature-bearing / randomized outputs
            if f in ("CATALOG.SIGN",) or f.endswith(".sig"):
                continue
            if "TEST-ONLY" in f and f.endswith(".pem"):
                # keys are stable once generated
                pass
            if f == "CATALOG.XML":
                continue  # may embed signature values
            if p.suffix == ".000" or f in ("README.md", "fake_fc.xml") or f.endswith(".crt"):
                h = hashlib.sha256(p.read_bytes()).hexdigest()
                lines.append(f"{h}  {rel}")
    write_text(out / "unsigned_manifest.sha256", "\n".join(sorted(lines)) + "\n")
    print(f"wrote fixtures under {out}")


if __name__ == "__main__":
    main()
