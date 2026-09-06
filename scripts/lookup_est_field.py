#!/usr/bin/env python3
"""Look up ESt (E10) field definitions in the ERiC schema documentation.

The 25MB HTML doc carries every type, enumeration and pattern the ERiC
plausibility checks enforce. Resolving fields here is far cheaper than
discovering them one rejection at a time.

    ./lookup_field.py E0100402 Person Vorsatz
"""
import html
import re
import sys
from pathlib import Path

DOC = Path(__file__).parent.parent / "downloads" / (
    "ERiC-44.2.4.0/Dokumentation/Datenarten/ElsterErklaerung/ESt/"
    "SchemaDokumentation/E10-2025.html"
)

_cache = {}


def _plain():
    if "plain" not in _cache:
        raw = DOC.read_text(encoding="utf-8", errors="replace")
        _cache["plain"] = re.sub(r"[ \t]+", " ", html.unescape(re.sub(r"<[^>]+>", " ", raw)))
    return _cache["plain"]


def _raw():
    # Schema fragments are HTML-escaped inside the page. Unescaping WITHOUT
    # stripping tags is what preserves xs:enumeration/@value.
    if "raw" not in _cache:
        _cache["raw"] = html.unescape(DOC.read_text(encoding="utf-8", errors="replace"))
    return _cache["raw"]


def resolve_type(typename, depth=0):
    """Follow restriction bases (and _RABE wrappers) to the real constraint."""
    if depth > 4:
        return {}
    doc = _raw()
    for m in re.finditer(rf'<xs:(?:simpleType|complexType)[^>]*name="{re.escape(typename)}"', doc):
        w = doc[m.start(): m.start() + 6000]
        vals = re.findall(r'<xs:enumeration value="([^"]{1,40})"', w)
        if vals:
            return {"enum": sorted(set(vals))[:30]}
        pat = re.search(r'<xs:pattern value="([^"]{1,150})"', w)
        if pat:
            return {"pattern": pat.group(1)}
        base = re.search(r'<xs:restriction base="([A-Za-z0-9_:]+)"', w)
        if base:
            return resolve_type(base.group(1).split(":")[-1], depth + 1)
    if typename.endswith("_RABE"):
        return resolve_type(typename[:-5], depth + 1)
    return {}


def describe(field):
    text = _plain()
    out = []

    # The definition site is the fragment that declares the element by name.
    for m in re.finditer(rf'name="{re.escape(field)}"', text):
        window = text[m.start() - 400: m.start() + 2500]

        typ = re.search(rf'name="{re.escape(field)}" type="([^"]+)"', window)
        doc = re.search(r"<xs:documentation>\s*(.*?)\s*</xs:documentation>", window, re.S)
        doc = doc or re.search(r"documentation>\s*([^<]{3,120})", window)
        line = re.search(r"Zeile\s*(\d+)", window)

        entry = {
            "type": typ.group(1) if typ else None,
            "doc": " ".join(doc.group(1).split())[:110] if doc else None,
            "line": line.group(1) if line else None,
        }
        if entry["type"] or entry["doc"]:
            out.append(entry)
        if len(out) >= 2:
            break

    constraints = {}
    for entry in out:
        if entry.get("type"):
            constraints = resolve_type(entry["type"])
            if constraints:
                break

    return out, constraints


def main():
    if not DOC.exists():
        sys.exit(f"schema doc not found: {DOC}")
    for field in sys.argv[1:] or ["Person"]:
        entries, cons = describe(field)
        print(f"=== {field} ===")
        if not entries:
            print("  not found\n")
            continue
        e = entries[0]
        print(f"  type       : {e['type']}")
        if e["doc"]:
            print(f"  meaning    : {e['doc']}")
        if e["line"]:
            print(f"  form line  : {e['line']}")
        if cons.get("enum"):
            print(f"  allowed    : {cons['enum']}")
        if cons.get("pattern"):
            print(f"  pattern    : {cons['pattern']}")
        if cons.get("length"):
            print(f"  length     : {cons['length']}")
        print()


if __name__ == "__main__":
    main()
