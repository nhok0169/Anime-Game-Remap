# Tweaks to the library's Lynae -> LynaePeppermint fix, the way an API user makes them: the fix is a plain list of edits
# (wwmiSteps), the same for every mod, so a tweak is a list operation with the library's own generic edits.
#
# The list, in order (print it with --show):
#   copies, toggles, slotRemap                       -- the mod's sections, copied and remapped onto the target's slots
#   textureListCalls, sharedBypass, modTexLines,     -- edits on a TARGET object, (target mod, "component<slot>")
#   additions, rerouteFilters, removedLines, rabbitFXPins, slotValues, hashes, dispatchSize, shapeKeyLists,
#   afterShaders, markLines, overlays, rerouteShaders, offPasses, blend, rebind
#   fixSections                                      -- the fix's own sections (its texture command lists, resources,
#                                                       shaders) put into graph (0, "", "fixSections")
#   shaderFiles, fixFiles, latch, texEdits           -- the edits on those sections
#
# The tweaks (each on by default; --only <name> runs one):
#   remap    the mod's component 0 drawn on the target's slot 1, and component 1 on slot 2
#   blue     the upper body's diffuse all blue: a ResRegCollect over the fix's texture lists, with a texture edit
#   orfix    an ORFix call on the upper body (component 3)
#   nodiff   the lower body (component 4) without its diffuse
#
#   py -3 lynaeTweaks.py <mod folder> [--only remap|blue|orfix|nodiff] [--show]
import argparse
import os
import sys

APISrc = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "..", "Anime Game Remap (for all users)", "api", "src", "py")
sys.path.insert(0, APISrc)
import FixRaidenBoss2 as AGR    # noqa: E402

TARGET = "LynaePeppermint"
UPPER = (TARGET, "component3")
FIX_SECTIONS = ("", "fixSections")                     # the graph of the fix's own sections, as a mod object
FIX_SECTIONS_ID = (0,) + FIX_SECTIONS                  # ... and as a graph id


def at(steps, name):
    """The index right after the last step called 'name'"""
    return max(i for i, step in enumerate(steps) if step.name == name) + 1


def inSection(prefix):
    """A key filter: every line of the sections whose name starts with 'prefix', nothing of the others"""
    return lambda data, modType, ini: AGR.Ranges.createFull() if data.sectionName.startswith(prefix) else AGR.Ranges.createEmpty()


args = argparse.ArgumentParser()
args.add_argument("folder")
args.add_argument("--only", choices = ["remap", "blue", "orfix", "nodiff"])
args.add_argument("--show", action = "store_true", help = "print the list")
args = args.parse_args()
wanted = lambda name: args.only in (None, name)

config = AGR.WWMIFixerConfig.ofFactory(AGR.ModTypes["Lynae"].value.iniFixBuilder.factoryOf("Lynae", TARGET))
steps = AGR.wwmiSteps(config)

# ---- remap: the slot remap's own table, retargeted
if wanted("remap"):
    remap = steps[at(steps, "slotRemap") - 1]
    moves = {"component0": "component1", "component1": "component2"}
    table = remap.remap
    for source, targets in table.items():
        if source[2] in moves:
            table[source] = [(t[0], t[1], moves[source[2]]) + tuple(t[3:]) for t in targets]
    remap.remap = table

# ---- blue: the upper body's diffuse is what its texture list binds at ps-t3 (and its first pass's list at ps-t0)
if wanted("blue"):
    blue = AGR.ResRegCollect(
        {FIX_SECTIONS_ID: ["ps-t3", "ps-t0"]},
        {"blue": AGR.TexReplace((0, "", "blueUpperDiffuse"),
                                AGR.TexEditor(filters = [AGR.CppColourReplaceFilter(AGR.CppColour(0, 0, 255, 255), replaceAlpha = False)]),
                                resSubType = "BlueUpper")},
        resPredicates = {FIX_SECTIONS_ID: lambda reg, value, data: data.sectionName.startswith("CommandListLynaeComponent3Textures")})
    steps.insert(at(steps, "fixSections"), blue)

# ---- orfix: an ORFix call before each of the upper body's draws, once per path through its graph
if wanted("orfix"):
    call = AGR.RegDelimitedAdd(("run", "CommandList\\global\\ORFix\\ORFix"), delimiterRegs = {"drawindexed": None},
                               mode = AGR.RegDelimitedAddMode.PerPath)
    steps.insert(at(steps, "additions"), AGR.GraphGroupEdit([{UPPER: [call]}]))

# ---- nodiff: the lower body's texture list without its diffuse (ps-t3, bound directly or through a list it calls)
if wanted("nodiff"):
    steps.insert(at(steps, "fixSections"), AGR.GraphGroupEdit(
        [{FIX_SECTIONS: [AGR.RegRemove({"ps-t3": None, "run": AGR.RegRemove.valueStartsWith("CommandListLynaeLowerDiffusePst3")})]}],
        keyFilters = [{FIX_SECTIONS: [inSection("CommandListLynaeComponent4Textures")]}]))

if args.show:
    print([step.name for step in steps])

AGR.CppStrategyOverrides.setFixer("Lynae", TARGET, AGR.makeWWMIFixer(config, steps))
service = AGR.RemapService(path = os.path.abspath(args.folder), downloadMode = AGR.DownloadMode.Disabled)
service.fix()
for path, error in service.stats.ini.skipped.items():
    print("SKIPPED", path, error)
AGR.CppStrategyOverrides.clear()
