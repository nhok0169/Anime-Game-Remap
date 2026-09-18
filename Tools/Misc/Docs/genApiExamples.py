"""
Regenerates the Input / Code / Result dropdowns of Docs/src/apiExamples.rst from the Integration
Tester's API docs tests: inputs from Tests/APIDocsTests/inputs, results from the expected_* goldens.
The prose around each example is kept; only the dropdowns are rebuilt.

    python genApiExamples.py            dry run: prints how many lines each section would get
    python genApiExamples.py --write    writes the .rst

Run it AFTER the goldens are regenerated (on Linux, see AI Agent Help/Testing/CLAUDE.md's
"Integration Tester"), then build the docs. Each example is one entry in SECTIONS, keyed by the
section's TITLE in the .rst: a new example needs a new test in ApiDocTests.py AND a new entry here,
and a title that is not found fails the run rather than being skipped. The Code block is written
here by hand (the tests' own scripts carry tester plumbing); keep it saying the same thing as the
test script it stands for.
"""

import os, re, sys

REPO = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", ".."))
RST = os.path.join(REPO, "Docs", "src", "apiExamples.rst")
TESTS = os.path.join(REPO, "Testing", "Integration Tester", "IntegrationTester", "Tests", "APIDocsTests")
INPUTS = os.path.join(TESTS, "inputs")

SKIP_FILES = re.compile(r"(RemapFixLog\.txt|summaryLog\.txt)$")


def golden(test):
    path = os.path.join(TESTS, f"expected_{test}")
    assert os.path.isdir(path), f"missing golden: {path}"
    return path


def readText(path):
    with open(path, "rb") as f:
        txt = f.read().decode("utf-8")
    return txt.replace("\r\n", "\n").replace("\t", "    ")


def indent(lines, n):
    pad = " " * n
    return [(pad + line) if line.strip() else "" for line in lines]


def codeBlock(lang, content, n, caption = None, linenoStart = None, emphasize = None, linenos = True):
    out = [" " * n + (f".. code-block:: {lang}" if lang else ".. code-block::")]
    if caption:
        out.append(" " * (n + 4) + f":caption: {caption}")
    if linenos:
        out.append(" " * (n + 4) + ":linenos:")
    if linenoStart:
        out.append(" " * (n + 4) + f":lineno-start: {linenoStart}")
    if emphasize:
        out.append(" " * (n + 4) + f":emphasize-lines: {emphasize}")
    out.append("")
    body = content.rstrip("\n").split("\n")
    while body and not body[0].strip():
        body.pop(0)
    out += indent(body, n + 4)
    out.append("")
    return out


def dropdown(title, body, n = 0, isOpen = False):
    out = [" " * n + f".. dropdown:: {title}"]
    if isOpen:
        out.append(" " * (n + 4) + ":open:")
    out.append(" " * (n + 4) + ":animate: fade-in-slide-down")
    out.append("")
    out += body
    out.append("")
    return out


# ---------------------------------------------------------------- file trees

def listTree(root, scriptRel):
    """(dirs, files) of 'root', with the test's own script shown as example.py and any other test script hidden"""
    entries = {}
    for dirpath, dirs, files in os.walk(root):
        rel = os.path.relpath(dirpath, root)
        rel = "" if rel == "." else rel.replace(os.sep, "/")
        for f in files:
            frel = f"{rel}/{f}" if rel else f
            if SKIP_FILES.search(f):
                continue
            if f.endswith(".py"):
                if frel != scriptRel:
                    continue
                frel = f"{rel}/example.py" if rel else "example.py"
            entries[frel] = True
    return sorted(entries)


def renderTree(rootName, paths):
    tree = {}
    for p in paths:
        node = tree
        parts = p.split("/")
        for part in parts[:-1]:
            node = node.setdefault(part + "/", {})
        node[parts[-1]] = None

    lines = [rootName]

    def walk(node, prefix):
        keys = sorted(node, key = lambda k: (not k.endswith("/"), k.lower()))
        for i, key in enumerate(keys):
            last = (i == len(keys) - 1)
            lines.append(prefix + "|")
            lines.append(prefix + "+--> " + key.rstrip("/"))
            if key.endswith("/"):
                walk(node[key], prefix + (" " if last else "|") + " " * 6)

    walk(tree, "")
    emphasize = next((i + 1 for i, line in enumerate(lines) if line.endswith("+--> example.py")), None)
    return "\n".join(lines), emphasize


def iniFiles(root, paths):
    names = [p for p in paths if p.lower().endswith(".ini")]
    base = [os.path.basename(p) for p in names]
    return [(p if base.count(os.path.basename(p)) > 1 else os.path.basename(p), os.path.join(root, *p.split("/")))
            for p in names]


def folderDropdown(label, root, rootName, scriptRel, intro, iniIntro):
    paths = listTree(root, scriptRel)
    tree, emphasize = renderTree(rootName, paths)
    body = [f"    {intro}", ""]
    body += dropdown("File Structure", codeBlock("", tree, 8, emphasize = emphasize, linenos = False), 4)
    body += ["    :raw-html:`<br />`", "", f"    {iniIntro}", ""]
    for title, path in iniFiles(root, paths):
        body += dropdown(title, codeBlock("ini", readText(path), 8, caption = os.path.basename(path)), 4)
    return dropdown(label, body)


# ---------------------------------------------------------------- sections

def pyVar(script, var):
    txt = readText(os.path.join(INPUTS, script))
    m = re.search(rf'^{var} = r"""(.*?)"""', txt, re.S | re.M)
    assert m, (script, var)
    return f'{var} = r"""{m.group(1)}"""'


def codeDropdown(code, caption = "example.py", linenoStart = None):
    return dropdown("Code", codeBlock("python", code, 4, caption = caption, linenoStart = linenoStart), isOpen = True)


def iniResults(test, iniName, intro):
    root = golden(test)
    stem = iniName[:-4]
    names = sorted(f for f in os.listdir(root) if f == iniName or re.fullmatch(re.escape(stem) + r"RemapFix\d+\.ini", f))
    assert iniName in names, (test, iniName)
    body = [f"    {intro}", ""] if intro else []
    for name in names:
        if len(names) == 1:
            body += codeBlock("ini", readText(os.path.join(root, name)), 4, caption = name)
        else:
            body += dropdown(name, codeBlock("ini", readText(os.path.join(root, name)), 8, caption = name), 4)
    return dropdown("Result", body)


def pathIniSection(test, iniName, code, resultIntro = None):
    out = dropdown("Input", codeBlock("ini", readText(os.path.join(INPUTS, iniName)), 4, caption = iniName))
    out += codeDropdown(code, caption = None)
    out += iniResults(test, iniName, resultIntro)
    return out


def strIniSection(test, script, var, iniName, code, resultIntro):
    inputCode = pyVar(script, var)
    out = dropdown("Input", codeBlock("python", inputCode, 4))
    out += codeDropdown(code, caption = None, linenoStart = inputCode.count("\n") + 2)
    out += iniResults(test, iniName, resultIntro)
    return out


def folderSection(test, rootRel, rootName, scriptRel, code, resultIntro, inputFrom = None):
    inputRoot = os.path.join(golden(inputFrom), *rootRel.split("/")) if inputFrom else os.path.join(INPUTS, *rootRel.split("/"))
    outputRoot = os.path.join(golden(test), *rootRel.split("/"))
    out = folderDropdown("Input", inputRoot, rootName, scriptRel, "Assume we have this file structure:",
                         "Assume below is the content of the .ini files")
    out += codeDropdown(code)
    out += folderDropdown("Result", outputRoot, rootName, scriptRel, resultIntro, "Below is the new content of the .ini files")
    return out


IMPORT = "import AnimeGameRemap as AGR\n\n"

KIRARA_CODE = IMPORT + '''RegRef = AGR.GIMICharFixerConfig.RegRef
RegValChecks = AGR.GIMICharFixerConfig.RegValChecks
TexEdit = AGR.GIMICharFixerConfig.TexEdit

ORFix = AGR.IniKeywords.ORFixPath.value
NNFix = r"CommandList\\global\\ORFix\\NNFix"
TexFx = r"CommandList\\TexFx\\TN.0"


# Edit Kirara's body so that her body's skin tone matches with her face
#
# -- Notes --:
# If you do not like how we edit her body, you can play around with her BodyDiffuse.dds or her BodyLightMap.dds
#   in your favourite image editor (Paint.net, Photoshop, etc...) or you can tweak the code below
#
# A filter is given the texture itself, so it edits the texture in place
def darkenDiffuse(texFile):
    AGR.GammaFilter(AGR.ColourConsts.SRGBGamma.value).transform(texFile)

def makeFaceOpaque(texFile):
    pixels = bytearray(texFile.getPixels())
    pixels[3::4] = bytes([1]) * (len(pixels) // 4)
    texFile.setPixels(bytes(pixels), texFile.width, texFile.height)

def reflectionKeys(obj: str):
    return [f"ResourceRef{obj}Diffuse", f"ResourceRef{obj}LightMap", "$CharacterIB"]


# ==== Override how Kirara is fixed ======

config = AGR.GIMICharFixerConfig()
config.drawnObjs = ["head", "body", "dress"]

# Kirara's body is drawn a second time as part of KiraraBoots' head, while KiraraBoots' own body is hidden
config.objSplits = [("head", ["head"]), ("body", ["body", "head"]), ("dress", ["dress"])]
config.objNewRegVals = [("body", [("ib", "null")])]

# only darken the copy of Kirara's body that is drawn as KiraraBoots' head
config.texEdits = [TexEdit("head", "ps-t1", "DarkenDiffuse", darkenDiffuse, srcObj = "body"),
                   TexEdit("face", "ps-t0", "OpaqueFaceDiffuse", makeFaceOpaque, toReg = "ps-t1")]

# ---- the rest is the same as the default fix ----
config.objRegRemovals = [("head", reflectionKeys("Head")), ("body", reflectionKeys("Body")), ("dress", reflectionKeys("Dress"))]
config.objRegRemaps = [("dress", [("ps-t1", [RegRef("ps-t0", RegValChecks.isDiffuse)], True),
                                  ("ps-t2", [RegRef("ps-t1", RegValChecks.isLightMap)], True)])]
config.objFixCalls = [("head", [ORFix, TexFx]), ("body", [ORFix, TexFx]), ("dress", [NNFix, TexFx])]

AGR.CppStrategyOverrides.setFixer("Kirara", "KiraraBoots", AGR.makeGIMICharFixer(config))

# ========================================

# fix the mod
remapService = AGR.RemapServiceCLI(verbose = False, keepBackups = False)
remapService.fix()

# go back to the default fix
AGR.CppStrategyOverrides.clear()
'''

PRINT_ALL = "\nfor fixedTxt in fixedResult.values():\n    print(fixedTxt)\n"

SECTIONS = {
    "Only Fix a .ini File Given the File Path":
        lambda: pathIniSection("iniFileFromFilePath_iniFileFixed", "CuteLittleRaiden.ini",
                               IMPORT + 'iniFile = AGR.IniFile("CuteLittleRaiden.ini")\niniFile.parse()\niniFile.fix()\n'),

    "Only Fix .Ini file Given Only a String Containing the Content of the File":
        lambda: strIniSection("iniFileFromStr_iniFileFixed", "iniFileFromStr_iniFileFixed.py", "shortWackyRaidenIniTxt", "CuteLittleRaiden.ini",
                              IMPORT + "iniFile = AGR.IniFile(txt = shortWackyRaidenIniTxt)\niniFile.parse()\nfixedResult = iniFile.fix()\n" + PRINT_ALL,
                              "The printed text of the fixed .ini file"),

    "Remove a Fix from a .ini File Given the File Path":
        lambda: pathIniSection("iniPath_iniFixRemoved", "PartiallyFixedRaiden.ini",
                               IMPORT + 'iniFile = AGR.IniFile("PartiallyFixedRaiden.ini")\niniFile.removeFix(keepBackups = False)\n'),

    "Remove a Fix from a .ini File Given Only a String Containing the Content of the File":
        lambda: strIniSection("iniStr_iniFixRemoved", "iniStr_iniFixRemoved.py", "showWackyRaidenIniTxtWithFix", "IniWithFixRemoved.ini",
                              IMPORT + "iniFile = AGR.IniFile(txt = showWackyRaidenIniTxtWithFix)\nfixCode = iniFile.removeFix(keepBackups = False)\n\nprint(fixCode)\n",
                              "The printed text with the fix removed"),

    "Fix a .ini File Given the File Path":
        lambda: pathIniSection("iniPath_prevRemovedIniFixed", "PartiallyFixedRaiden.ini",
                               IMPORT + 'iniFile = AGR.IniFile("PartiallyFixedRaiden.ini")\niniFile.removeFix(keepBackups = False)\niniFile.parse()\niniFile.fix()\n'),

    "Fix a .ini File Given Only A String Containing the Content of the File":
        lambda: strIniSection("iniStr_prevRemovedIniFixed", "iniStr_prevRemovedIniFixed.py", "showWackyRaidenIniTxtWithFix", "FixedIni.ini",
                              IMPORT + "iniFile = AGR.IniFile(txt = showWackyRaidenIniTxtWithFix)\niniFile.removeFix(keepBackups = False)\niniFile.parse()\nfixedResult = iniFile.fix()\n" + PRINT_ALL,
                              "The printed text of the fixed .ini file"),

    "Fixing a .ini File Without Showing the Mod on the Original Character":
        lambda: pathIniSection("iniPath_hideOrig", "changeVersionKeqing.ini",
                               IMPORT + 'iniFile = AGR.IniFile("changeVersionKeqing.ini")\niniFile.parse()\niniFile.fix(hideOrig = True)\n'),

    "Fixing a .ini File to a Specific Version of the Game":
        lambda: pathIniSection("iniPath_toOldVersion", "changeVersionKeqing.ini",
                               IMPORT + 'version = AGR.CppVersion.parse("4.0")\n\n'
                               '# fromVersion: the version the mod was made for, toVersion: the version to fix the mod to\n'
                               'iniFile = AGR.IniFile("changeVersionKeqing.ini", fromVersion = version, toVersion = version)\niniFile.parse()\niniFile.fix()\n'),

    "Fixing Many Mods":
        lambda: folderSection("fullFix_modFixed", "fullFix/RaidenShogun", "RaidenShogun", "Mod/pythonScript/Run/fullFix_modFixed.py",
                              IMPORT + 'fixService = AGR.RemapServiceCLI(path = "../../", verbose = False, keepBackups = False)\nfixService.fix()\n',
                              "Contains the fixed files for the mods."),

    "Undo the Fix from Many Mods":
        lambda: folderSection("fullFix_modFixUndoed", "fullFix/RaidenShogun", "RaidenShogun", "Mod/pythonScript/Run/fullFix_modFixUndoed.py",
                              IMPORT + 'fixService = AGR.RemapServiceCLI(path = "../../", verbose = False, keepBackups = False, undoOnly = True)\nfixService.fix()\n',
                              "Below contains the new content with the previous changes made by the script removed",
                              inputFrom = "fullFix_modFixed"),

    "Override the Default Remap for a Character":
        lambda: folderSection("iniPath_ImplOverride", "overrideFix", "Mods", "iniPath_ImplOverride.py", KIRARA_CODE,
                              "Below contains the new content with the alternative fix for Kirara applied"),

    "Forcibly remap a mod to a different character":
        lambda: folderSection("iniPath_forcedType", "overrideFix", "Mods", "iniPath_forcedType.py",
                              IMPORT + 'fixService = AGR.RemapServiceCLI(verbose = False, keepBackups = False, forcedType = "rosaria")\nfixService.fix()\n',
                              "Below contains the new content with the fix for Rosaria applied onto the Kirara mod"),

    "Remap Only a Few Selected Characters":
        lambda: folderSection("fullFix_someFix", "multiFix/select", "Mods", "fullFix_someFixed.py",
                              IMPORT + 'fixService = AGR.RemapServiceCLI(verbose = False, keepBackups = False, types = ["kequeen", "aMbEr", "ACTINGGRANDMASTER"])\nfixService.fix()\n',
                              "Below contains the new content with only the mods for Keqing, Jean and Amber fixed"),

    "Fixing Entire Mods Without Showing Mods on the Original Character":
        lambda: folderSection("fullFix_hideOrig", "multiFix/oldVers", "AmberCN", "fullFix_hideOrig.py",
                              IMPORT + 'fixService = AGR.RemapServiceCLI(verbose = False, keepBackups = False, types = ["BaronBunnyCN"], hideOrig = True)\nfixService.fix()\n',
                              "Below contains the new content with the mod only shown on the remapped character, and not on the original character"),

    "Fixing Entire Mods to a Specific Version of the Game":
        lambda: folderSection("fullFix_oldVers", "multiFix/oldVers", "AmberCN", "fullFix_oldVers.py",
                              IMPORT + '# fromVersion: the version the mods were made for, version: the version to fix the mods to\n'
                              'fixService = AGR.RemapServiceCLI(verbose = False, keepBackups = False, types = ["BaronBunnyCN"], version = "4.0", fromVersion = "4.0")\nfixService.fix()\n',
                              "Below contains the new content with the fix applied for the game version 4.0"),
}

def main():
    raw = open(RST, "rb").read().decode("utf-8")
    lines = raw.replace("\r\n", "\n").split("\n")

    def titleAt(i):
        return i + 1 < len(lines) and lines[i].strip() and re.fullmatch(r"[~\-=]{3,}", lines[i + 1]) and len(lines[i + 1]) >= len(lines[i].rstrip())

    titles = [i for i in range(len(lines)) if titleAt(i)]
    found = set()

    # work bottom-up so earlier indices stay valid
    for idx in reversed(range(len(titles))):
        start = titles[idx]
        end = titles[idx + 1] if idx + 1 < len(titles) else len(lines)
        title = lines[start].strip()

        if title not in SECTIONS:
            continue
        found.add(title)

        section = lines[start:end]
        first = next(i for i, l in enumerate(section) if l.startswith(".. dropdown:: Input"))
        result = next(i for i, l in enumerate(section) if l.startswith(".. dropdown:: Result"))
        # the Result dropdown runs until the next non-blank line at column 0
        stop = next((i for i in range(result + 1, len(section)) if section[i] and not section[i][0].isspace()), len(section))

        new = SECTIONS[title]()
        print(f"{title}: {stop - first} old lines -> {len(new)} new lines")
        section[first:stop] = new
        lines[start:end] = section

    missing = set(SECTIONS) - found
    assert not missing, f"sections not found: {missing}"

    txt = "\n".join(lines)

    if "--write" in sys.argv:
        open(RST, "wb").write(txt.replace("\n", "\r\n").encode("utf-8"))
        print("written")


if __name__ == "__main__":
    main()
