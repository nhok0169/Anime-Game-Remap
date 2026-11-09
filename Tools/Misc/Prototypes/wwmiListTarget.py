# THE TARGET OF THE PYTHON ACCEPTANCE TEST (design, 2026-11-07): the whole WWMI fix as one list of generic edits, the same
# for every mod. NOT RUNNABLE YET -- every name marked (new) is API this design adds. The C++ wwmiSteps(config) is to be
# this list, step for step, and an A/B over the 68-mod corpus proves the two identical.
#
# HOW A STEP GETS WHAT THE MOD HAS. A step's SHAPE -- the edit, the objects it is keyed by, where it writes -- comes from
# the config. The mod only decides VALUES (which lines a texture list holds, which run lines an addition adds). Those are
# produced when the step runs, by a named library function handed the run (new: FRB.FixRun -- the .ini, its parse data,
# the mod type, the target, the two versions), and the mod's readings (new: FRB.WWMIReading, plain bound data: what it
# draws, its reroutes, roles, texture lists, file groups, ... -- read once per .ini by config.reading(run) and filed on
# its parse data). Nothing is hidden on a fixer.
#
# GENERIC HOOKS THIS NEEDS (all new, each "a value given when the edit runs"):
#   G1  GraphGroupEdit.everyGroup        an object's edits wherever the object lands (groups are made by the remap)
#   G2  RegSurroundedAdd/RegAdd/RegNewVals/RegAnchorAdd/RegRemove accept a producer  (run) -> values  for their table
#   G3  GraphCreate.sectionsOf(group, run)
#   G4  ResRegCollect predicates / resource edits read the run
#   G5  GraphGroupRemap.remapAtRun        (BUILT)
import FixRaidenBoss2 as FRB

TARGET = "LynaePeppermint"
config = FRB.WWMIFixerConfig.ofFactory(FRB.ModTypes["Lynae"].value.iniFixBuilder.factoryOf("Lynae", TARGET))
W = FRB.WWMIData(config)        # (new) config-only names: target objects, list names, slot numbers by version


def onTargets(kind, edit):
    """One generic edit on every planned target object, in every group (G1)"""
    step = FRB.GraphGroupEdit([{W.targetObj(c): [edit(c)] for c in config.plan}], everyGroup = True)
    step.name = kind
    return step


steps = [
    # ---- the mod's sections, copied onto the target ----
    FRB.wwmiCopiesStep(config),          # GraphCopy + GraphRename + RegNewVals(hash) + GraphVarTake per meshCounterparts part
    FRB.wwmiTogglesStep(config),         # GraphCopy + GraphRename + RegRemove/RegRemap per toggle the readings list
    FRB.wwmiSlotRemapStep(config),       # GraphGroupRemap: config.plan, + reroutes / silent / other hashed objects at run

    # ---- edits on the target objects, by kind ----
    onTargets("textureListCalls", lambda c: FRB.RegSurroundedAdd(
        lambda run: [("run", W.textureListName(c))] if W.reading(run).textureLines("main", c) else [],
        afterRegs = {"match_first_index": None}, latest = False)),
    onTargets("sharedBypass",     lambda c: FRB.WWMI.bypass(c)),          # RegAnchorAdd + RegAdd, values from readings.bypasses
    onTargets("modTexLines",      lambda c: FRB.RegAnchorAdd.replaceKeys(config.texRegPrefix,
                                                                         lambda run: W.reading(run).modTexLines(c))),
    onTargets("drawTwins",        lambda c: FRB.WWMI.mirrorTwin(c)),      # RegAnchorAdd after drawindexed, base from drawRanges
    onTargets("additions",        lambda c: FRB.RegSurroundedAdd(lambda run: W.additions(run, c),
                                                                 afterRegs = W.sharedListRuns(c))),
    onTargets("rerouteFilters",   lambda c: FRB.RegRemove(lambda run: W.rerouteDrops(run, c))),
    onTargets("removedLines",     lambda c: FRB.RegRemove(lambda run: W.reading(run).removedLines(c))),
    onTargets("rabbitFXPins",     lambda c: FRB.RegAnchorAdd.after("run", "CommandList\\RabbitFX\\SetTextures",
                                                                   lambda run: W.rabbitFXPin(run, c))),
    onTargets("slotValues",       lambda c: FRB.RegNewVals(lambda run: W.slotNumbers(c, run.toVersion), addNewKVPs = False)),
    onTargets("hashes",           lambda c: FRB.RegAssetRemap(W.hashAssets(), fromVersion = "run", toVersion = "run")),
    onTargets("dispatchSize",     lambda c: FRB.RegNewVals(W.dispatchSizes())),
    onTargets("shapeKeyLists",    lambda c: FRB.RegNewVals(lambda run: W.shapeKeyRebinds(run))),
    onTargets("afterShaders",     lambda c: FRB.WWMI.runBeforeCleanup(lambda run: W.afterShaderRuns(run, c))),
    onTargets("markLines",        lambda c: FRB.WWMI.drawRunAdd(lambda run: W.markSections(run, c), "after")),
    onTargets("overlays",         lambda c: FRB.WWMI.drawRunAdd(lambda run: W.overlaySections(run, c), "after")),
    onTargets("rerouteShaders",   lambda c: FRB.WWMI.drawRunAdd(lambda run: W.rerouteShaderSections(run, c), "replace")),
    onTargets("offPasses",        lambda c: FRB.WWMI.drawRunAdd(lambda run: W.offPassRuns(run, c), "replace")),

    # ---- the blend ----
    FRB.wwmiBlendStep(config),           # ResRegCollect(everyGroup) + VGRemapBlendReplace, its lift chosen at run
    FRB.wwmiRebindStep(config),          # RegNewVals on the blend register, ref values only

    # ---- the fix's own sections, and the edits on them ----
    FRB.GraphCreate((0, "", "fixSections"), sectionsOf = lambda group, run: W.fixSections(run, group)),   # G3
    FRB.wwmiShaderFilesStep(config),     # ResRegCollect + FileAddCreate / FileRefAdd over the shader sections
    FRB.wwmiFixFilesStep(config),        # ResRegCollect + FileAddCreate / FileDownloadCreate / TexCreate
    FRB.wwmiLatchStep(config),           # RegIfTemplateAdd x2 on the merge-window lists
    *FRB.wwmiTexEditSteps(config),       # ResRegCollect + TexReplace per config.texEdits entry
]

FRB.CppStrategyOverrides.setFixer("Lynae", TARGET, FRB.makeWWMIFixer(config, steps))

# OPEN QUESTIONS IN THIS DESIGN, before any of it is built:
#  1. W.fixSections(run, group) is the ~30 kinds of section the fix writes (texture lists, shader sections, hide sections,
#     skeleton, resources). As one function it is the old program again; split per kind it is ~30 GraphCreate steps.
#  2. Steps like wwmiCopiesStep / wwmiBlendStep are a few generic edits each, built by a function -- the list shows the
#     function, not the edits, unless each is spelled out here (another ~60 lines).
#  3. The order the fix's sections are written in, and which group a copy lands in, must stay byte-identical (the sweep).
