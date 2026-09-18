"""Which textures does each slot of a mod render with -- the MOD's, or the GAME's?

    python slotTextures.py <mod folder> [<mod folder> ...]

A GIMI ``TextureOverride`` binds its ``ps-t`` registers for the draw call its ``hash`` matches and
for no other, so in an UNFIXED mod:

  * a slot whose own section declares ``ps-t`` registers renders with THE MOD's textures;
  * a slot that declares none renders with THE GAME's, however thoroughly the mod repainted its
    own copy of that texture.

Mods disagree about this constantly -- of five mods of one skin, the identity mod and a recolour
bind every register, an NSFW edit has no sections at all for one component, a hair mod binds for
one slot and not its sibling, and a port binds the 5.x register where its sibling binds the 6.x
one. A fix has to reproduce whatever the mod did, per slot, and getting it wrong is invisible in
every count and every byte comparison: it shows up only as somebody else's texture on the
character.

This prints, per mod, the table to read BEFORE reasoning about which textures a slot ought to use
(Creating Remaps' "A ``TextureOverride`` binds registers only for the draw its hash matches"), and
then -- if the folder has been fixed -- what each draw of each remapped section actually binds.
This library marks a downloaded (game) texture with the ``RemapDL`` keyword in its resource name,
so the two sides compare without opening a single ``.dds``.

A draw slot is a ``TextureOverride`` carrying a ``match_first_index``; the structural sections
beside it (``...IB``, ``...Blend``, ``...Position``, ``...VertexLimitRaise``) bind no textures by
construction and are not listed.

**This reports; it does not decide.** Which source slots merge into which target section is the
fixer's configuration, not something the ``.ini`` text says, so a cross-check between the two
tables would need that mapping and would be wrong without it. Read the two halves against each
other. The only failure it raises on its own is a folder with no ``.ini`` file, because a missing
input is not a pass.

Needs nothing but the ``.ini`` file -- no API, no build."""
import os
import re
import sys

Section = re.compile(r"^\[(.+)\]$")
RegLine = re.compile(r"^(ps-t\d+)\s*=\s*(.+)$", re.IGNORECASE)
Marker = "Remap ---"

problems = []


def check(ok, what):
    print(("  ok   " if ok else "  FAIL ") + what)
    if (not ok):
        problems.append(what)


def iniFiles(folder):
    for root, _, files in os.walk(folder):
        for f in sorted(files):
            if (f.lower().endswith(".ini")):
                yield os.path.join(root, f)


def readSections(text):
    """[(name, [(key, value), ...]), ...] -- order kept; the last binding of a register wins."""
    sections, name, kvps = [], None, []
    for line in text.splitlines():
        s = line.strip()
        if (not s or s.startswith(";")):
            continue

        match = Section.match(s)
        if (match is not None):
            if (name is not None):
                sections.append((name, kvps))
            name, kvps = match.group(1), []
        elif (name is not None and "=" in s):
            key, _, value = s.partition("=")
            kvps.append((key.strip(), value.strip()))
    if (name is not None):
        sections.append((name, kvps))
    return sections


def sourceWants(sections):
    """{draw slot: "mod" | "game"} for every TextureOverride the MODDER wrote.

    A draw slot is one carrying a `match_first_index`: that is what names a range of the object's
    index buffer, and it is the discriminator that separates the slots which actually render from
    the structural sections (`...IB`, `...Blend`, `...Position`, `...VertexLimitRaise`) that never
    bind a texture and would otherwise flood the table."""
    wants = {}
    for name, kvps in sections:
        if (not name.lower().startswith("textureoverride")):
            continue
        # this library's own output is not the modder's intent
        if ("remap" in name.lower()):
            continue
        if (not any(k.lower() == "match_first_index" for k, _ in kvps)):
            continue
        wants[name] = "mod" if any(RegLine.match(f"{k} = {v}") for k, v in kvps) else "game"
    return wants


def fixedDraws(sections):
    """{remapped section name: [(draw value, resource bound to the lowest ps-t), ...]}."""
    draws = {}
    for name, kvps in sections:
        if (not name.lower().startswith("textureoverride")):
            continue

        here, bound = [], None
        for key, value in kvps:
            match = RegLine.match(f"{key} = {value}")
            if (match is not None and (bound is None or match.group(1).lower() <= bound[0])):
                bound = (match.group(1).lower(), value)
            elif (key.lower() == "drawindexed"):
                here.append((value, bound[1] if bound is not None else None))
        if (here):
            draws[name] = here
    return draws


def analyse(folder):
    label = os.path.basename(folder.rstrip("\\/")) or folder
    print(f"\n=== {label}")

    paths = list(iniFiles(folder))
    if (not paths):
        # A folder with no .ini is NOT a pass. Returning quietly here is how a run over five mods
        # came to report success having touched three.
        check(False, f"{label}: no .ini file under {folder}")
        return

    for path in paths:
        text = open(path, encoding = "utf-8", errors = "replace").read().replace("\r\n", "\n")
        original, marked, remapped = text.partition(Marker)

        wants = sourceWants(readSections(original))
        if (wants):
            print(f"  {os.path.relpath(path, folder)}")
            width = max(len(n) for n in wants)
            for name in sorted(wants):
                art = "the MOD's textures" if wants[name] == "mod" else "the GAME's (it binds no ps-t)"
                print(f"    {name:<{width}}  renders with {art}")

        if (not marked):
            continue

        for name, draws in sorted(fixedDraws(readSections(remapped)).items()):
            print(f"  [{name}]")
            for value, resource in draws:
                kind = "game" if (resource and "RemapDL" in resource) else "mod"
                print(f"      drawindexed = {value:<24} <- the {kind}'s ({resource})")


for target in sys.argv[1:]:
    analyse(target)

print()
print("PROBLEMS:" if problems else "READ THE TWO HALVES AGAINST EACH OTHER -- every mod produced a table")
for problem in problems:
    print("  -", problem)
sys.exit(1 if problems else 0)
