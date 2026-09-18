#
# ===== overrideScript2 =====
#
# The OTHER way to override a fix: building a GIMIParser and a GIMIFixer by hand out of the API's
# own edits, instead of declaring a GIMICharFixerConfig and letting makeGIMICharFixer assemble them
# (which is what overrideScript.py does, and what you should reach for first).
#
# This is what you want when the config cannot say what your fix does -- an edit no field covers, or
# a character that is not the standard GIMI shape at all. It is also just a readable place to see
# what those edits are.
#
#   py -3 overrideScript2.py            fix a copy of GanyuTwilight and print what came out
#   py -3 overrideScript2.py -s <folder> a different mod folder
#   py -3 overrideScript2.py --keep     keep the working copy instead of deleting it
#
# NOT a faithful GanyuTwilight fix -- overrideScript.py is that, and it is A/B-proven byte-identical
# to the compiled one. This is a deliberately smaller fix, chosen so each edit's effect is visible in
# the output: it renames, swaps the hashes, drops the head's normal map, shifts what is left down a
# slot, and re-issues NNFix. It leaves out the blend remap, the downloads, the drawindexed move and
# the face swap. The mod it produces is NOT playable; it is readable.
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


MOD, TARGET = "GanyuTwilight", "Ganyu"

# A "mod object" is (component, object). The component is almost always "" -- it exists for mods
# that draw the same object more than once. These are the graphs the parser sorts sections into and
# the fixer edits, and the names are yours to choose: they only have to agree between the two.
OBJS = [("", "head"), ("", "body"), ("", "dress")]
BLEND, IB = ("", "blend"), ("", "ib")

NN_FIX = "CommandList\\global\\ORFix\\NNFix"


# ============================================================================== the parser

def makeParser(iniFile, modTypeId):
    """Sorts the .ini file's sections into one graph per mod object.

    A parser is a GIMIParser plus a rule for deciding which mod object a section belongs to. The
    rule is 'objTargetFuncs': each is called per section and returns the mod objects it belongs to.

    The rule below is by NAME, which is the simplest thing that works and is easy to read. The
    shipped parsers classify by HASH instead, through GIMISectionClassifier -- that is what makes a
    fix survive a modder renaming their sections, and what tells head from body when both carry the
    same 'ib' hash and differ only in their match_first_index. Name matching cannot do that; it is
    fine here because GanyuTwilight's sections are named after their objects.
    """
    def classify(parser, sectionName, section, disjoint, part, kvps):
        name = sectionName.lower()
        if (not name.startswith("textureoverride")):
            return []

        # Skip what a PREVIOUS fix wrote. A mod folder in the wild is rarely pristine -- this one
        # still carries a remap block from an earlier run -- and without this guard the rule below
        # matches [TextureOverrideGanyuTwilightGanyuRemapBlend] as a blend and fixes the fix:
        # doubled names, and a HashNotFound where the hash was already the target's.
        #
        # A hash-based classifier does not need this, which is a large part of why the shipped
        # parsers use one.
        if ("remap" in name):
            return []

        for modObj in OBJS + [BLEND]:
            if (name.endswith(modObj[1])):
                return [modObj]

        # Her shared draw call: [TextureOverrideGanyuTwilightIB].
        return [IB] if (name.endswith("ib")) else []

    return FRB.GIMIParser(iniFile,
                          modObjs = OBJS + [BLEND, IB],
                          objTargetFuncs = [classify],
                          modTypeId = modTypeId)


# =============================================================================== the fixer

def makeFixer(parser, toModName, modTypeId):
    """Copies each graph, edits the copy, and writes it out under a remapped name.

    A fixer is a GIMIFixer plus a list of edits. GIMIFixer deep-copies every graph the parser built,
    hands the copies to the edits, and appends the result to the .ini file -- the original sections
    are never touched, which is why an unfixed mod still works after a fix.

    The edits are grouped by mod object, so each graph gets only the edits meant for it. There are
    two kinds and they compose in the same list:

        BaseRegEdit       runs per IfContentPart -- RegRemove, RegRemap, RegNewVals, RegAssetRemap
        BaseIniGraphEdit  walks the whole graph itself -- GraphRename, RegDelimitedAdd,
                          RegSurroundedAdd, RegFillMissing

    ORDER MATTERS within an object's list. They run left to right over the same graph, so an edit
    that reads a register has to come before one that renames it away.
    """
    modType = modTypeOf(modTypeId)
    ini = parser._iniFile

    # ---- 1. rename the copy ----
    #
    # This is what turns the copy into the remapped mod. Without it the copy renders as a verbatim
    # duplicate of the source, and the game sees two sections claiming the same hash.
    #
    # CppIniNamingTools, not the pure-Python IniNamingTools of the same name -- the latter has a
    # confirmed bug in getModSuffixedName. Each resource kind has its own convention:
    # getRemapFixName for an ordinary section, getRemapBlendName for a Blend.buf, getRemapIbName for
    # an .ib, and so on. Using the generic one everywhere produces names no other tool recognises.
    naming = FRB.CppIniNamingTools
    rename = FRB.GraphRename(lambda name: naming.getRemapFixName(name, toModName))
    renameBlend = FRB.GraphRename(lambda name: naming.getRemapBlendName(name, toModName))
    renameIb = FRB.GraphRename(lambda name: naming.getRemapIbName(name, toModName))

    # ---- 2. point the copy at the target's model ----
    #
    # Every 'hash = ...' naming GanyuTwilight becomes the one naming Ganyu. RegAssetRemap does it in
    # two steps -- reverse-look-up the old value to find which row owns it, then forward-look-up that
    # row against the target -- so it does not need to be told which KIND of hash a register holds.
    #
    # 'fromModName' is not optional in practice: leave it empty and the reverse lookup becomes
    # non-deterministic wherever the source and target share a value.
    hashRemap = FRB.RegAssetRemap({"hash": (modType.hashes, "HashNotFound")},
                                  toModName, MOD, ini.fromVersion, ini.toVersion)

    # ---- 3. the register shift ----
    #
    # GanyuTwilight has a normal map (ps-t0) and Ganyu does not, so it goes and the two textures
    # behind it slide down a slot.
    #
    # A RegRemove with None as the value means "every occurrence, whatever its value"; a callable
    # there would remove only the ones it accepts.
    dropNormalMap = FRB.RegRemove({"ps-t0": None})

    # BOTH renames in ONE RegRemap, which is what makes this a shift rather than two renames that
    # collapse into one: the part is rebuilt in a single pass, consulting the rules once per
    # ORIGINAL key, so ps-t1 -> ps-t0 can never be re-read as the input to ps-t2 -> ps-t1. Two
    # separate RegRemaps in sequence would land both textures on ps-t0.
    shiftDown = FRB.RegRemap({"ps-t1": ["ps-t0"], "ps-t2": ["ps-t1"]})

    # ---- 4. re-issue the external library ----
    #
    # NNFix is a 3dmigoto command list that fixes up the remapped model's shading. It has to run
    # immediately before every draw call, and once at the end of any path that draws nothing -- which
    # is exactly what RegDelimitedAdd does: it adds a KVP once per delimiter-free stretch of every
    # execution path through the graph.
    #
    # pathEndOnlyWhenUndelimited keeps the trailing addition off a path that already drew. Without
    # it a path picks up one more call after its last draw, which for a library that toggles state
    # per call leaves that state toggled.
    addNNFix = FRB.RegDelimitedAdd([("run", NN_FIX)], {"drawindexed": []},
                                   pathEndOnlyWhenUndelimited = True)

    # The mod's own NNFix calls go first, or the re-issue above just duplicates them.
    dropOldFixCalls = FRB.RegRemove({"run": lambda _ind, val: val == NN_FIX})

    # ---- putting them together ----
    #
    # The dict is keyed by mod object; the list is that graph's edits in order. It is wrapped in a
    # one-element list because a fixer can write more than one .ini file (a merge does), and each
    # entry describes one of them.
    edits = {
        ("", "head"): [dropOldFixCalls, dropNormalMap, shiftDown, rename, addNNFix, hashRemap],
        ("", "body"): [dropOldFixCalls, rename, addNNFix, hashRemap],
        ("", "dress"): [dropOldFixCalls, rename, addNNFix, hashRemap],
        BLEND: [renameBlend, hashRemap],
        IB: [renameIb, hashRemap],
    }

    return FRB.GIMIFixer(parser,
                         graphGroupEdits = [FRB.GraphGroupEdit([edits])],
                         # Which mods to fix TO. Without it the fixer runs its edits zero times and
                         # the copy comes out unedited.
                         modsToFix = [toModName])


# ===================================================================================== run

_modTypes = {}


def modTypeOf(key):
    """CppGlobalModTypes has registerAll/registerMissing/all and no get(), hence the scan."""
    if (not _modTypes):
        FRB.CppGlobalModTypes.registerAll()
        for modType in FRB.CppGlobalModTypes.all():
            _modTypes[modType.name] = _modTypes[modType.modTypeId] = modType

    return _modTypes[key]


def onReadOnly(func, path, _excInfo):
    """A mod folder copied out of a real install carries read-only files; rmtree refuses those."""
    os.chmod(path, stat.S_IWRITE)
    func(path)


def main():
    parser = argparse.ArgumentParser(description = "Overriding a fix with GIMIParser/GIMIFixer")
    parser.add_argument("-s", "--folder", default = MOD, help = "the mod folder to fix")
    parser.add_argument("--keep", action = "store_true", help = "keep the working copy")
    args = parser.parse_args()

    os.chdir(os.path.dirname(os.path.abspath(__file__)))
    if (not os.path.isdir(args.folder)):
        raise SystemExit("overrideScript2: no such folder: {}".format(os.path.abspath(args.folder)))

    # A COPY, always. This fix is deliberately incomplete, so pointing it at the real folder would
    # leave a mod that does not work.
    work = "_demo_" + args.folder
    if (os.path.isdir(work)):
        shutil.rmtree(work, onerror = onReadOnly)
    shutil.copytree(args.folder, work)

    modTypeOf(MOD)
    FRB.CppStrategyOverrides.clear()
    FRB.CppStrategyOverrides.setParser(MOD, makeParser)
    FRB.CppStrategyOverrides.setFixer(MOD, TARGET, makeFixer)

    service = FRB.RemapService(path = work, keepBackups = False)
    service.fix()
    FRB.CppStrategyOverrides.clear()

    stats = service.stats
    print("ini fixed={} skipped={}".format(len(stats.ini.fixed), len(stats.ini.skipped)))

    # stats.ini.skipped is the ONLY place a failure shows up with no logger attached -- a fix that
    # raises is recorded here and nothing is printed anywhere else.
    for path, err in stats.ini.skipped.items():
        print("SKIPPED {}: {}".format(os.path.basename(path), err))

    for path in sorted(stats.ini.fixed)[:1]:
        print()
        print("--- what the edits produced, from {} ---".format(os.path.basename(path)))
        with open(path, "r", encoding = "utf-8") as handle:
            text = handle.read()

        marker = text.find("; --------------- ")
        print(text[marker:] if (marker >= 0) else "(no remap block -- the fixer wrote nothing)")

    if (args.keep):
        print("kept: {}".format(os.path.abspath(work)))
    else:
        shutil.rmtree(work, onerror = onReadOnly)

    return 0


if (__name__ == "__main__"):
    sys.exit(main())
