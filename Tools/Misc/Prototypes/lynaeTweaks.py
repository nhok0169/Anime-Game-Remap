# Tweaks to the library's Lynae -> LynaePeppermint fix, the way an API user makes them: the fix is a list of edits, built
# for each .ini from the pair's config (WWMIFixerConfig) and what the mod draws, and handed to a rewrite with
# CppIniFixFactory.withList -- so a tweak is a change of the config's choices, or a list operation with the library's own
# generic edits.
#
# The list, in order (print it with --show):
#   copies, toggles, slotRemap                       -- the mod's sections, copied and remapped onto the target's slots
#   textureListCalls, sharedBypass, modTexLines,     -- edits on a TARGET object, (target mod, "component<slot>")
#   additions, rerouteFilters, removedLines, rabbitFXPins, slotValues, hashes, dispatchSize, shapeKeyLists,
#   afterShaders, markLines, overlays, rerouteShaders, offPasses, blend, rebind
#   fixSections, textureLists, glowLists, ...       -- the fix's own sections, one step per kind, into graph
#                                                       (0, "", "fixSections")
#   shaderFiles, fixFiles, latch, texEdits           -- the edits on those sections
#
# The tweaks (each on by default; --only <name> runs one):
#   remap    the mod's component 0 drawn on the target's slot 1, and component 1 on slot 2: the plan's slots, which every
#            step reads (the slot remap, the slot's numbers, its pass guards and its texture lists alike)
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

# ---- remap: the plan's choice of slot for components 0 and 1
if wanted("remap"):
    plan = config.plan
    plan[0].slot, plan[1].slot = 1, 2
    config.plan = plan

# ---- blue: the upper body's diffuse is what its texture list binds at ps-t3 (and its first pass's list at ps-t0)
blue = AGR.ResRegCollect(
    {FIX_SECTIONS_ID: ["ps-t3", "ps-t0"]},
    {"blue": AGR.TexReplace((0, "", "blueUpperDiffuse"),
                            AGR.TexEditor(filters = [AGR.CppColourReplaceFilter(AGR.CppColour(0, 0, 255, 255), replaceAlpha = False)]),
                            resSubType = "BlueUpper")},
    resPredicates = {FIX_SECTIONS_ID: lambda reg, value, data: data.sectionName.startswith("CommandListLynaeComponent3Textures")})

# ---- orfix: an ORFix call before each of the upper body's draws, once per path through its graph
orfix = AGR.GraphGroupEdit([{UPPER: [AGR.RegDelimitedAdd(("run", "CommandList\\global\\ORFix\\ORFix"), delimiterRegs = {"drawindexed": None},
                                                         mode = AGR.RegDelimitedAddMode.PerPath)]}])

# ---- nodiff: the lower body's texture list without its diffuse (ps-t3, bound directly or through a list it calls)
nodiff = AGR.GraphGroupEdit(
    [{FIX_SECTIONS: [AGR.RegRemove({"ps-t3": None, "run": AGR.RegRemove.valueStartsWith("CommandListLynaeLowerDiffusePst3")})]}],
    keyFilters = [{FIX_SECTIONS: [inSection("CommandListLynaeComponent4Textures")]}])


def rewrite(steps):
    """The fixer's list for one .ini, with the tweaks put in"""
    steps = list(steps)
    if wanted("orfix"):
        steps.insert(at(steps, "additions"), orfix)
    # (the texture lists are written by the step named after them)
    if wanted("blue"):
        steps.insert(at(steps, "textureLists"), blue)
    if wanted("nodiff"):
        steps.insert(at(steps, "textureLists"), nodiff)
    if args.show:
        print([step.name for step in steps])
    return steps


AGR.CppStrategyOverrides.setFixer("Lynae", TARGET, AGR.makeWWMIFixer(config).withList(rewrite))
service = AGR.RemapService(path = os.path.abspath(args.folder), downloadMode = AGR.DownloadMode.Disabled)
service.fix()
for path, error in service.stats.ini.skipped.items():
    print("SKIPPED", path, error)
AGR.CppStrategyOverrides.clear()
