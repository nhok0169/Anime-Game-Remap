"""Builds the Miyabi download files a Miyabi -> MiyabiBlossom fix needs beyond the five components of WWMI-style
assets, from Miyabi's character-menu frame dump (3.2):

- her two hair accessories, which the game draws as objects of their own and only for her base outfit, so the skin
  never draws them (epoch 1: "the hair clasp is missing on every mod"):
    MiyabiHairAccessory1  ib ef64351c, the clasp plate   (pre-skinned at draw 35, drawn at 45)
    MiyabiHairAccessory2  ib 4b465238, the gold charms   (pre-skinned at draw 33, drawn at 44)
  Each is written in the Hair component's layout -- Position (40), Blend (32), Texcoord (20, the hair's
  COLOR / half2 / float2 / half2) and an R32 index buffer -- with every vertex weighted 1.0 to HAIR bone 0, so the
  fix can pre-skin it on the Hair pass. The plate's own bone IS hair bone 0 (identical to the float); the charms
  hang on four physics bones of their own that the skin has no counterpart for, so they are attached rigidly at
  their rest positions (they no longer swing). Their rest positions sit just under the plate, as on the base card;
  the dump's posed charm buffer (draw 44's vb0) does NOT -- back-solved through hair bone 0 it lands at the front
  of the head, 14.5 cm away -- so the rest positions are used, not that buffer. Checked: the plate posed on hair
  bone 0 reproduces draw 45's vb0 exactly. Both read the Hair atlas.
- the frozen ps-cb3 (2048 B, the per-draw material table) of her Legs draw, main pass (draw 48, ps a24c103a) and
  outline pass (draw 59, ps a9f95d03): the skin's Body draws have the stocking material switched off, and no
  texture edit can switch it on. Rows 29, 30, 97, 102, 105, 120, 124 and 127 change between sessions and scenes
  and are pinned at their menu values by this copy -- an update risk, chosen by the maintainer (2026-10-27).

    py -3 zzmiMiyabiExtras.py [<frame dump>] [<out folder>]
"""
import os
import struct
import sys

import numpy as np

Z = r"E:/Computer/Games/Wuthering Waves Mods/Importer/ZZMI"
DUMP = Z + "/FrameAnalysis-Miyabi-2026-10-15-093322"
OUT = r"E:/Computer/Games/Genshin/Repos/Repos/Fix-Raiden-Boss/Data/Mod Downloads/ZZZ/Miyabi/3_2"
# name -> (pre-skin draw, main draw, ib hash)
Accessories = {"HairAccessory1": ("000035", "000045", "ef64351c"),
               "HairAccessory2": ("000033", "000044", "4b465238")}
Cb3 = {"LegsMainCb3": "000048", "LegsOutlineCb3": "000059"}


def dumpFile(dump, draw, slot, ext = ".buf"):
    hits = [f for f in os.listdir(dump) if f.startswith(f"{draw}-{slot}=") and f.endswith(ext)]
    assert len(hits) == 1, (draw, slot, hits)
    return os.path.join(dump, hits[0])


def header(path):
    """the key: value lines of a dump .txt before its data"""
    out = {}
    for line in open(path, encoding = "utf-8", errors = "replace"):
        if ":" in line and not line.startswith((" ", "\t", "vb", "element")):
            k, v = line.split(":", 1)
            out.setdefault(k.strip(), v.strip())
    return out


def main(dump, out):
    os.makedirs(out, exist_ok = True)
    for name, (pre, draw, ibHash) in Accessories.items():
        pos = open(dumpFile(dump, pre, "vb0"), "rb").read()
        n = int(header(dumpFile(dump, pre, "vb0", ".txt"))["vertex count"])
        assert len(pos) == n * 40, (name, len(pos), n)
        # texcoord: the pre-skin's vb1 (stride 28: COLOR, float2, float2, float2) -> the hair's (stride 20: COLOR,
        # half2, float2, half2); the main draw binds the same buffer
        tcSrc = open(dumpFile(dump, draw, "vb1"), "rb").read()
        assert len(tcSrc) == n * 28, (name, len(tcSrc))
        tc = bytearray()
        for i in range(n):
            v = tcSrc[i * 28:(i + 1) * 28]
            uv0, uv1, uv2 = struct.unpack_from("<2f", v, 4), struct.unpack_from("<2f", v, 12), struct.unpack_from("<2f", v, 20)
            tc += v[:4] + np.array(uv0, np.float16).tobytes() + struct.pack("<2f", *uv1) + np.array(uv2, np.float16).tobytes()
        blend = struct.pack("<4f4I", 1.0, 0.0, 0.0, 0.0, 0, 0, 0, 0) * n
        ibTxt = header(dumpFile(dump, draw, f"ib", ".txt"))
        ibRaw = open(dumpFile(dump, draw, "ib"), "rb").read()
        assert f"-ib={ibHash}-" in dumpFile(dump, draw, "ib"), (name, ibHash)
        count, first = int(ibTxt["index count"]), int(ibTxt["first index"])
        fmt = ibTxt["format"]
        idx = np.frombuffer(ibRaw, np.uint16 if "R16" in fmt else np.uint32)[first:first + count].astype(np.uint32)
        assert idx.max() < n, (name, idx.max(), n)
        for suffix, data in (("Position.buf", pos), ("Blend.buf", blend), ("Texcoord.buf", bytes(tc)),
                             ("A.ib", idx.tobytes())):
            open(os.path.join(out, f"Miyabi{name}{suffix}"), "wb").write(data)
        print(f"Miyabi{name}: {n} vertices, {count} indices ({fmt} -> R32_UINT)")
    for name, draw in Cb3.items():
        data = open(dumpFile(dump, draw, "ps-cb3"), "rb").read()
        assert len(data) == 2048, (name, len(data))
        open(os.path.join(out, f"Miyabi{name}.buf"), "wb").write(data)
        print(f"Miyabi{name}.buf: {len(data)} bytes from draw {draw}")


if __name__ == "__main__":
    main(sys.argv[1] if len(sys.argv) > 1 else DUMP, sys.argv[2] if len(sys.argv) > 2 else OUT)
