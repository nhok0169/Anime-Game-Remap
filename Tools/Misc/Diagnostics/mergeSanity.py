"""The two sections a MERGE must write into every .ini it fixes, and neither is about the mod.

    python mergeSanity.py [--merged] <fixed folder> [<fixed folder> ...]

A merge lays every source component's buffers end to end and draws them through the TARGET's hashes,
so two sections have to be there beside the draws:

  * the target's ``ib`` with ``handling = skip`` -- without it the target's OWN draws still run, and
    the position / blend / texcoord overrides (which apply BY HASH to every draw) feed them the
    merged buffers through the target's own index buffer;
  * a ``VertexLimitRaise`` with ``override_vertex_count`` -- the merged model has every component's
    vertices and the target's own limit is its own model's, so everything past it reads whatever
    follows in memory;
  * the remapped `blend`_ section's ``handling = skip`` and ``draw`` -- they re-issue the vertex pass
    over the merged buffer, and without them the game runs its own with the target's vertex count, so
    everything past it is never skinned.

All THREE were MISSING for a mod that carries only one of the skin's components (2026-09-22): each is
made by remapping that section out of the mod, the fix took them from the skeleton component, and a
Bangs-only mod carries none of them. Each failure drew a different wreck -- see
`AI Agent Help/CreatingRemaps/Images/CitlaliWhisper/6_7/CitlaliBrokenModel.jpg` and
`CitlaliBrokenModel2.jpg` -- and every other check in this folder passed both times.

Reports per fixed .ini (one that carries a `...RemapFix` section). The ib check is for EVERY shape;
the vertex-limit one needs `--merged`, because a classic fix writes no override (its target is the
same mesh) and names its copied command lists `...RemapFix` too, so there is no way to tell a merge's
output from a classic one by reading it. Exits non-zero on any report.
"""
import os, re, sys

args = [a for a in sys.argv[1:] if a != "--merged"]
mergedOnly = "--merged" in sys.argv

bad = total = 0
for root in args:
    for d, _, fs in os.walk(root):
        for f in fs:
            if not f.lower().endswith(".ini") or f.upper().startswith("DISABLED"):
                continue

            path = os.path.join(d, f)
            txt = open(path, encoding="utf-8", errors="replace").read()
            if "RemapFix]" not in txt:
                continue

            total += 1
            rel = os.path.relpath(path, root)

            skips = 0
            limits = 0
            blends = 0
            blendsDone = 0
            for section in re.split(r"(?m)^\[", txt)[1:]:
                name, _, body = section.partition("]")
                lowered = name.lower()

                if "remapib" in lowered and re.search(r"(?mi)^\s*handling\s*=\s*skip", body):
                    skips += 1

                # In a REMAPPED section: the mod's own VertexLimitRaise is still in the file, on the
                # SOURCE's hash, and counts for nothing here.
                if "remapfix" in lowered and re.search(r"(?mi)^\s*override_vertex_count\s*=", body):
                    limits += 1

                if "remapblend" in lowered and "textureoverride" in lowered:
                    blends += 1
                    if (re.search(r"(?mi)^\s*handling\s*=\s*skip", body)
                            and re.search(r"(?mi)^\s*draw\s*=", body)):
                        blendsDone += 1

            # A SPLIT writes one per target component and a classic fix one, so "at least one" is the
            # invariant every shape shares.
            if skips < 1:
                bad += 1
                print(f"  {rel}: no '...RemapIB' section with `handling = skip` -- the target's own draws still run")

            if mergedOnly and limits == 0:
                bad += 1
                print(f"  {rel}: merged members but no remapped `override_vertex_count` -- nothing raises the target's vertex limit")

            if mergedOnly and blends != blendsDone:
                bad += 1
                print(f"  {rel}: {blends - blendsDone} of {blends} remapped blend sections without `handling = skip` + `draw` -- the merged vertices are never skinned")

print(f"{total} fixed .ini files checked, {bad} missing a section a merge must write")
sys.exit(1 if bad else 0)
