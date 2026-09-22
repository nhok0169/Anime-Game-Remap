#
# ===== wwmiPassLayout =====
#
# What a pixel shader actually READS in each ps-t register, for every draw of a WWMI frame dump --
# the two questions `wwmiDrawTable.py` cannot answer, because it carries bindings forward the way
# the device does:
#
#   1. WHICH REGISTERS THAT DRAW SET, and which it merely inherited. A slot is often drawn by
#      several shaders and only ONE of them sets the whole set; the others set ps-t0 and leave the
#      rest standing from an earlier draw. A carried-forward table shows all of them fully bound,
#      so it reads as several different layouts of the same slot, and mirroring one of those onto
#      the target mirrors another component's leftover state.
#
#   1b. AND WHETHER THE DRAW IT INHERITED FROM IS EVEN THE SAME CHARACTER. `~` says a register was
#      standing from an earlier draw; it does not say whose. A frame holds every character on
#      screen, so the draw that last wrote ps-t5 may belong to an NPC three metres away, and its
#      texture then reads as "what this character binds there". That cost an in-game round on
#      Chisa: her upper body INHERITS 4848ae14 at ps-t5 from a draw that turns out to appear
#      byte-identically in the other character's dump too, it was transcribed into the fix as hers,
#      and the kimono came back yellow. `--against <the other dump>` marks such a register `!`
#      instead of `~`: any draw whose (start, count, pixel shader) appears in BOTH dumps is
#      something in the scene that is not either character. ONLY a `sets` line is evidence.
#
#   2. WHAT KIND of texture each bound one is, classified from its PIXELS rather than from where it
#      sits: a normal map is (R, G, B=0) around 127, a material mask is coded (R high, G low,
#      B ~126, A 0), a detail / id map is near black, a matcap or ramp is small, and what is left is
#      a diffuse. Two passes of one game do NOT have to agree about the order -- measured on
#      ChisaParfait, her two body clothing shaders read the normal at ps-t0 and the detail map at
#      ps-t2, and her slot 5 shader reads them the other way round.
#
#   python wwmiPassLayout.py <FrameAnalysis folder> [--starts 0 13566 ...] [--downloads <folder>]
#
# `--starts` limits the report to draws whose StartIndexLocation is one of those (a component's
# index_offset from Metadata.json); with no `--starts` every draw of the frame is listed. The
# textures are read out of the dump folder itself, so nothing needs downloading; `--downloads`
# names a `Data/Mod Downloads/WuWa/<Char>/<ver>` folder to fall back to for a texture the dump
# only wrote as a .jpg preview.
#
# See Creating Remaps' "A PASS IS A REGISTER LAYOUT, AND ONLY THE DRAW THAT SETS IT SAYS WHAT IT IS".
#

import argparse
import glob
import os
import re
import sys

import numpy as np

DrawPattern = re.compile(r"^(?P<draw>\d{6}) (?P<call>[A-Za-z]+)\((?P<args>.*)\)(?P<tail>.*)$")
SlotPattern = re.compile(r"^\s+(?P<slot>\d+): (?:view=\S+ )?resource=\S+ hash=(?P<hash>[0-9a-f]+)(?: orig_hash=(?P<orig>[0-9a-f]+))?")
DrawIndexedPattern = re.compile(r"IndexCount:(\d+), StartIndexLocation:(\d+), BaseVertexLocation:(-?\d+)")


def loadApi():
    """The API, for reading a .dds the dump wrote -- Pillow cannot decode BC7"""
    here = os.path.dirname(os.path.abspath(__file__))
    api = os.path.abspath(os.path.join(here, "..", "..", "..", "Anime Game Remap (for all users)", "api", "src", "py"))
    sys.path.insert(0, api)
    if (hasattr(os, "add_dll_directory")):
        os.add_dll_directory(os.path.join(api, "FixRaidenBoss2"))
    import FixRaidenBoss2 as FRB
    return FRB


def draws(folder, starts):
    """Every DrawIndexed of the frame, with the ps-t slots THAT draw set and the ones it inherited"""
    state, fresh, pending, ps = {}, {}, None, None
    owner = {}                       # slot -> the draw that last SET it, as (start, count, ps)
    out = []
    with open(os.path.join(folder, "log.txt"), "r", encoding = "utf-8", errors = "replace") as f:
        for line in f:
            line = line.rstrip("\r\n")
            match = DrawPattern.match(line)
            if (match is not None):
                call, args, tail = match.group("call"), match.group("args"), match.group("tail")
                pending = None
                if (call == "PSSetShaderResources"):
                    pending = "ps-t"
                elif (call == "PSSetShader"):
                    found = re.search(r"hash=([0-9a-f]+)", tail)
                    ps = found.group(1) if (found is not None) else None
                elif (call == "DrawIndexed"):
                    counts = DrawIndexedPattern.search(args)
                    if (counts is not None):
                        key = (int(counts.group(2)), int(counts.group(1)), ps)
                        for slot in fresh:                 # this draw now owns what it just set
                            owner[slot] = key
                        if (not starts or key[0] in starts):
                            out.append({"draw": int(match.group("draw")), "ps": ps, "start": key[0],
                                        "count": key[1], "bound": dict(state), "fresh": dict(fresh),
                                        "key": key, "owner": dict(owner)})
                    fresh = {}
                continue
            if (pending == "ps-t"):
                slot = SlotPattern.match(line)
                if (slot is not None):
                    state[int(slot.group("slot"))] = slot.group("hash")
                    fresh[int(slot.group("slot"))] = slot.group("hash")
    return out


def kindOf(path, FRB):
    """What a bound texture IS, from its pixels"""
    try:
        if (path.lower().endswith(".dds")):
            tex = FRB.TextureFile(path)
            tex.open()
            px = np.frombuffer(tex.getPixels(), dtype = np.uint8).reshape(tex.height, tex.width, 4)
            width, height = tex.width, tex.height
        else:
            from PIL import Image
            img = Image.open(path).convert("RGBA")
            px = np.asarray(img)
            width, height = img.size
    except Exception as error:
        return f"unreadable ({error})"

    r, g, b, a = (float(px[..., i].mean()) for i in range(4))
    note = f"{width}x{height} mean {r:5.1f} {g:5.1f} {b:5.1f} A{a:5.1f}"
    if (width <= 512 and height <= 512):
        return f"{note}  matcap / ramp (small)"
    if (b < 8 and abs(r - 127) < 30 and abs(g - 127) < 30):
        return f"{note}  NORMAL (RG, B = 0)"
    if (r < 14 and g < 14 and b < 14):
        return f"{note}  detail / id (near black)"
    if (g < 64 and r > 120 and 100 < b < 150 and a < 32):
        return f"{note}  MASK (coded; G is how shiny)"
    return f"{note}  diffuse"


def fileOf(folder, downloads, draw, slot, texHash):
    """The dump's own file for a bound slot, or the download folder's copy of that hash"""
    for pattern in (os.path.join(folder, f"{draw:06d}-ps-t{slot}={texHash}*"),
                    os.path.join(folder, f"{draw:06d}-ps-t{slot}={texHash}(*")):
        found = sorted(glob.glob(pattern))
        dds = [p for p in found if (p.lower().endswith(".dds"))]
        if (dds or found):
            return (dds or found)[0]
    if (downloads):
        found = sorted(glob.glob(os.path.join(downloads, f"*Texture{texHash}.dds")))
        if (found):
            return found[0]
    return None


def main():
    parser = argparse.ArgumentParser(description = "what each pass of a WWMI frame actually reads, per register")
    parser.add_argument("folder", help = "a FrameAnalysis-<Character>-<date> folder")
    parser.add_argument("--starts", nargs = "*", type = int, default = [],
                        help = "only draws at these StartIndexLocations (a component's index_offset)")
    parser.add_argument("--downloads", default = None,
                        help = "a Data/Mod Downloads/WuWa/<Char>/<ver> folder, for a texture the dump only previewed")
    parser.add_argument("--against", default = None,
                        help = "the OTHER character's FrameAnalysis folder; a register inherited from a draw "
                               "present in both is marked ! -- it belongs to neither character")
    args = parser.parse_args()

    FRB = loadApi()
    mine = draws(args.folder, set())
    shared = set()
    if (args.against is not None):
        shared = {e["key"] for e in mine} & {e["key"] for e in draws(args.against, set())}
        print(f"{len(shared)} draws appear in both dumps identically -- they are not either "
              f"character's, and a register marked ! was left standing by one of them" + os.linesep)
    for entry in draws(args.folder, set(args.starts)):
        setHere = entry["fresh"]
        print(f"draw {entry['draw']:6d}  start {entry['start']:7d}  count {entry['count']:7d}  ps={entry['ps']}"
              f"   {'sets the whole set' if (len(setHere) >= 4) else ('sets ' + ', '.join(f'ps-t{s}' for s in sorted(setHere)) if (setHere) else 'SETS NOTHING -- every register is inherited')}")
        for slot in sorted(entry["bound"]):
            texHash = entry["bound"][slot]
            # ~ = inherited from an earlier draw; ! = inherited from a draw that is not this character
            mark = " " if (slot in setHere) else ("!" if (entry["owner"].get(slot) in shared) else "~")
            path = fileOf(args.folder, args.downloads, entry["draw"], slot, texHash)
            print(f"   {mark}ps-t{slot:<2d} {texHash:10s} {kindOf(path, FRB) if (path) else 'no dump of it'}")


if (__name__ == "__main__"):
    main()
