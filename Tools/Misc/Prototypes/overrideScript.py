#
# ===== overrideScript =====
#
# GanyuTwilight's parser and fixer, declared in Python and registered over the compiled-in ones,
# then A/B'd against them. `--ab` fixes two copies of the mod -- one compiled, one overridden -- and
# compares every file. "It ran" proves nothing here; matching output does.
#
#   py -3 overrideScript.py             fix GanyuTwilight with the overrides
#   py -3 overrideScript.py --ab        ... and diff it against the compiled fix
#   py -3 overrideScript.py --ab --keep ... keeping the two working copies to inspect
#
# Everything above `--- run ---` is the fix. The rest is the A/B harness, which a real prototype
# would not have.
#
# ONE FALSE ALARM: both runs FETCH the assets a mod left out, so a network hiccup shows as a few
# ...RemapDL files "only in one run" while every .ini still matches. The download counts printed per
# run are what tells that apart from a real difference; the reason is printed too, and has so far
# always read "Could not resolve hostname".
#

import argparse
import filecmp
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
OBJS = ["head", "body", "dress"]

NN_FIX = "CommandList\\global\\ORFix\\NNFix"
TN_0 = "CommandList\\TexFx\\TN.0"


def parserConfig():
    """What GanyuTwilight's .ini file looks like."""
    config = FRB.GIMICharParserConfig()
    config.modTypeId = modTypeOf(MOD).modTypeId
    config.drawnObjs = OBJS
    config.downloadCharFolder = MOD
    config.downloadVersionFolder = "4_4"
    config.downloadPrefix = MOD

    # Per character, and worth checking rather than copying -- Amber and Mona are 12.
    config.texcoordStride = 20
    return config


def fixerConfig():
    """What her fix does differently. Remapped onto Ganyu, the direction that LOSES a normal map.

    GanyuTwilight is a 4.4-era model: ps-t0 normal map, ps-t1 diffuse, ps-t2 lightmap. Ganyu predates
    that and reads ps-t0 diffuse, ps-t1 lightmap. So the normal map goes and the other two slide down
    a slot -- both renames in ONE entry, or ps-t1 -> ps-t0 gets re-read as the input to ps-t2 -> ps-t1.
    """
    config = FRB.GIMICharFixerConfig()
    config.drawnObjs = OBJS

    # The head's normal map goes, along with the reflection resources and the shared IB reference.
    config.objRegRemovals = [(obj, (["ps-t0"] if (obj == "head") else [])
                                   + ["ResourceRef" + obj.capitalize() + "Diffuse",
                                      "ResourceRef" + obj.capitalize() + "LightMap", "$CharacterIB"])
                             for obj in OBJS]

    config.objRegRemaps = [("head", [("ps-t1", ["ps-t0"]), ("ps-t2", ["ps-t1"])])]

    # NNFix rather than ORFix on the head, because the target has no normal map -- ORFix is the
    # normal-map one. TexFx is re-issued alongside it, naming ps-t0 as the diffuse's home now that
    # the shift has put it there. body and dress are not listed, so they take the default: NNFix.
    config.objFixCalls = [("head", [NN_FIX, TN_0])]

    # The shared drawindexed comes off ("", "ib") and is re-issued per drawn object.
    config.moveDrawIndexed = True
    return config


_modTypes = {}


def modTypeOf(key):
    """CppGlobalModTypes has registerAll/registerMissing/all and no get(), hence the scan."""
    if (not _modTypes):
        FRB.CppGlobalModTypes.registerAll()
        for modType in FRB.CppGlobalModTypes.all():
            _modTypes[modType.name] = _modTypes[modType.modTypeId] = modType

    return _modTypes[key]


# ================================================================================= run

def runFix(folder, override):
    FRB.CppGlobalModTypes.registerAll()
    FRB.CppStrategyOverrides.clear()

    if (override):
        FRB.CppStrategyOverrides.setParser(MOD, FRB.makeGIMICharParser(parserConfig()))
        FRB.CppStrategyOverrides.setFixer(MOD, TARGET, FRB.makeGIMICharFixer(fixerConfig()))

    service = FRB.RemapService(path = folder, keepBackups = False)
    service.fix()
    FRB.CppStrategyOverrides.clear()
    return service.stats


def report(label, stats):
    print("   {}: ini fixed={} skipped={}, downloads fixed={} hit={}".format(
        label, len(stats.ini.fixed), len(stats.ini.skipped),
        len(stats.download.fixed), len(stats.download.hit)))

    for path, err in list(stats.ini.skipped.items()) + list(stats.download.skipped.items()):
        print("      SKIPPED {}: {}".format(os.path.basename(path), err))


def onReadOnly(func, path, _excInfo):
    """A mod folder copied out of a real install carries read-only files; rmtree refuses those."""
    os.chmod(path, stat.S_IWRITE)
    func(path)


def snapshot(folder):
    return sorted(os.path.relpath(os.path.join(root, name), folder).replace(os.sep, "/")
                  for root, _dirs, files in os.walk(folder) for name in files)


def runAB(source, keep = False):
    source = os.path.abspath(source)
    folders = [os.path.join(os.path.dirname(source), prefix + os.path.basename(source))
               for prefix in ("_ab_cpp_", "_ab_py_")]

    for folder in folders:
        if (os.path.isdir(folder)):
            shutil.rmtree(folder, onerror = onReadOnly)
        shutil.copytree(source, folder)

    report("compiled  ", runFix(folders[0], override = False))
    report("overridden", runFix(folders[1], override = True))

    cppFiles, pyFiles = snapshot(folders[0]), snapshot(folders[1])
    onlyOne = sorted(set(cppFiles) ^ set(pyFiles))
    differing = [name for name in cppFiles if name in set(pyFiles) and not filecmp.cmp(
        os.path.join(folders[0], name.replace("/", os.sep)),
        os.path.join(folders[1], name.replace("/", os.sep)), shallow = False)]

    print()
    for name in onlyOne:
        print("ONLY IN ONE RUN: {}".format(name))
    for name in differing:
        print("DIFFERS        : {}".format(name))

    identical = (not onlyOne and not differing)
    print()
    print("{} files compared, {} only in one run, {} differing".format(
        len(cppFiles), len(onlyOne), len(differing)))
    print("IDENTICAL -- the override reproduces the compiled fix exactly" if identical
          else "NOT IDENTICAL")

    # Deleted unless asked for: these are working copies of a mod sitting in the folder GIMI loads
    # mods out of, so leaving them behind adds two mods the game would try to load.
    for folder in folders:
        if (keep):
            print("kept: {}".format(folder))
        else:
            shutil.rmtree(folder, onerror = onReadOnly)

    return 0 if identical else 1


def main():
    parser = argparse.ArgumentParser(description = "GanyuTwilight's fix, declared in Python")
    parser.add_argument("-s", "--folder", default = MOD, help = "the mod folder to fix")
    parser.add_argument("--ab", action = "store_true", help = "diff against the compiled fix")
    parser.add_argument("--keep", action = "store_true", help = "keep the --ab working copies")
    args = parser.parse_args()

    os.chdir(os.path.dirname(os.path.abspath(__file__)))
    if (not os.path.isdir(args.folder)):
        raise SystemExit("overrideScript: no such folder: {}".format(os.path.abspath(args.folder)))

    if (args.ab):
        return runAB(args.folder, keep = args.keep)

    report("overridden", runFix(args.folder, override = True))
    return 0


if (__name__ == "__main__"):
    sys.exit(main())
