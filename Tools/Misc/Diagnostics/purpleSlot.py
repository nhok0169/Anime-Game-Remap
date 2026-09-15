#
# Bind an unmistakable texture to one remapped slot, and look.
#
# An .ini says what the fix INTENDED; a substituted texture says what the GPU actually USED, and the
# two diverge exactly where the bug is. This is the diagnostic that settled Bennett's hair after
# three measured hypotheses of mine had missed (see AI Agent Help/Overview, "WHEN YOU CANNOT TELL
# WHAT A DRAW IS USING, REPLACE THE TEXTURE WITH SOMETHING UNMISTAKABLE").
#
# It rebinds ONE register of every remapped section of one component to a flat magenta texture, and
# nothing else. Reversible: --off puts the original lines back from the backup it writes.
#
#   py -3 purpleSlot.py <mod folder> --component Eye
#   py -3 purpleSlot.py <mod folder> --component Eye --off
#
# Reading the result:
#   the slot turns MAGENTA  -> this section is what draws it, and the bug is in the texture or the
#                              UVs it samples
#   the slot is UNCHANGED   -> this section is NOT what draws it. Something else is: the skin's own
#                              geometry, another .ini, or a draw that was never skipped
#

import argparse
import os
import re
import struct
import sys

Magenta = (255, 0, 255, 255)
TexName = "PurpleSlotDiagnostic.dds"
ResName = "ResourcePurpleSlotDiagnostic"
Backup = ".prePurple.bak"


def writeFlatDds(path: str, size: int = 256) -> None:
    """An uncompressed 32-bit DDS of one colour -- no Compressonator, no Pillow DDS writer needed"""
    DDSD = 0x1 | 0x2 | 0x4 | 0x1000 | 0x8               # CAPS HEIGHT WIDTH PIXELFORMAT PITCH
    # 4 magic + 7 dwords + 44 reserved + 13 dwords (8 pixelformat, 4 caps, 1 reserved2) = 128
    header = struct.pack("<4sIIIIIII44xIIIIIIIIIIIII",
                         b"DDS ", 124, DDSD, size, size, size * 4, 0, 0,
                         32,                             # DDS_PIXELFORMAT.dwSize
                         0x41,                           # DDPF_RGB | DDPF_ALPHAPIXELS
                         0,                              # fourCC
                         32,                             # bit count
                         0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000,
                         0x1000, 0, 0, 0, 0)             # caps1..4, reserved2
    if (len(header) != 128):
        raise ValueError(f"DDS header is {len(header)} bytes, not 128")
    r, g, b, a = Magenta
    with open(path, "wb") as f:
        f.write(header)
        f.write(bytes([b, g, r, a]) * (size * size))


def sections(text: str):
    """(name, start, end) for every section, over LF-normalised text"""
    marks = [(m.group(1), m.start()) for m in re.finditer(r"^\[([^\]]+)\]", text, re.M)]
    for i, (name, start) in enumerate(marks):
        yield name, start, (marks[i + 1][1] if (i + 1 < len(marks)) else len(text))


def activeInis(folder: str):
    import glob

    return [p for p in sorted(glob.glob(os.path.join(folder, "**", "*.ini"), recursive = True))
            if (not os.path.basename(p).upper().startswith("DISABLED"))]


def main() -> int:
    parser = argparse.ArgumentParser(description = "Rebind one remapped slot to a flat magenta texture")
    parser.add_argument("mod", help = "the mod folder")
    parser.add_argument("--component", default = "Eye", help = "target component to paint (default: %(default)s)")
    parser.add_argument("--register", default = "ps-t0", help = "which register to replace (default: %(default)s)")
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

    touched = 0
    for iniPath in activeInis(folder):
        with open(iniPath, "rb") as f:
            raw = f.read()
        crlf = b"\r\n" in raw
        text = raw.decode("utf-8", "replace").replace("\r\n", "\n")

        edits = []
        for name, start, end in sections(text):
            if (f"BennettAdventure{args.component}" not in name or "Remap" not in name):
                continue
            body = text[start:end]
            hit = re.search(rf"^(\s*){re.escape(args.register)}\s*=.*$", body, re.M)
            if (not hit):
                continue
            edits.append((start + hit.start(), start + hit.end(),
                          f"{hit.group(1)}{args.register} = {ResName}", name, hit.group(0).strip()))

        if (not edits):
            continue

        for begin, finish, replacement, name, was in reversed(edits):
            text = text[:begin] + replacement + text[finish:]
            print(f"  {os.path.relpath(iniPath, folder)} :: {name}")
            print(f"      {was}  ->  {args.register} = {ResName}")

        if (f"[{ResName}]" not in text):
            # the mod's own texture resources declare a filename and nothing else; match that
            text += f"\n\n[{ResName}]\nfilename = {TexName}\n"

        backup = iniPath + Backup
        if (not os.path.isfile(backup)):
            with open(backup, "wb") as f:
                f.write(raw)

        with open(iniPath, "wb") as f:
            f.write((text.replace("\n", "\r\n") if crlf else text).encode("utf-8"))

        writeFlatDds(os.path.join(os.path.dirname(iniPath), TexName))
        touched += 1

    if (not touched):
        print(f"no remapped {args.component} section binds {args.register} -- nothing to paint.\n"
              f"That is itself an answer: the fix is not binding that register at all.")
        return 1

    print(f"\n{touched} .ini file(s) painted. Reload the mod and look at the {args.component}:")
    print(f"  MAGENTA   -> this section draws it; the bug is the texture or the UVs")
    print(f"  UNCHANGED -> something ELSE draws it, and the fix is editing the wrong thing")
    print(f"\nundo with:  py -3 {os.path.basename(__file__)} {args.mod} --component {args.component} --off")
    return 0


if (__name__ == "__main__"):
    sys.exit(main())
