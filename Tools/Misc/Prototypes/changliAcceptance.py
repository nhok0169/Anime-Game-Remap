# CHANGLI -> CHANGLINYMPH, WRITTEN FROM THE API WITH WWMIParser / WWMIFixer
#
# The acceptance test of the WWMI refactor for one more character: the pair's parser and fixer written by an API user --
# a WWMIParserConfig and a WWMIFixerConfig set up in Python and handed to makeWWMIParser / makeWWMIFixer -- without reading
# the shipped configs (ofFactory). Depends on nothing of another pair's.
#
# What the GAME decides is read from the API's versioned asset tables, at the run's versions: her texture roles by hash
# (Hashes); every register a binding is made at (TextureRegisters: the skin's slot's register for the role her part plays
# there, its other passes' <role>@other, her own draws' registers for her shaders); the skin's passes, meshes and tags
# (DrawPasses, Hashes); the made textures' colours (MaterialColours); the downloads; and -- by withGameData, which both
# templates call -- passes, tags, shaders, shape keys, vertex group maps, numberings, centroids and thumbprints. WWMI's and
# RabbitFX's own names are IniKeywords. What this file holds is the pair's CHOICES; game data with no table yet is marked
# "(no table yet)".
#
#   py -3 changliAcceptance.py <mod folder> [--fromVersion X] [--version Y] [--download normal]
#   py -3 changliAcceptance.py <mod folder> --compiled     # the shipped fix, for an A/B
import argparse
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "..", "Anime Game Remap (for all users)", "api", "src", "py"))
import FixRaidenBoss2 as AGR    # noqa: E402

SOURCE, TARGET = "Changli", "ChangliNymph"
S, T = AGR.ModTypes[SOURCE].value, AGR.ModTypes[TARGET].value
C = AGR.WWMIFixerConfig
V = AGR.Version.parse
VB0 = lambda modType: AGR.WWMIFixerConfig.vb0VersionsOf(modType)            # her models' versions
OLDEST = lambda modType: min(VB0(modType).values(), key = V)
NEWEST = lambda modType: max(VB0(modType).values(), key = V)


# ================================================================================================= her textures
# her roles; which are masks (a flat mask marks no region, so it is no candidate for one)
ROLES = ["bangsMap", "bangsDiffuse", "bangsStrand", "bangsOutline", "hairMap", "hairDiffuse", "hairStrand", "hairOutline",
         "faceMask", "faceDiffuse", "upperNormal", "upperMask", "upperDiffuse", "upperDetail", "upperRamp", "lowerNormal",
         "lowerMask", "lowerDiffuse", "coatNormal", "coatDiffuse", "coatOutline", "eyeIris", "eyeMask", "eyeDiffuse",
         "eyeHighlight", "glow"]
REGION_ROLES = {"upperMask", "lowerMask", "faceMask"}
# her game textures a fix binds when the mod has none, beyond her ROLES: what her own game shaders read
SHADER_READS = ["upperShared", "upperSubsurface", "tattooNoise", "tattooFlow"]
# a role the mod has no texture for takes her game texture, downloaded -- but these, which the skin's slot draws with its own
GAME_LEFT = ["hairMap", "eyeIris", "eyeMask", "eyeDiffuse"]
# the skin's blindfold, as a download of a file made for it (no table yet: a file, not a game hash)
BLINDFOLD = ("blindfold", "BlindfoldOnNymph")


def textureFacts(version):
    """Her texture facts at a version: each role's hashes (every generation Hashes files her under), where her own draws bind
    each role (TextureRegisters: her main pass and RabbitFX's keys), and the downloads of her game textures"""
    facts = AGR.WWMITextureFacts()
    hashRows = S.hashes.repo.toNestedDict()
    facts.roles = {textureHash.lower(): role for names in hashRows.values() for hashType, textureHash in names.get(SOURCE, {}).items()
                   for role in [AGR.WWMITextureFacts.roleOfHashType(hashType, set(ROLES))] if role is not None}

    registers = {}
    for names in S.textureRegisters.repo.toNestedDict().values():
        for component, rows in names.get(SOURCE, {}).items():
            slot = AGR.WWMIParserConfig.slotWithPrefix(("", component), AGR.WWMINames.SlotPrefix)
            if slot is None:
                continue                           # a mesh of hers besides her slots (her blindfold)
            for roleType, reg in rows.items():
                role = roleType.split("@")[0]
                if role in ROLES and (roleType == role or roleType == role + "@rabbitfx"):
                    registers.setdefault(slot, {})[reg] = role
    facts.registerRoles = registers

    downloads = {role: S.hashes.get([SOURCE, role], None, False) for role in ROLES + SHADER_READS if role not in GAME_LEFT}   # her current ones
    facts.fallbackTextures = {**{role: textureHash for role, textureHash in downloads.items() if textureHash is not None}, BLINDFOLD[0]: BLINDFOLD[1]}
    folder = max((S.hashes.versionOf(h, [SOURCE, role]) for role, h in downloads.items() if h is not None), key = lambda v: v)
    facts.downloadVersionFolder = AGR.DownloadTools.versionFolder(folder)
    facts.regionRoles = REGION_ROLES
    return facts


# ================================================================================================= the parser
def parserConfig():
    config = AGR.WWMIParserConfig()
    config.version = OLDEST(S)                     # her parser reads mods of every model of hers
    config.textures = textureFacts(V(NEWEST(S)))
    config.hashOnlyObjs = list(config.hashOnlyObjs) + [("blindfoldMesh", "blindfoldMesh")]   # ...and her blindfold, a mesh of its own
    return config


# ================================================================================================= the fixer
K = AGR.IniKeywords
SLOT = lambda n: AGR.WWMINames.SlotPrefix + str(n)


def need(value, what):
    """A table row the fix cannot do without: missing, it says which rather than binding a wrong register"""
    if value is None:
        raise LookupError("no asset table row for " + what)
    return value


def component(slot, bindings, sameFile = False):
    entry = C.SourceComponent(slot, bindings)
    entry.sameFileSection = sameFile
    return entry


def fixerConfig(fromVersion, toVersion):
    FROM, TO = V(fromVersion), V(toVersion)
    # a binding of one of her roles at the register the skin's slot binds the role her part plays there (TextureRegisters;
    # kind "@other": on the slot's other pass) -- or, for her own shaders, at the register her own draw binds it
    onTarget = lambda slot, roles, src = -1, kind = "": [
        C.Binding(need(T.getTextureRegister(SLOT(slot), role + kind, TO), TARGET + " " + SLOT(slot) + " " + role + kind), srcRole, src)
        for srcRole, role in roles.items()]
    onSource = lambda comp, roles: [C.Binding(need(S.getTextureRegister(SLOT(comp), role, FROM), SOURCE + " " + SLOT(comp) + " " + role), role)
                                    for role in roles]

    config = C()
    config.targetId = AGR.ModTypeId.ChangliNymph
    config.version, config.sourceVersion = toVersion, fromVersion
    config.sourceVersionFromVb0 = True             # a mod's own vb0 says which model of hers it is

    # ---- each source component -> the skin's slot, and the role of the skin's each of her roles plays there: her bangs,
    # hair, face and eyes onto their own slots; her upper body, legs, coat and tattoo all through the skin's BODY slot (the
    # skirt slot draws with a G-buffer shader in the overworld, which cut her parts away), in the body shader's layout -- her
    # coat a made flat cloth mask (the body's glow map is left to the game)
    body = lambda normal, mask, diffuse: {normal: "bodyNormal", mask: "bodyMask", diffuse: "bodyDiffuse"}
    config.plan = {
        0: component(0, onTarget(0, {"bangsMap": "bangsMap", "bangsDiffuse": "bangsDiffuse", "bangsStrand": "bangsStrand"})),
        1: component(1, onTarget(1, {"hairMap": "hairMap", "hairDiffuse": "hairDiffuse", "hairStrand": "hairStrand"})),
        2: component(2, onTarget(2, {"faceMask": "faceMask", "faceDiffuse": "faceDiffuse"})),
        3: component(4, onTarget(4, body("upperNormal", "upperMask", "upperDiffuse"))),
        4: component(4, onTarget(4, body("lowerNormal", "lowerMask", "lowerDiffuse")), True),
        5: component(4, onTarget(4, body("coatNormal", "flatClothMask", "coatDiffuse")), True),
        6: component(6, onTarget(6, {"eyeIris": "eyeIris", "eyeMask": "eyeMask", "eyeDiffuse": "eyeDiffuse"})),
        7: component(4, onSource(7, ["glow"]), True),
    }
    plan = config.plan
    # her hair kept off the skin's outline pass (its hair slot's other pass, DrawPasses)
    plan[1].offPasses = [need(T.getDrawPass(SLOT(1), "other.ps", TO), TARGET + " " + SLOT(1) + " other pass")]

    # ---- her upper body's glow drawn over the skin's body draw by HER upper-body shader (an overlay), and her tattoo by her
    # tattoo shader: their code and constants from GameShaders (withGameData), each texture at the register her own draw binds it
    upper = C.Shader.ofGame("ChangliUpper")
    upper.registers = onSource(3, ["upperNormal", "upperMask", "upperDiffuse", "upperDetail", "upperRamp", "upperShared", "upperSubsurface"])
    upper.lines = [(K.DepthStencilStateMerge, "true"), (K.DepthFunc, "equal"), (K.DepthWriteMask, "zero"),
                   *[(K.Mask + "[%d]" % n, "0") for n in range(1, 7)]]
    upper.onlyWith = K.WWMIColourTarget
    upper.yieldToRabbitFX = True
    plan[3].overlay = upper
    tattoo = C.Shader.ofGame("ChangliTattoo")
    tattoo.registers = onSource(7, ["glow", "tattooNoise", "tattooFlow"])
    tattoo.onlyWith = K.WWMIColourTarget
    plan[7].shader = tattoo
    config.plan = plan

    # ---- what each slot's OTHER pass (its outline; the eyes' highlight) binds
    config.extraPassRegs = {
        0: {"other": onTarget(0, {"bangsOutline": "bangsOutline"}, kind = "@other")},
        4: {"other": [b for src, role in ((3, "upperDiffuse"), (4, "lowerDiffuse"), (5, "coatOutline")) for b in onTarget(4, {role: "bodyDiffuse"}, src, "@other")]},
        6: {"other": onTarget(6, {"eyeHighlight": "eyeHighlight"}, kind = "@other")},
    }

    # ---- her blindfold: the skin draws its own on a mesh of its own (Hashes' "blindfoldMesh"), which takes the skin's blindfold
    # download -- the mesh's pass, its vertex shaders and their tags from DrawPasses, its register from TextureRegisters
    mesh = need(T.hashes.get([TARGET, "blindfoldMesh"], TO, False), TARGET + " blindfold")
    meshPass = need(T.getDrawPass("blindfoldMesh", "main.ps", TO), TARGET + " blindfold pass")
    config.sharedMeshes = {mesh: {meshPass: [C.Binding(need(T.getTextureRegister("blindfoldMesh", "blindfold", TO), TARGET + " blindfold register"), "blindfold")]}}
    config.meshCounterparts = ["blindfoldMesh"]
    meshShaders = list(T.getDrawPassShaders("blindfoldMesh", "main", TO))
    config.passVertexShaders = {meshPass: meshShaders}
    config.filterIndices = dict(zip(meshShaders, T.getDrawPassTags("blindfoldMesh", "main", TO)))

    # ---- the textures the fix makes, each in the colour the skin's material reads as it (MaterialColours)
    config.createdTextures = [C.CreatedTexture("flatClothMask", material = (SLOT(4), "bodyMask.cloth"))]
    # ---- her bangs' outline, brightened for the skin's outline pass (no table yet: the gain)

    config.sourceTextures = textureFacts(FROM)

    # (the lines a remapped section drops are the default: WWMIFixerConfig.gameRemovedRegs)

    # ---- the skin's skeleton is past 256 bones and reads 8 influences a vertex (no table yet: both numbers); her shape keys
    # are retargeted
    config.cleanTexcoords = True
    config.carryByRegisterRole = True
    config.targetInfluences = 8

    config.sourceLabels = {0: "bangs", 1: "hair", 2: "face", 3: "upper body", 4: "legs and skirt", 5: "coat and sashes", 6: "eyes",
                           7: "chest flame tattoo"}
    config.targetLabels = {0: "bangs", 1: "hair", 2: "face", 3: "sheer dress", 4: "body", 5: "skirt and veils", 6: "eyes", 7: "shawl"}
    return config


# ================================================================================================= the run
if __name__ == "__main__":
    args = argparse.ArgumentParser()
    args.add_argument("folder")
    args.add_argument("--fromVersion")
    args.add_argument("--version")
    args.add_argument("--download", default = "disabled", choices = ["disabled", "normal"])
    args.add_argument("--compiled", action = "store_true", help = "run the shipped fix instead")
    args = args.parse_args()

    FROM, TO = args.fromVersion or NEWEST(S), args.version or str(AGR.WWMIFixerConfig.newestVersionOf(T))   # WWMIFixer::toVersion
    if not args.compiled:
        AGR.CppStrategyOverrides.setParser(SOURCE, AGR.makeWWMIParser(parserConfig()))
        AGR.CppStrategyOverrides.setFixer(SOURCE, TARGET, AGR.makeWWMIFixer(fixerConfig(FROM, TO)))
    service = AGR.RemapService(path = os.path.abspath(args.folder), toModTypeIds = {int(AGR.ModTypeId.ChangliNymph)},
                               downloadMode = AGR.DownloadMode.Normal if args.download == "normal" else AGR.DownloadMode.Disabled,
                               fromVersion = V(args.fromVersion) if args.fromVersion else None, toVersion = V(args.version) if args.version else None)
    service.fix()
    for path, error in service.stats.ini.skipped.items():
        print("SKIPPED", path, error)
    AGR.CppStrategyOverrides.clear()
