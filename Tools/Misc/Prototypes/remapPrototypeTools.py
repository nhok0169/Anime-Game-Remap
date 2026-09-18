"""
Shared .ini hygiene for the Bennett <-> BennettAdventure prototypes.

Nothing here is about either DIRECTION of the remap. It is about GIMI's own file conventions -- which
files the game loads, and what a merged mod's master means -- so both prototypes need all of it, and
each one of these was learned the expensive way on a real mod (2026-09-15):

- `activeInis`      GIMI ignores a file named DISABLED*, and a pass that reads one is reading claims
                    that were never real. One pass filled in buffers a REFUSED merged master named.
- `pickVariant`     a merged master binds every variant behind $swapvar and resolving `run =` finds
                    the first branch only, so fixing it writes resources for variants the split never
                    processed. Picking one variant makes the mod ordinary.
- `syncVariantHashes`  ...but the per-variant files are STALE, because GIMI's hash-update tools skip
                    DISABLED*. The master is the authority -- not the version table.

See AI Agent Help/CreatingRemaps/CLAUDE.md, "A MERGED MOD'S DISABLED VARIANTS CARRY STALE HASHES".
"""

import os
from typing import Dict, List, Optional


def activeInis(folder: str) -> List[str]:
    """
    Every .ini under the folder that the game will actually load.

    GIMI ignores a file whose name begins with DISABLED, so a pass that edits one is editing
    something with no effect -- and worse, it is reading that file's claims as if they were real.
    retargetTexcoords once walked a refused merged master this way and filled in buffers the writer
    had never written, from siblings, which is precisely the dangling-reference state the master was
    refused for (2026-09-15).
    """
    import glob

    return [p for p in sorted(glob.glob(os.path.join(folder, "**", "*.ini"), recursive = True))
            if (not os.path.basename(p).upper().startswith("DISABLED"))]

def mergedMasters(folder: str, disabled: bool = False) -> List[str]:
    """
    Every .ini in the folder that binds its buffers behind a $swapvar branch (a merged mod's master).

    `disabled` looks for one that has ALREADY been renamed out of the way, which is what makes the
    reconciliation below idempotent: a second run over a mod whose variant was picked earlier must
    still be able to reach the authority, or it silently skips the very corrections it exists for.
    """
    import glob

    if (disabled):
        candidates = [p for p in sorted(glob.glob(os.path.join(folder, "**", "*.ini"), recursive = True))
                      if (os.path.basename(p).upper().startswith("DISABLED"))]
    else:
        candidates = activeInis(folder)

    out = []
    for path in candidates:
        text = open(path, encoding = "utf-8", errors = "replace").read()
        if ("$swapvar" in text and "run = CommandList" in text):
            out.append(path)
    return out


def masterResources(masters: List[str]) -> Dict[str, str]:
    """
    Every resource the master declares whose file actually EXISTS, as an absolute path.

    Paths inside a .ini are Windows paths on every OS, and are relative to the .ini that names them,
    so they are resolved against the master's own folder here and re-expressed against the variant's
    folder by the caller. Only resources that resolve are returned: the point is to repair a dead
    reference from a live one, never the reverse.
    """
    out = {}
    for master in masters:
        base, section = os.path.dirname(master), None
        for line in open(master, encoding = "utf-8", errors = "replace"):
            stripped = line.strip()
            if (stripped.startswith("[") and stripped.endswith("]")):
                section = stripped[1:-1]
                continue

            key, sep, value = stripped.partition("=")
            if (section is None or not sep or key.strip().lower() != "filename"):
                continue
            path = os.path.normpath(os.path.join(base, value.strip().replace("\\", os.sep)))
            if (os.path.isfile(path)):
                out.setdefault(section, path)
    return out

def masterHashes(masters: List[str]) -> Dict[str, str]:
    """Every ``hash =`` a merged master declares, keyed by the section that declares it"""
    out = {}
    for master in masters:
        section = None
        with open(master, encoding = "utf-8", errors = "replace") as handle:
            for line in handle:
                stripped = line.strip()
                if (stripped.startswith("[") and stripped.endswith("]")):
                    section = stripped[1:-1]
                    continue

                key, sep, value = stripped.partition("=")
                if (section is not None and sep and key.strip().lower() == "hash"):
                    out.setdefault(section, value.strip())
    return out

def syncVariantHashes(folder: str, masters: List[str]) -> None:
    """
    Carry the merged master's hashes into whatever .ini files are enabled now.

    GIMI's hash-update tools skip a file named ``DISABLED*``, so in a merged mod only the MASTER is
    kept current -- the per-variant files rot at whatever game version they were merged at. Enabling
    one as-is hands the game hashes it no longer emits, and the character's own geometry simply never
    matches: the mod renders BROKEN while the remapped sections, keyed on the TARGET's hashes, draw
    perfectly. Every log line still says the fix worked.

    The master is the authority here rather than a version table, because it is the file the game was
    demonstrably loading. Bennett is the reason that distinction matters: his draw_vb history says
    ``8b2a1582`` was superseded at 4.1, and this mod's working master kept ``8b2a1582``.
    """
    import glob

    wanted = masterHashes(masters)
    if (not wanted):
        return

    for path in activeInis(folder):

        with open(path, "rb") as handle:
            original = handle.read()
        hadCRLF = b"\r\n" in original
        lines = original.decode("utf-8", errors = "replace").replace("\r\n", "\n").split("\n")

        section, changes = None, []
        for i, line in enumerate(lines):
            stripped = line.strip()
            if (stripped.startswith("[") and stripped.endswith("]")):
                section = stripped[1:-1]
                continue

            key, sep, value = stripped.partition("=")
            if (section not in wanted or not sep or key.strip().lower() != "hash"):
                continue
            if (value.strip() == wanted[section]):
                continue

            changes.append(f"{section}: {value.strip()} -> {wanted[section]}")
            lines[i] = line[:len(line) - len(line.lstrip())] + f"hash = {wanted[section]}"

        if (not changes):
            continue

        backup = path + ".preHashSync.bak"
        if (not os.path.isfile(backup)):
            with open(backup, "wb") as handle:
                handle.write(original)

        fixed = "\n".join(lines)
        with open(path, "wb") as handle:
            handle.write((fixed.replace("\n", "\r\n") if hadCRLF else fixed).encode("utf-8"))

        print(f"  refreshed {len(changes)} stale hash(es) in {os.path.relpath(path, folder)}, from the merged master:")
        for change in changes:
            print(f"      {change}")

def syncVariantResources(folder: str, masters: List[str]) -> None:
    """
    Repair the enabled .ini files' dead resource references from the master's live ones.

    A merge deduplicates: it moves every shared texture into one folder and rewrites the MASTER to
    point there, leaving each variant still naming a bare filename beside itself. Those references
    have been dead ever since, and nothing says so -- picking a variant produced a model with every
    texture missing, black in the original as well as the remap (2026-09-15).

    Two repairs, in this order, and only ever to a reference that does NOT resolve:

    1. the master declares the same resource and ITS path resolves -- rewrite the variant's to it,
       re-expressed relative to the variant's own folder (a `.ini` path is a Windows path on every
       OS, so it is written back with backslashes);
    2. nothing can supply it -- DROP every binding of that resource. The master binds nothing for
       this mod's front hair at all, which is how a draw is left to the GAME's textures. A binding
       to a file that does not exist is worse than no binding, because ORFix/NNFix re-slot whatever
       is bound whether the section bound it or not.
    """
    import re

    live = masterResources(masters)
    if (not live):
        return

    for path in activeInis(folder):
        with open(path, "rb") as handle:
            original = handle.read()
        hadCRLF = b"\r\n" in original
        lines = original.decode("utf-8", errors = "replace").replace("\r\n", "\n").split("\n")
        base = os.path.dirname(path)

        # ---- which resources this file declares, and whether they resolve ----
        declared, section = {}, None
        for i, line in enumerate(lines):
            stripped = line.strip()
            if (stripped.startswith("[") and stripped.endswith("]")):
                section = stripped[1:-1]
                continue
            key, sep, value = stripped.partition("=")
            if (section is None or not sep or key.strip().lower() != "filename"):
                continue
            declared[section] = (i, value.strip())

        repaired, dead = [], set()
        for name, (i, value) in declared.items():
            if ("Remap" in name):
                # the FIX's own outputs, not the mod's references. They are missing only because the
                # fix has not written them yet (or could not, for want of the very inputs repaired
                # here), and the fix manages their lifetime. Dropping them would delete its work.
                continue
            if (os.path.isfile(os.path.normpath(os.path.join(base, value.replace("\\", os.sep))))):
                continue
            if (name in live):
                rel = os.path.relpath(live[name], base).replace(os.sep, "\\")
                indent = lines[i][:len(lines[i]) - len(lines[i].lstrip())]
                lines[i] = f"{indent}filename = {rel}"
                repaired.append(f"{name}: {value} -> {rel}")
            else:
                dead.add(name)

        # ---- drop every binding of a resource nothing can supply ----
        dropped, keep, section = [], [], None
        for line in lines:
            stripped = line.strip()
            if (stripped.startswith("[") and stripped.endswith("]")):
                section = stripped[1:-1]
            else:
                key, sep, value = stripped.partition("=")
                if (sep and value.strip() in dead and key.strip().lower() != "filename"):
                    dropped.append(f"{section}: {stripped}  (no file anywhere; left to the game)")
                    continue
            keep.append(line)

        if (not repaired and not dropped):
            continue

        backup = path + ".preResSync.bak"
        if (not os.path.isfile(backup)):
            with open(backup, "wb") as handle:
                handle.write(original)

        fixed = "\n".join(keep)
        with open(path, "wb") as handle:
            handle.write((fixed.replace("\n", "\r\n") if hadCRLF else fixed).encode("utf-8"))

        rel = os.path.relpath(path, folder)
        if (repaired):
            print(f"  repaired {len(repaired)} dead resource path(s) in {rel}, from the merged master:")
            for r in repaired:
                print(f"      {r}")
        if (dropped):
            print(f"  dropped {len(dropped)} binding(s) in {rel} that name a file nothing has:")
            for d in dropped:
                print(f"      {d}")


def pickVariant(folder: str, variant: Optional[str]) -> None:
    """
    Turn a merged mod into an ordinary single-variant one, or refuse it.

    The master binds every variant behind $swapvar and the per-variant .ini files are DISABLED, so
    the master is all the game loads -- and fixing it emits resources for variants the split never
    processed. Rather than write a file full of references to buffers that were never created, this
    refuses outright unless --variant names one to keep.
    """
    masters = mergedMasters(folder)
    if (not masters):
        # Already picked on an earlier run: the master has been renamed out of the way. Reconcile
        # anyway -- the variant files are stale in hashes AND in resource paths, and a re-run that
        # quietly skipped that would be a pass that never fires.
        old = mergedMasters(folder, disabled = True)
        if (old):
            if (variant is not None):
                enabled = [os.path.relpath(p, folder) for p in activeInis(folder)]
                print(f"  NOTE: a variant is already enabled here, so --variant {variant!r} changes "
                      f"nothing: {', '.join(enabled) or '(none)'}")
                print(f"    to switch, rename the master back and the enabled variant to DISABLED* first")
            syncVariantHashes(folder, old)
            syncVariantResources(folder, old)
        return

    variants = {}
    for root, _dirs, files in os.walk(folder):
        for name in files:
            if (name.upper().startswith("DISABLED") and name.lower().endswith(".ini")):
                variants[os.path.basename(root)] = os.path.join(root, name)

    if (variant is None):
        print("\n  MERGED MOD -- REFUSING TO FIX IT")
        print(f"    {', '.join(os.path.relpath(m, folder) for m in masters)} binds every variant behind $swapvar,")
        print("    and this prototype resolves only the FIRST branch. Fixing it would write resources for")
        print("    variants the split never processed -- references to buffers that do not exist, which")
        print("    render as whatever was bound before them and report nothing.")
        if (variants):
            print(f"\n    Pick one with --variant: {', '.join(sorted(variants))}")
            print("    That disables the master and enables that variant, making this an ordinary mod.")
        else:
            print("\n    No DISABLED per-variant .ini files found to pick from.")
        raise SystemExit(1)

    if (variant not in variants):
        raise SystemExit(f"--variant {variant!r} is not one of: {', '.join(sorted(variants)) or '(none found)'}")

    disabledMasters = []
    for master in masters:
        disabled = os.path.join(os.path.dirname(master), "DISABLED" + os.path.basename(master))
        os.replace(master, disabled)
        disabledMasters.append(disabled)
        print(f"  disabled the merged master: {os.path.relpath(master, folder)}")

    src = variants[variant]
    enabled = os.path.join(os.path.dirname(src), os.path.basename(src)[len("DISABLED"):])
    os.replace(src, enabled)
    print(f"  enabled the '{variant}' variant: {os.path.relpath(enabled, folder)}")

    syncVariantHashes(folder, disabledMasters)
    syncVariantResources(folder, disabledMasters)
    print("  (to undo: rename the .ini files back, and restore any .preHashSync.bak)")
