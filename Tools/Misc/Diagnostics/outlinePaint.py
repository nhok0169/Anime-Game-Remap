#
# Paint the OUTLINE of one slice of a fixed section's index buffer, and look.
#
# GI draws a character's outline as a second pass over the same draw call: the mesh again, pushed out along each
# vertex's outline normal, front faces culled, under the outline vertex shader (ORFix tags it `vs == 037730.0`).
# A shard, a wedge or a dark patch from that pass looks exactly like a texture or geometry fault in a screenshot,
# and the pose, the camera and the idle animation change between shots -- so comparing "with" and "without" frames
# does not locate anything. This does it the other way round: the section's outline is drawn as usual, EXCEPT one
# slice of the index buffer, drawn last with a flat magenta texture on ps-t0 .. ps-t3. The outline takes its colour
# from those textures, so that slice's outline comes out GREY-TEAL instead of dark, from any angle, in any pose.
# The normal pass is never touched. It found Yaoyao5's hair shards on YaoyaoBamboo in four rounds, after many rounds
# of "with and without" had not (2026-09-28; see Creating Remaps' Yaoyao point 8).
#
#   py -3 outlinePaint.py <mod folder> --ini YaoYao.ini --section <fixed section>            # 8 slices, all paintless
#   py -3 outlinePaint.py <mod folder> ... --state 3                                         # paint slice 3, reload
#   py -3 outlinePaint.py <mod folder> ... --ranges 16461:3294,19755:3291 --state 3          # slice 3 of EACH range
#   py -3 outlinePaint.py <mod folder> --off                                                 # restore
#
# --ranges bisects only part of the buffer (default: all of it); with several ranges, state k paints sub-slice k of
# EVERY range, so ranges whose shards sit in different places (back hair and front locks) narrow in one round.
# --state N rewrites only the start value of $outlinePaint -- re-run with another N and reload. A KEY to cycle it is
# written too (--key, default VK_DOWN), but in practice the cycle desynchronised twice while every press reported
# success: set the state and reload instead. N = parts paints nothing.
#
# Reading the result:
#   a mark turns grey-teal  -> it is the outline of that slice: narrow the range and go again
#   it stays dark           -> not that slice's outline (another slice, another section, or not the outline at all)
#

import argparse
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from purpleSlot import writeFlatDds, sections  # noqa: E402

TexName = "OutlinePaintDiagnostic.dds"
ResName = "ResourceOutlinePaintDiagnostic"
Var = "$outlinePaint"
Backup = ".preOutlinePaint.bak"
OutlineVs = "037730.0"


def indexCount(text: str, body: str, folder: str) -> int:
    """How many indices the section's own `ib` resource holds, from its file size and declared format"""
    hit = re.search(r"^\s*ib\s*=\s*(\S+)", body, re.M)
    if (not hit):
        raise ValueError("the section binds no ib")
    res = hit.group(1)
    for name, start, end in sections(text):
        if (name.lower() != res.lower()):
            continue
        resBody = text[start:end]
        fileHit = re.search(r"^\s*filename\s*=\s*(.+?)\s*$", resBody, re.M)
        if (not fileHit):
            break
        width = 2 if re.search(r"^\s*format\s*=\s*DXGI_FORMAT_R16_UINT", resBody, re.M | re.I) else 4
        path = os.path.join(folder, fileHit.group(1).replace("\\", os.sep))
        return os.path.getsize(path) // width
    raise ValueError(f"no resource [{res}] with a filename")


def main() -> int:
    parser = argparse.ArgumentParser(description = "Paint one slice of a fixed section's outline grey-teal")
    parser.add_argument("mod", help = "the mod folder")
    parser.add_argument("--ini", help = "the .ini holding the section, relative to the mod folder")
    parser.add_argument("--section", help = "the fixed section whose `drawindexed = auto` is split")
    parser.add_argument("--ranges", default = None, help = "first:count[,first:count...] in indices (default: the whole buffer)")
    parser.add_argument("--parts", type = int, default = 8, help = "slices per range (default: %(default)s)")
    parser.add_argument("--state", type = int, default = None, help = "the slice to paint; --parts paints nothing")
    parser.add_argument("--key", default = "VK_DOWN", help = "the key cycling the state (default: %(default)s)")
    parser.add_argument("--off", action = "store_true", help = "restore the .ini files from the backup and stop")
    args = parser.parse_args()

    folder = os.path.abspath(args.mod)
    if (not os.path.isdir(folder)):
        return print(f"no such folder: {folder}") or 2

    if (args.off):
        restored = 0
        for root, _dirs, files in os.walk(folder):
            for name in sorted(files):
                if (name.endswith(Backup)):
                    src = os.path.join(root, name)
                    os.replace(src, src[:-len(Backup)])
                    restored += 1
                    print(f"  restored {os.path.relpath(src[:-len(Backup)], folder)}")
        print(f"\n{restored} file(s) restored" if restored else "\nnothing to restore")
        return 0

    if (not args.ini or not args.section):
        return print("--ini and --section are needed (or --off)") or 2

    iniPath = os.path.join(folder, args.ini)
    backup = iniPath + Backup
    with open(iniPath, "rb") as f:
        current = f.read()
    crlf = b"\r\n" in current

    # A state change on an already painted file only moves the start value
    if (os.path.isfile(backup) and args.state is not None and Var.encode() in current):
        text, n = re.subn(rb"global \$outlinePaint = \d+", b"global $outlinePaint = " + str(args.state).encode(), current)
        if (n != 1):
            return print("the painted file has no single start value; run --off and paint again") or 1
        with open(iniPath, "wb") as f:
            f.write(text)
        print(f"state {args.state}. Reload (F10) and look for grey-teal")
        return 0

    raw = open(backup, "rb").read() if os.path.isfile(backup) else current
    text = raw.decode("utf-8").replace("\r\n", "\n")
    found = [(s, e) for name, s, e in sections(text) if name == args.section]
    if (len(found) != 1):
        return print(f"[{args.section}] is in {args.ini} {len(found)} times, not once") or 1
    start, end = found[0]
    body = text[start:end]
    if (body.count("drawindexed = auto") != 1):
        return print("the section has no single `drawindexed = auto` to split") or 1

    total = indexCount(text, body, os.path.dirname(iniPath))
    ranges = [tuple(int(x) for x in r.split(":")) for r in args.ranges.split(",")] if args.ranges else [(0, total)]
    parts = args.parts

    def sub(first, count, k):
        tris = count // 3
        return first + 3 * (tris * k // parts), first + 3 * (tris * (k + 1) // parts)

    lines = [f"if vs == {OutlineVs}"]
    for k in range(parts):
        painted = sorted(sub(f, c, k) for f, c in ranges)
        rest, cur = [], 0
        for a, b in painted:
            if (a > cur):
                rest.append((cur, a))
            cur = max(cur, b)
        if (cur < total):
            rest.append((cur, total))
        lines.append(f"\t{'if' if k == 0 else 'else if'} {Var} == {k}")
        lines += [f"\t\tdrawindexed = {b - a}, {a}, 0" for a, b in rest]
        lines += [f"\t\tps-t{r} = {ResName}" for r in range(4)]
        lines += [f"\t\tdrawindexed = {b - a}, {a}, 0" for a, b in painted]
        print(f"  slice {k}: " + ", ".join(f"{a}-{b}" for a, b in painted))
    lines += ["\telse", "\t\tdrawindexed = auto", "\tendif", "else", "\tdrawindexed = auto", "endif"]

    text = text[:start] + body.replace("drawindexed = auto", "\n".join(lines)) + text[end:]
    state = parts if args.state is None else args.state
    if ("[Constants]\n" in text):
        text = text.replace("[Constants]\n", f"[Constants]\nglobal {Var} = {state}\n", 1)
    else:
        text += f"\n[Constants]\nglobal {Var} = {state}\n"
    text += (f"\n[KeyOutlinePaintDiagnostic]\nkey = {args.key}\ntype = cycle\n{Var} = {','.join(str(k) for k in range(parts + 1))}\n"
             f"\n[{ResName}]\nfilename = {TexName}\n")

    if (not os.path.isfile(backup)):
        with open(backup, "wb") as f:
            f.write(raw)
    with open(iniPath, "wb") as f:
        f.write((text.replace("\n", "\r\n") if crlf else text).encode("utf-8"))
    writeFlatDds(os.path.join(os.path.dirname(iniPath), TexName))

    print(f"\n{args.ini} :: [{args.section}] -- {total} indices, state {state}"
          f"{' (nothing painted)' if state >= parts else ''}. Reload (F10) and look for grey-teal; --off restores")
    return 0


if __name__ == "__main__":
    sys.exit(main())
