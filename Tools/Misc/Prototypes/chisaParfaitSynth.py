#
# ===== chisaParfaitSynth =====
#
#   py -3 chisaParfaitSynth.py [out folder] [--from <identity mod>]      (default out: ./synth)
#
# `--from` names the pristine identity mod to build from. The maintainer's own copy is usually FIXED
# IN PLACE, and this refuses a fixed source rather than stripping one (see `pristine` below), so the
# normal move is to copy their folder somewhere, undo the copy, and point `--from` at it -- which
# also leaves their live folder alone.
#
# Synthetic ChisaParfait mods for the structural axes the three real ones do NOT cover, built from a
# pristine copy of `ChisaParfaitIdentity`. The audit gate asks what the fix does with every mod a
# person COULD make, and ChisaParfait has three mods on the internet, so most shapes have to be built.
#
# What the real ones already cover, measured rather than assumed (`modAxes.py`): a mod that binds no
# textures at all (ChisaParfait1, the identity), one that binds a little (ChisaParfait2), one that
# binds a lot and ships a second .ini and has a non-Latin FILE name (ChisaParfait3), toggled
# `drawindexed` ranges in one section (2 and 3), and 32-bit index buffers throughout.
#
# What this builds:
#   SynthNoPanel    the identity with its whole component 5 removed -- a mod missing a component.
#                   Component 5 is the one that MERGES onto Chisa's slot 3 with component 3, so its
#                   absence exercises the merge's "one of my sources is not here" path.
#   SynthHidden     `ib = null` inside component 6 -- a HIDDEN object, not a missing one. It must not
#                   be treated as absent and must not trigger a download (the Bennett lesson).
#   SynthRecolour   every texture section moved into a second `tex.ini` beside the mesh `.ini`, with
#                   the hair diffuse tinted so the recolour is attributable in the output.
#   SynthDisabled   a `DISABLED*.ini` beside the live one carrying a stale component hash -- what
#                   GIMI's and WWMI's hash-update tools skip, so real mods carry these.
#   SynthPackaged   GUID file names and `.assets` buffers, the shape a mod manager repacks into. The
#                   2026-09-25 round found a packaged mod rendering NOTHING BUT ITS WEAPON, because
#                   two mesh paths were still built as siblings of the index file.
#   SynthRabbitFX   textures bound through RabbitFX's three resource lines instead of by hash, which
#                   is how ~half of this character's real mods do it and none of the SKIN's do.
#
# NOT built, and why: a 16-bit index buffer, which bit the Neuvillette port. A WWMI index buffer is
# `DXGI_FORMAT_R32_UINT` and this character's mesh is 280k+ indices, so 16 bits cannot address it --
# the shape is not reachable rather than untested.
#
import os
import re
import shutil
import struct
import sys

Wwmi = r"C:\Users\AlexX\Documents\Games\Mods\XXMI-Launcher-Portable-v1.8.5\WWMI"

args = sys.argv[1:]
Source = None
if "--from" in args:
    i = args.index("--from")
    Source = os.path.abspath(args[i + 1])
    del args[i:i + 2]
Out = os.path.abspath(args[0]) if args else os.path.join(os.getcwd(), "synth")

Repo = os.environ.get("AG_REMAP_REPO") or \
    r"C:\Users\AlexX\Documents\Games\Mods\Repos\Anime-Game-Remap"
APISrc = os.path.join(Repo, "Anime Game Remap (for all users)", "api", "src", "py")
sys.path.insert(0, APISrc)
if hasattr(os, "add_dll_directory"):
    os.add_dll_directory(os.path.join(APISrc, "FixRaidenBoss2"))
import FixRaidenBoss2 as FRB


def find(name):
    for base in (Wwmi, os.path.join(Wwmi, "Mods")):
        p = os.path.join(base, name)
        if os.path.isdir(p):
            return p
    raise SystemExit(f"no {name} under {Wwmi} or its Mods/")


def readIni(path):
    return open(path, "rb").read().decode("utf-8").replace("\r\n", "\n")


def writeIni(path, text):
    open(path, "wb").write(text.replace("\n", "\r\n").encode("utf-8"))


def pristine(src, dst):
    """Copy WITHOUT any previous fix. Asserts the source had none, rather than stripping blindly.

    The Neuvillette round lost a day to a 'pristine' copy taken after an earlier fix had already
    deleted one of the mod's own files: every comparison then passed on two equally wrong outputs.
    So this refuses a source that carries a fix instead of trying to undo it.
    """
    shutil.copytree(src, dst)
    for dirpath, _dirs, files in os.walk(dst):
        for f in files:
            if "Remap" in f:
                raise SystemExit(f"{src} is not pristine: it still holds {f}. Undo it first "
                                 f"(FixRaidenBoss7.py -s <folder> -u) and take the copy again.")
            if f.lower().endswith(".ini"):
                t = readIni(os.path.join(dirpath, f))
                if "Remap ---" in t:
                    raise SystemExit(f"{src} is not pristine: {f} carries a fix block.")


def sections(text):
    """[(name or None, body)] -- the preamble comes back with name None."""
    parts = re.split(r"(?m)^(?=\[)", text)
    out = []
    for part in parts:
        m = re.match(r"\[([^\]]+)\]", part)
        out.append((m.group(1) if m else None, part))
    return out


def rebuild(parts):
    return "".join(body for _name, body in parts)


def tint(path, scale):
    """Multiply a .dds's RGB, so a recolour is attributable in the fix's output."""
    tex = FRB.TextureFile(path)
    tex.open()
    px = bytearray(tex.getPixels())
    for i in range(0, len(px) - 3, 4):
        for c in range(3):
            px[i + c] = min(255, int(px[i + c] * scale[c]))
    tex.setPixels(bytes(px), tex.width, tex.height)
    tex.save()


def main():
    shutil.rmtree(Out, ignore_errors=True)
    os.makedirs(Out)
    identity = Source if Source else find("ChisaParfaitIdentity")
    made = []

    # ---- SynthNoPanel: a mod missing a whole component --------------------------------------
    d = os.path.join(Out, "ChisaParfaitSynthNoPanel")
    pristine(identity, d)
    ini = os.path.join(d, "mod.ini")
    parts = [p for p in sections(readIni(ini)) if p[0] != "TextureOverrideComponent5"]
    writeIni(ini, rebuild(parts))
    made.append(("SynthNoPanel", "component 5's section removed outright"))

    # ---- SynthHidden: ib = null, a HIDDEN object rather than a missing one --------------------
    d = os.path.join(Out, "ChisaParfaitSynthHidden")
    pristine(identity, d)
    ini = os.path.join(d, "mod.ini")
    parts = []
    for name, body in sections(readIni(ini)):
        if name == "TextureOverrideComponent6":
            body = body.replace("if $mod_enabled\n", "if $mod_enabled\n    ib = null\n", 1)
        parts.append((name, body))
    writeIni(ini, rebuild(parts))
    made.append(("SynthHidden", "component 6 hidden with ib = null"))

    # ---- SynthRecolour: the textures in their OWN .ini beside the mesh one --------------------
    d = os.path.join(Out, "ChisaParfaitSynthRecolour")
    pristine(identity, d)
    ini = os.path.join(d, "mod.ini")
    mesh, tex = [], []
    for name, body in sections(readIni(ini)):
        isTexture = name is not None and re.match(r"(Resource|TextureOverride)Texture\d+$", name)
        (tex if isTexture else mesh).append((name, body))
    writeIni(ini, rebuild(mesh))
    writeIni(os.path.join(d, "tex.ini"),
             "; a texture-only recolour, in its own .ini beside the mesh file\n\n" + rebuild(tex))
    tint(os.path.join(d, "Textures", "Components-1 t=a94ee44f.dds"), (1.0, 0.45, 0.75))
    made.append(("SynthRecolour", "textures split into tex.ini, hair diffuse tinted magenta"))

    # ---- SynthDisabled: a DISABLED variant carrying a stale hash ------------------------------
    d = os.path.join(Out, "ChisaParfaitSynthDisabled")
    pristine(identity, d)
    stale = readIni(os.path.join(d, "mod.ini")).replace("hash = e611d493", "hash = deadbeef")
    writeIni(os.path.join(d, "DISABLEDold.ini"), stale)
    made.append(("SynthDisabled", "DISABLEDold.ini beside the live one, component hash stale"))

    # ---- SynthPackaged: GUID names and .assets buffers ----------------------------------------
    d = os.path.join(Out, "ChisaParfaitSynthPackaged")
    pristine(identity, d)
    ini = os.path.join(d, "mod.ini")
    text = readIni(ini)
    meshes = os.path.join(d, "Meshes")
    guid = "3f2a91c4-77d8-4e61-9a05-{:012d}"
    for i, f in enumerate(sorted(os.listdir(meshes))):
        if not f.lower().endswith(".buf"):
            continue
        new = guid.format(i) + ".assets"
        os.rename(os.path.join(meshes, f), os.path.join(meshes, new))
        text = text.replace(f"Meshes/{f}", f"Meshes/{new}").replace(f"Meshes\\{f}", f"Meshes\\{new}")
    writeIni(ini, text)
    os.rename(ini, os.path.join(d, guid.format(999) + ".ini"))
    made.append(("SynthPackaged", "GUID .assets buffers and a GUID .ini"))

    # ---- SynthRabbitFX: textures through RabbitFX rather than by hash -------------------------
    # Copied from a real Chisa mod's shape: the three `Resource\RabbitFX\*` lines and the call,
    # inside the component's own block, with the by-hash TextureOverrideTexture* sections dropped so
    # nothing else can bind them.
    d = os.path.join(Out, "ChisaParfaitSynthRabbitFX")
    pristine(identity, d)
    ini = os.path.join(d, "mod.ini")
    byHash = {}
    for name, body in sections(readIni(ini)):
        m = name and re.match(r"TextureOverrideTexture(\d+)$", name)
        if m:
            h = re.search(r"(?m)^hash\s*=\s*(\S+)", body)
            if h:
                byHash[h.group(1)] = f"ResourceTexture{m.group(1)}"

    # role -> hash, from the fix's own roles table for this character
    perComponent = {
        0: {"Diffuse": "f2646d21", "Lightmap": "d3b9ba76", "Normalmap": "9ccd7ea7"},
        1: {"Diffuse": "a94ee44f", "Lightmap": "3f433212", "Normalmap": "d547f3c6"},
        2: {"Diffuse": "53e96488", "Lightmap": "226d9bc4"},
        3: {"Diffuse": "4c420ea9", "Lightmap": "6b7ae743", "Normalmap": "3c4279a9"},
        4: {"Diffuse": "1d79fc96", "Lightmap": "4668fce8", "Normalmap": "b9a888ec"},
        5: {"Diffuse": "1e1b7bbc", "Lightmap": "2f911db8", "Normalmap": "56e725c4"},
        6: {"Diffuse": "226b31fc"},
        7: {"Diffuse": "71a6e63f", "Lightmap": "2c990f51", "Normalmap": "e4463fca"},
    }

    parts = []
    for name, body in sections(readIni(ini)):
        if name and re.match(r"(Resource|TextureOverride)Texture\d+$", name):
            if name.startswith("TextureOverride"):
                continue                       # drop the by-hash binding; keep the Resource section
        m = name and re.match(r"TextureOverrideComponent(\d+)$", name)
        if m:
            lines = []
            for role, h in perComponent.get(int(m.group(1)), {}).items():
                res = byHash.get(h)
                if res:
                    lines.append(f"        Resource\\RabbitFX\\{role} = ref {res}\n")
            if lines:
                lines.append("        run = Commandlist\\RabbitFX\\SetTextures\n")
                body = body.replace("        run = CommandListTriggerResourceOverrides\n",
                                    "".join(lines) + "        run = CommandListTriggerResourceOverrides\n", 1)
        parts.append((name, body))
    writeIni(ini, rebuild(parts))
    made.append(("SynthRabbitFX", "textures bound through RabbitFX's resource lines, by-hash dropped"))

    print(f"built into {Out}:")
    for name, what in made:
        print(f"  {name:<14} {what}")
    print("\nfix them with the launcher and read the output; none of these is a mod anyone shipped,")
    print("so the question is only whether the fix does something SENSIBLE rather than something")
    print("that crashes, silently skips, or binds another character's art.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
