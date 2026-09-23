##### Credits

# ===== Anime Game Remap (AG Remap) =====
# Authors: Albert Gold#2696, NK#1321
#
# if you used it to remap your mods pls give credit for "Albert Gold#2696" and "Nhok0169"
# Special Thanks:
#   nguen#2011 (for support)
#   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
#   HazrateGolabi#1364 (for being awesome, and improving the code)

##### EndCredits

"""The character list lives in FOUR places outside the library. This asks the library and diffs.

    python checkModTypeTables.py

`api/README.md` and `apiMirror/README.md` (markdown, and they must stay identical to each other),
`Docs/src/commandOpts.rst` (a list-table) and the Python `ModTypes` enum. Every one of them is
written by hand, and every one of them has been behind the library at some point: on 2026-09-20 the
tables carried a MISSPELLED character (`BarabaraSummertime`, which no `--types` argument could
match) and the enum was six characters behind, and on 2026-09-22 all four were missing the Citlali
and Chisa pairs -- two remaps' worth, neither noticed by reading.

Run it after adding a `ModTypeId`, and before closing out a remap.
"""
# ASK THE LIBRARY, then diff the four places the character list is written down by hand:
#   api/README.md, apiMirror/README.md (markdown, must stay identical to each other),
#   Docs/src/commandOpts.rst (a list-table), and the Python ModTypes enum.
import os, re, sys
Repo = r"E:\Computer\Games\Genshin\Repos\Repos\Fix-Raiden-Boss"
Api = os.path.join(Repo, "Anime Game Remap (for all users)", "api", "src", "py")
sys.path.insert(0, Api)
os.add_dll_directory(os.path.join(Api, "FixRaidenBoss2"))
import FixRaidenBoss2 as FRB

live = {}
# The binding exposes one factory per character rather than an all() -- enumerate those.
for builder, game in ((FRB.GIBuilder, "GI"), (FRB.WWMIBuilder, "WuWa")):
    for attr in dir(builder):
        if attr.startswith("_"):
            continue
        factory = getattr(builder, attr)
        if not callable(factory):
            continue
        try:
            modType = factory()
        except Exception:
            continue
        name = getattr(modType, "name", None)
        if name:
            live[name] = (game, tuple(sorted(getattr(modType, "aliases", None) or [])))
print(f"library: {len(live)} mod types ({sum(1 for g, _ in live.values() if g == 'GI')} GI, "
      f"{sum(1 for g, _ in live.values() if g == 'WuWa')} WuWa)")

def namesIn(path, pattern):
    text = open(path, encoding = "utf-8", errors = "replace").read()
    # SCOPE TO THE MOD-TYPE TABLE. commandOpts.rst carries other list-tables (the download modes,
    # the game types) whose rows look the same, and counting those as strays made a clean run
    # report four unknowns -- a checker that always prints noise is one nobody reads.
    if path.endswith(".rst"):
        start = text.find("* - Name")
        if start >= 0:
            stop = text.find(".. list-table::", start)
            text = text[start:stop if stop > 0 else len(text)]
    return set(re.findall(pattern, text, re.M))

places = {
    "api/README.md": (os.path.join(Repo, "Anime Game Remap (for all users)", "api", "README.md"),
                      r"^\|\s*([A-Za-z0-9]+)\s*\|\s*(?:GI|WuWa)\s*\|"),
    "apiMirror/README.md": (os.path.join(Repo, "Anime Game Remap (for all users)", "apiMirror", "README.md"),
                            r"^\|\s*([A-Za-z0-9]+)\s*\|\s*(?:GI|WuWa)\s*\|"),
    "Docs/src/commandOpts.rst": (os.path.join(Repo, "Docs", "src", "commandOpts.rst"),
                                 r"^\s*\*\s+-\s+\*\*([A-Za-z0-9]+)\*\*"),
}

expected = set(live)
bad = 0
for label, (path, pattern) in places.items():
    if not os.path.exists(path):
        print(f"  !! {label}: missing")
        bad += 1
        continue
    got = namesIn(path, pattern)
    # the tables carry non-character rows too; only judge names the library knows plus strays that
    # LOOK like a character (a stray is what caught `BarabaraSummertime`)
    missing = sorted(expected - got)
    extra = sorted(n for n in got - expected if n[0].isupper() and len(n) > 3)
    print(f"  {label:28s} {len(got & expected)}/{len(expected)} present"
          + (f" | MISSING {missing}" if missing else "")
          + (f" | UNKNOWN {extra}" if extra else ""))
    bad += bool(missing) + bool(extra)

# the fourth list: the Python enum
enumPath = None
for d, _, fs in os.walk(os.path.join(Api, "FixRaidenBoss2")):
    for f in fs:
        if f == "ModTypes.py":
            enumPath = os.path.join(d, f)
if enumPath:
    text = open(enumPath, encoding = "utf-8", errors = "replace").read()
    got = set(re.findall(r"^\s{4}([A-Za-z0-9]+)\s*=", text, re.M))
    missing = sorted(n for n in expected if n not in got and n not in text)
    print(f"  {'ModTypes enum':28s} {len(expected) - len(missing)}/{len(expected)} present"
          + (f" | MISSING {missing}" if missing else ""))
    bad += bool(missing)
else:
    print("  !! ModTypes.py not found")

print("ALL FOUR AGREE WITH THE LIBRARY" if not bad else f"{bad} place(s) disagree")
sys.exit(1 if bad else 0)
