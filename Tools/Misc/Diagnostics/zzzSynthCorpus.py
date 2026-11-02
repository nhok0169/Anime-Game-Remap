"""A ZZZ sweep corpus built from a character's IDENTITY mod: the structural shapes a ZZMI fix reads the mod for.

    py -3 zzzSynthCorpus.py <identity mod folder> <out folder> [--name Miyabi] [--accessories ef64351c,4b465238]
                            [--list <corpus.txt>]

The identity mod is the game's own model as a ZZMI mod (Tools/Misc/Prototypes/zzmiIdentityMod.py), which no
fix of a real mod moves past its simplest paths. Each folder below is that mod with one shape added, so a sweep
(`regressBuilds.py`) reaches what the ZZMI template reads off a mod's sections:

  <Name>Identity           the identity mod as it is
  <Name>RecolourSibling    a sibling `Recolour.ini` replacing the Body diffuse by its texture hash alone
                           (`hash = <tex_a_diffuse>` / `this =`)
  <Name>RecolourBelow      the same `Recolour.ini` one folder down, which a fix reads as the game does
  <Name>RecolourDisabled   the same in a `DISABLED` folder, which the game does not load
  <Name>AccessorySkip      the extra objects the fix adds (--accessories: the ib hashes of the objects the
                           character's ZZMIFixerConfig::Component::extraObjects names) hidden behind `$acc == 0`
  <Name>AccessorySkipElse  the same, hidden in the `else` branch of an `if` that sets another variable
  <Name>Cross              the Body pre-skinned into a buffer of its own (`= copy so0`) and drawn again from the
                           Legs section with the Body's ib -- the shape ZZMIFixerConfig::crossSlotDraws moves
  <Name>CrossRanges        the same, the Body's own draw split into two halves under two toggles

None is a real mod: they cover the reading, not what a real author writes. `--list` writes the folders, one
per line, for `regressBuilds.py`.
"""
import argparse
import os
import re
import shutil
import sys

Repo = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", ".."))
APISrc = os.path.join(Repo, "Anime Game Remap (for all users)", "api", "src", "py")
sys.path.insert(0, APISrc)
if hasattr(os, "add_dll_directory"):
    os.add_dll_directory(os.path.join(APISrc, "FixRaidenBoss2"))
import FixRaidenBoss2 as FRB  # noqa: E402


def newestHashes(name):
    """The newest version's rows of every component of 'name': {component name: {hash type: hash}}"""
    table = getattr(FRB.ModTypes, name).value.hashes.repo.toNestedDict()
    out = {}
    for ver in sorted(table, key = lambda v: tuple(int(p) for p in re.findall(r"\d+", v))):
        for comp, row in table[ver].items():
            if comp.startswith(name):
                out.setdefault(comp, {}).update(row)
    return out


def sectionAt(lines, header):
    """(start, end) line indices of the section 'header' (its `[...]` line to the line before the next one)"""
    start = lines.index(header)
    end = start + 1
    while end < len(lines) and not lines[end].startswith("["):
        end += 1
    return start, end


def insertInto(lines, header, extra, atEnd = True):
    start, end = sectionAt(lines, header)
    while atEnd and end > start + 1 and lines[end - 1].strip() == "":
        end -= 1
    at = end if atEnd else start + 1
    return lines[:at] + extra + lines[at:]


def recolour(name, hashes):
    h = hashes[name + "Body"]["tex_a_diffuse"]
    return ["[TextureOverride%sBodyDiffuseRecolour]" % name, "hash = " + h, "this = Resource%sBodyRecolour" % name, "",
            "[Resource%sBodyRecolour]" % name, "filename = %sBodyADiffuse.dds" % name, ""]


def accessorySkip(lines, accessories, inElse):
    head = ["[Constants]", "global persist $acc = 0", "global $accOther = 0", "",
            "[KeyAcc]", "key = VK_UP", "type = cycle", "$acc = 0,1", ""]
    body = []
    for i, h in enumerate(accessories):
        body += ["[TextureOverrideAccessory%d]" % i, "hash = " + h]
        if inElse:
            body += ["if $acc == 1", "\t$accOther = 1", "else", "\thandling = skip", "endif", ""]
        else:
            body += ["if $acc == 0", "\thandling = skip", "endif", ""]
    return head + lines + [""] + body


def cross(lines, name, ranges):
    blend = "[TextureOverride%sBodyBlend]" % name
    start, end = sectionAt(lines, blend)
    body = lines[start:end]
    at = body.index("endif")
    body = body[:at] + ["\tResource%sBodySkinned = copy so0" % name] + body[at:]
    lines = lines[:start] + body + lines[end:]

    bodyA = "[TextureOverride%sBodyA]" % name
    start, end = sectionAt(lines, bodyA)
    draw = next(l for l in lines[start:end] if l.startswith("drawindexed"))
    count = int(draw.split("=")[1].split(",")[0])
    if ranges:
        half = (count // 6) * 3
        section = lines[start:end]
        at = section.index(draw)
        section[at:at + 1] = ["if $halfA == 1", "\tdrawindexed = %d, 0, 0" % half, "endif",
                              "if $halfB == 1", "\tdrawindexed = %d, %d, 0" % (count - half, half), "endif"]
        lines = lines[:start] + section + lines[end:]
        lines = ["[Constants]", "global persist $halfA = 1", "global persist $halfB = 1", "",
                 "[KeyHalfA]", "key = VK_LEFT", "type = cycle", "$halfA = 0,1", "",
                 "[KeyHalfB]", "key = VK_RIGHT", "type = cycle", "$halfB = 0,1", ""] + lines

    lines = insertInto(lines, "[TextureOverride%sLegsA]" % name,
                       ["vb0 = Resource%sBodySkinned" % name, "ib = Resource%sBodyAIB" % name,
                        "drawindexed = %d, 0, 0" % count])
    return lines + ["", "[Resource%sBodySkinned]" % name, ""]


def write(src, out, iniName, lines, extras = None):
    if os.path.isdir(out):
        shutil.rmtree(out)
    shutil.copytree(src, out)
    with open(os.path.join(out, iniName), "w", encoding = "utf-8", newline = "\r\n") as f:
        f.write("\n".join(lines))
    for rel, more in (extras or {}).items():
        path = os.path.join(out, rel)
        os.makedirs(os.path.dirname(path), exist_ok = True)
        with open(path, "w", encoding = "utf-8", newline = "\r\n") as f:
            f.write("\n".join(more))
        # a recolour in a subfolder names its texture from there
        if os.path.dirname(rel):
            texture = next(f for f in os.listdir(src) if f.endswith("BodyADiffuse.dds"))
            shutil.copy2(os.path.join(src, texture), os.path.join(os.path.dirname(path), texture))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("identity")
    ap.add_argument("out")
    ap.add_argument("--name", default = "Miyabi")
    ap.add_argument("--accessories", default = "ef64351c,4b465238")
    ap.add_argument("--list", default = "")
    a = ap.parse_args()

    iniName = next(f for f in os.listdir(a.identity) if f.lower().endswith(".ini"))
    with open(os.path.join(a.identity, iniName), encoding = "utf-8") as f:
        lines = f.read().splitlines()
    hashes = newestHashes(a.name)
    accessories = [h for h in a.accessories.split(",") if h]
    name = a.name

    shapes = [
        ("Identity", lines, None),
        ("RecolourSibling", lines, {"Recolour.ini": recolour(name, hashes)}),
        ("RecolourBelow", lines, {os.path.join("Sub", "Recolour.ini"): recolour(name, hashes)}),
        ("RecolourDisabled", lines, {os.path.join("DISABLED Sub", "Recolour.ini"): recolour(name, hashes)}),
        ("AccessorySkip", accessorySkip(lines, accessories, False), None),
        ("AccessorySkipElse", accessorySkip(lines, accessories, True), None),
        ("Cross", cross(lines, name, False), None),
        ("CrossRanges", cross(lines, name, True), None),
    ]
    os.makedirs(a.out, exist_ok = True)
    made = []
    for suffix, shapeLines, extras in shapes:
        out = os.path.join(a.out, name + suffix)
        write(a.identity, out, iniName, shapeLines, extras)
        made.append(out)
        print(out)
    if a.list:
        with open(a.list, "w", encoding = "utf-8") as f:
            f.write("\n".join(made) + "\n")
    print("built", len(made), "folders")


if __name__ == "__main__":
    main()
