#
# ===== overrideVgRemap =====
#
# Override ONE vertex group remap at runtime, without rebuilding the C++ tables.
#
# A vertex group remap is the blend-weight table: which of the SOURCE character's vertex groups
# becomes which of the TARGET's. It is what `*RemapBlend.buf` is built from, so getting it wrong
# does not break the .ini at all -- the fix reports success, the mod loads, and the model deforms
# wrongly in game. If a game update moved a character's vertex groups, this is the file to edit.
#
#   py -3 overrideVgRemap.py --dump             print the remap the compiled tables ship
#   py -3 overrideVgRemap.py -s <mod folder>    fix that folder with the override applied
#   py -3 overrideVgRemap.py -s <mod folder> --ab   ... and diff it against NO override
#
# ALWAYS --ab A CHANGE. If the two runs come back identical, the override did not reach the bytes,
# whatever the log said -- and that has been the failure mode here far more often than a crash.
#
# Everything above `--- run ---` is the override. The rest is the harness.
#

import argparse
import os
import shutil
import stat
import sys

API = os.environ.get("AG_REMAP_REPO", os.path.join(
    "E:", os.sep, "Computer", "Games", "Genshin", "Repos", "Repos", "Fix-Raiden-Boss",
    "Anime Game Remap (for all users)", "api", "src", "py"))

sys.path.insert(0, os.path.abspath(API))
if (hasattr(os, "add_dll_directory")):
    os.add_dll_directory(os.path.join(os.path.abspath(API), "FixRaidenBoss2"))

import FixRaidenBoss2 as FRB  # noqa: E402


# ================================================================================= the override

SOURCE, TARGET = "KeqingOpulent", "Keqing"

# The game version this override registers AT. The table floor-matches, so anything at or below the
# version being fixed to wins over the shipped row -- the compiled Keqing pair sits at "4.0", so
# "6.1" takes precedence and "1.0" would be ignored.
AT_VERSION = "6.1"

# {source vertex group: target vertex group}, MERGED OVER the shipped remap -- so only the groups
# that actually moved need listing, and everything else keeps what it has.
#
# A source group with no entry at all is NOT dropped: BlendFile::remapIndices writes it as the
# negative bone index -index-1 and keeps its weight, so the game reads a garbage bone matrix for
# that share of the vertex. That is what a kink in the mesh looks like. Deleting a line is not a
# no-op.
#
# ---- KeqingOpulent -> Keqing, 2026-09-09 (issue #213, "kink at the elbow") ----
#
# Found with Tools/VGRemapFinder over fresh frame dumps of both skins (6.7). Neither skin's bones
# have moved since the 4.0-era dumps -- old-vs-new comes out as an exact identity for both -- and
# the finder's proposal agrees with the shipped remap on all 101 rows it has. What the shipped remap
# (and the pure-Python one before it) never had is a row for KeqingOpulent 75 and 99: two 57-vertex
# elbow helper bones, one per arm, weighted up to 0.35 on the skin around the elbow, whose vertices
# are otherwise driven 58% by the upper arm (47 / 49) and 20% by the forearm (71 / 95). Unmapped,
# they became bone -76 / -100 in the RemapBlend.buf.
#
# Where to send them: the same patch of skin on the base Keqing sits within 0.006 units of these
# vertices and is driven 69% by her upper arm (40 / 66) and 28% by her forearm (22 / 48), with no
# helper bone of her own there (her 45/46 and 71/72 are upper-arm-side helpers the hand-made remap
# already sends to KeqingOpulent's upper arm, and they carry ~1% of that skin). So the elbow helper
# goes to the upper arm it is attached to, on the same side as its majority weight.
#
# The compiled table has carried these two rows since the 2026-09-09 rebuild (VGRemapData.cpp), so
# against that build `--dump` reports "changed by this script: nothing" -- the override is kept as
# the record of the fix and as the template for the next one.
EDITS = {
    75: 40,     # left elbow helper  -> left upper arm  (mirror of 99)
    99: 66,     # right elbow helper -> right upper arm (mirror of 75)
}

# ...or set this to a whole {source: target} dict to replace the remap outright instead of merging.
REPLACE = None


def overrideRemap():
    if (REPLACE is not None):
        return dict(REPLACE)

    merged = shippedRemap()
    merged.update(EDITS)
    return merged


def applyOverride():
    # EVERY ModType shares one VGRemaps table -- each falls back to ModDataAssets' singleton unless
    # it was handed its own, and none of the shipped ones are. So this one addRows is process-wide,
    # which is exactly what makes it an override rather than a local edit.
    #
    # addRows REPLACES a row whose full key already exists; it does not merge into it. Merging is
    # overrideRemap's job above.
    table = modTypeOf(SOURCE).vgRemaps
    table.addRows([(["1.0", SOURCE, "", AT_VERSION, TARGET, ""], overrideRemap())])


# --------------------------------------------------------------------------------- run

_modTypes = {}


def modTypeOf(key):
    if (not _modTypes):
        FRB.CppGlobalModTypes.registerAll()
        for modType in FRB.CppGlobalModTypes.all():
            _modTypes[modType.name] = modType

    return _modTypes[key]


def shippedRemap():
    """The remap the compiled tables hand out for this pair, as a plain dict."""
    remap = modTypeOf(SOURCE).getVGRemap(TARGET)
    return {} if (remap is None) else dict(remap.remap)


def dump():
    for name, remap in (("shipped", shippedRemap()), ("override", overrideRemap())):
        gaps = [i for i in range(max(remap) + 1) if i not in remap] if remap else []
        print("{} {} -> {}: {} entries, source 0..{}, target max {}".format(
            name.ljust(8), SOURCE, TARGET, len(remap),
            max(remap) if remap else -1, max(remap.values()) if remap else -1))
        print("         unmapped source groups: {}".format(gaps if gaps else "none"))

    changed = {k: v for k, v in overrideRemap().items() if shippedRemap().get(k) != v}
    print("changed by this script: {}".format(changed if changed else "nothing"))
    return 0


def runFix(folder, override):
    FRB.CppGlobalModTypes.registerAll()
    if (override):
        applyOverride()

    service = FRB.RemapService(path = folder, keepBackups = False)
    service.fix()

    stats = service.stats
    print("   {}: ini fixed={} skipped={}".format(
        "overridden" if override else "shipped   ", len(stats.ini.fixed), len(stats.ini.skipped)))
    for path, err in stats.ini.skipped.items():
        print("      SKIPPED {}: {}".format(os.path.basename(path), err))

    return stats


def onReadOnly(func, path, _excInfo):
    """A mod folder copied out of a real install carries read-only files; rmtree refuses those."""
    os.chmod(path, stat.S_IWRITE)
    func(path)


def blends(folder, skipExistingIn = None):
    """The remapped Blend.buf files under 'folder'.

    A mod that has been fixed before still carries the blend from THAT fix, and it is not rewritten
    by this one -- so anything already present in the untouched original is left out, or it shows up
    as an unchanged file and reads like half the run failed.
    """
    found = sorted(os.path.relpath(os.path.join(root, name), folder)
                   for root, _dirs, files in os.walk(folder) for name in files
                   if name.lower().endswith(".buf") and "remapblend" in name.lower())

    if (skipExistingIn is None):
        return found

    return [name for name in found if not os.path.isfile(os.path.join(skipExistingIn, name))]


def runAB(source, keep = False):
    """Fix two copies -- one shipped, one overridden -- and compare the remapped Blend.buf files.

    Only the blend is compared: a vertex group remap cannot reach anything else, so a difference
    anywhere else would be noise and a difference here is the whole point.
    """
    source = os.path.abspath(source)
    folders = [os.path.join(os.path.dirname(source), prefix + os.path.basename(source))
               for prefix in ("_vg_shipped_", "_vg_override_")]

    for folder in folders:
        if (os.path.isdir(folder)):
            shutil.rmtree(folder, onerror = onReadOnly)
        shutil.copytree(source, folder)

    # The overridden run goes SECOND and in a fresh process it would not matter, but applyOverride
    # is process-wide and cannot be undone, so the order here is load-bearing.
    runFix(folders[0], override = False)
    runFix(folders[1], override = True)

    print()
    found = blends(folders[0], skipExistingIn = source)
    if (not found):
        print("NO *RemapBlend.buf WAS WRITTEN -- nothing was remapped, so there is nothing to")
        print("compare. Check the skipped counts above and that -s points at the right mod.")
        return 1

    differing = 0
    for name in found:
        a = open(os.path.join(folders[0], name), "rb").read()
        b = open(os.path.join(folders[1], name), "rb").read()
        delta = sum(1 for x, y in zip(a, b) if x != y) + abs(len(a) - len(b))
        differing += (delta > 0)
        print("{:<7} {} ({} bytes differ)".format("DIFFERS" if delta else "same", name, delta))

    print()
    print("{} of {} remapped Blend.buf files changed".format(differing, len(found)))
    print("the override reached the bytes" if differing else
          "THE OVERRIDE CHANGED NOTHING -- either EDITS is empty, or it re-states what is already "
          "shipped, or AT_VERSION is older than the version being fixed to")

    for folder in folders:
        # Deleted unless asked for: these are working copies sitting in the folder GIMI loads mods
        # out of, so leaving them behind adds two mods the game would try to load.
        if (keep):
            print("kept: {}".format(folder))
        else:
            shutil.rmtree(folder, onerror = onReadOnly)

    return 0


def main():
    parser = argparse.ArgumentParser(description = "override one vertex group remap at runtime")
    parser.add_argument("-s", "--folder", default = SOURCE, help = "the mod folder to fix")
    parser.add_argument("--dump", action = "store_true", help = "print the remap and exit")
    parser.add_argument("--ab", action = "store_true", help = "diff the Blend.buf against no override")
    parser.add_argument("--keep", action = "store_true", help = "keep the --ab working copies")
    args = parser.parse_args()

    os.chdir(os.path.dirname(os.path.abspath(__file__)))
    if (args.dump):
        return dump()

    if (not os.path.isdir(args.folder)):
        raise SystemExit("overrideVgRemap: no such folder: {}".format(os.path.abspath(args.folder)))

    if (args.ab):
        return runAB(args.folder, keep = args.keep)

    runFix(args.folder, override = True)
    return 0


if (__name__ == "__main__"):
    sys.exit(main())
