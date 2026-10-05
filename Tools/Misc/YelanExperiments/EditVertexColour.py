"""
Rewrites the vertex colour (the COLOR element: the first 4 bytes of every Texcoord.buf line) of the
vertices some mod objects use, and saves the result as a NEW Texcoord.buf.

Why: the vertex colour is a per-vertex shading parameter set the shader reads (outline width, shadow
bias, ... -- the meaning is the shader's). Measured on this mod against both game models:

    | vertices                  | R   | G   | B   | A                    |
    | mod Head object (face)    | 255 | 128 | 128 | 128 (some 0/26/51)   |
    | mod Body object           | 255 | 188 | 188 | 77                   |
    | Yelan, the game's model   | 255 | 128 | 128 | 128 (some 26..77)    |
    | YelanTranquil Body A/B/C  | 255 | 128 | 128 | 77 / 102 / 128 / 0   |

The mod's Body carries G = B = 188 where both game models say 128 -- a value Yelan's shader may
ignore and YelanTranquil's may not. This script makes that testable.

    py -3 EditVertexColour.py                                   Body: G,B -> 128   -> yelanTexcoordVC.buf
    py -3 EditVertexColour.py --objects Body --set G=128 B=128 A=77
    py -3 EditVertexColour.py --objects Head Body --set A=128 --dest yelanTexcoordA128.buf
    py -3 EditVertexColour.py --objects Head Body --set R=255 --zeroUV1 --dest yelanTexcoordUV1.buf    (colour untouched, second UV zeroed)

Then MixedFilter.py --texcoordFile <file> draws with it (the .ini's Texcoord resource points at it).
"""

import argparse
import os

import numpy as np

Here = os.path.dirname(os.path.abspath(__file__))
Stride = 20
Channels = {"R": 0, "G": 1, "B": 2, "A": 3}


if (__name__ == "__main__"):
    parser = argparse.ArgumentParser(description = __doc__, formatter_class = argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--src", default = os.path.join(Here, "yelanTexcoord.buf"))
    parser.add_argument("--dest", default = os.path.join(Here, "yelanTexcoordVC.buf"))
    parser.add_argument("--prefix", default = "yelan", help = "the .ib files' prefix (default: %(default)s)")
    parser.add_argument("--objects", nargs = "+", default = ["Body"], help = "the mod objects whose vertices are edited (default: %(default)s)")
    parser.add_argument("--set", nargs = "+", default = ["G=128", "B=128"], metavar = "CH=VALUE", help = "channel assignments, R/G/B/A = 0..255 (default: %(default)s)")
    parser.add_argument("--zeroUV1", action = "store_true",
                        help = "also zero TEXCOORD1 (bytes 12..20 of every line) on the edited vertices. Yelan's model carries a second UV on every "
                               "vertex; YelanTranquil's shader only has one on 3010 of 25954 body vertices (her sheer panels), so a mod feeding it "
                               "Yelan's second UV everywhere may be sampling something it should not")
    args = parser.parse_args()

    raw = np.fromfile(args.src, dtype = np.uint8)
    if (len(raw) % Stride):
        raise SystemExit(f"'{args.src}' is not a whole number of {Stride}-byte lines")
    rows = raw.reshape(-1, Stride).copy()

    vertices = set()
    for objectName in args.objects:
        ibPath = os.path.join(os.path.dirname(os.path.abspath(args.src)), f"{args.prefix}{objectName}.ib")
        ib = np.fromfile(ibPath, dtype = "<u4")
        vertices.update(np.unique(ib).tolist())
    index = np.array(sorted(vertices), dtype = np.int64)

    edits = []
    for item in args.set:
        channel, value = item.split("=", 1)
        edits.append((Channels[channel.upper()], int(value)))

    before = {ch: np.bincount(rows[index, Channels[ch]], minlength = 256) for ch in "RGBA"}
    for column, value in edits:
        rows[index, column] = value
    if (args.zeroUV1):
        rows[index, 12:20] = 0
        print(f"TEXCOORD1 zeroed on {len(index)} vertices")
    after = {ch: np.bincount(rows[index, Channels[ch]], minlength = 256) for ch in "RGBA"}

    print(f"{len(rows)} vertices, {len(index)} in {', '.join(args.objects)}")
    for ch in "RGBA":
        top = lambda counts: [(int(v), int(counts[v])) for v in np.argsort(counts)[::-1][:4] if counts[v]]
        print(f"  {ch}: {top(before[ch])} -> {top(after[ch])}")
    rows.tofile(args.dest)
    print(f"wrote: {args.dest} ({os.path.getsize(args.dest)} bytes)")
