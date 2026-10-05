#
# ===== abFixBindings =====
#
# A/B two FIXED copies of one mod -- the same mod fixed by two builds, or before and after a change
# to the mod's own .ini -- by what the remap actually BINDS rather than by its text:
#
#   py -3 abFixBindings.py <fixed copy A> <fixed copy B>
#
# Every `ps-tN` / `vbN` / `ib` line inside a section whose name carries `Remap` is resolved to the
# md5 of the FILE its resource names, so two runs that declare one texture under different section
# names compare equal and a binding that moved to a different file (or to a file that is not there:
# `MISSING`) does not. Every `*RemapBlend.buf` is then compared byte for byte. Prints one summary
# line per half plus up to 30 differing bindings.
#
# Why it exists (2026-10-05): a textual diff of two fixed `.ini` files is all noise -- download names,
# section order, a twin texture override a hash update added -- and `abWWMI.py` compares a prototype
# with the compiled tables, not two builds. This is what proved four changes in one day changed only
# what they meant to: a shared-code reader (only the one mod it was for moved, across 8 Sanhua and 23
# Chisa / ChisaParfait mods), two hash-table edits and a mod-side hash update (0 bindings moved over
# 13 mods). Prove it can fail before trusting a zero: run it over a pair you KNOW differs.
#
# To fix one copy with an OLDER build in the same session, without swapping the installed `.pyd`:
# copy `api/src/py/FixRaidenBoss2` into `<scratch>/Anime Game Remap (for all users)/api/src/py/`,
# drop the old `core.*.pyd` into it, and run the launcher with `AG_REMAP_REPO=<scratch>`; check
# with `FixRaidenBoss2.core.__file__` that the scratch copy is the one that loads.
#

import glob
import hashlib
import os
import re
import sys


def md5(path):
    try:
        with open(path, "rb") as f:
            return hashlib.md5(f.read()).hexdigest()[:8]
    except OSError:
        return "MISSING"


def bindings(root):
    """(ini, section, register, nth) -> md5 of the file bound there, for every remapped section"""
    out = {}
    for ini in glob.glob(os.path.join(root, "**", "*.ini"), recursive = True):
        rel = os.path.relpath(ini, root)
        if (os.path.basename(ini).lower().startswith("disabled")):
            continue
        with open(ini, encoding = "utf-8", errors = "replace") as f:
            text = f.read()
        sections = {m.group(1): m.group(2) for m in re.finditer(r"(?ms)^\[([^\]]+)\]\s*$(.*?)(?=^\[|\Z)", text)}
        files = {}
        for name, body in sections.items():
            filename = re.search(r"(?im)^\s*filename\s*=\s*(.+?)\s*$", body)
            if (filename):
                path = os.path.join(os.path.dirname(ini), filename.group(1).replace("\\", "/"))
                files[name.lower()] = md5(os.path.normpath(path))
        for name, body in sections.items():
            if ("Remap" not in name):
                continue
            seen = {}
            for m in re.finditer(r"(?im)^\s*(ps-t\d+|vb\d|ib)\s*=\s*(?:ref\s+)?(\S+)", body):
                reg = m.group(1).lower()
                seen[reg] = seen.get(reg, 0) + 1
                out[(rel, name, reg, seen[reg])] = files.get(m.group(2).lower(), m.group(2))
    return out


def main():
    if (len(sys.argv) != 3):
        raise SystemExit("usage: abFixBindings.py <fixed copy A> <fixed copy B>")
    a, b = sys.argv[1], sys.argv[2]

    ta, tb = bindings(a), bindings(b)
    diff = sorted(k for k in set(ta) | set(tb) if ta.get(k) != tb.get(k))
    print(f"{len(ta)} bindings in A, {len(tb)} in B, {len(diff)} differ")
    for k in diff[:30]:
        print("  ", k, ta.get(k), "->", tb.get(k))

    blends = [os.path.relpath(p, a) for p in glob.glob(os.path.join(a, "**", "*RemapBlend.buf"), recursive = True)]
    bad = [r for r in blends if md5(os.path.join(b, r)) == "MISSING" or md5(os.path.join(a, r)) != md5(os.path.join(b, r))]
    print(f"{len(blends)} RemapBlend files, {len(bad)} differ or missing {bad[:5]}")


if (__name__ == "__main__"):
    main()
