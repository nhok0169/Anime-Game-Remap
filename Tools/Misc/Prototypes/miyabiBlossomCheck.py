"""Structural checks over a mod folder fixed Miyabi -> MiyabiBlossom (miyabiBlossomFix.py), the pre-flight before an
in-game round: every filename = line resolves, no *NotFound placeholder, every remapped section's hash is its target
component's, and every remapped Blend.buf is its source with exactly the shipped vertex group row applied (weights
unchanged, every index inside the target palette). Shown to fail on a copy with one flipped blend index and one
source hash left in a remapped section.

    py -3 miyabiBlossomCheck.py <fixed mod folder>
"""
import json
import os
import re
import struct
import sys

R = r"E:/Computer/Games/Genshin/Repos/Repos/Fix-Raiden-Boss"
Routes = {"Body": "Dress", "Legs": "Body", "Hair": "Hair", "HairShadow": "HairShadow"}
Palette = {"Dress": 77, "Body": 116, "Hair": 39, "HairShadow": 39}


def hashesOf(name):
    out = {}
    for e in json.load(open(f"{R}/Data/Mod Downloads/ZZZ/{name}/3_2/{name}Hash.json")):
        for k in ("ib", "draw_vb", "position_vb", "blend_vb", "texcoord_vb"):
            out.setdefault(e["component_name"], set()).add(e[k])
        # ...and its textures: a recolour's texture-hash sections are copied onto them
        for obj in e.get("texture_hashes", []):
            out[e["component_name"]].update(h for _, _, h in obj)
    # the sword's textures, which no Hash.json lists: HashData files them with the outfit (Body / Dress),
    # whose fixer carries a weapon recolour onto the skin's
    out.setdefault("Body" if name == "Miyabi" else "Dress", set()).update(
        {"718d7915", "b73ed7e7", "e1603ca5"} if name == "Miyabi" else {"e1215530", "9d2adcc5", "4659445f"})
    return out


def vgRows():
    sys.path.insert(0, R + "/Anime Game Remap (for all users)/api/src/py")
    import FixRaidenBoss2 as F
    mt = {m.name: m for m in F.ZZMIBuilder.all()}["Miyabi"]
    rows = {s: dict(mt.getVGRemap("MiyabiBlossom", fromComp = s, toComp = t).remap) for s, t in Routes.items()}
    # the prototype's in-game corrections to the shipped rows
    sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
    from miyabiBlossomFix import RowOverrides
    for (s, t), over in RowOverrides.items():
        rows[s].update(over)
    return rows


def sections(text):
    out, cur = {}, None
    for line in text.splitlines():
        m = re.match(r"\s*\[([^\]]+)\]\s*$", line)
        if m:
            cur = m.group(1)
            out[cur] = []
        elif cur is not None:
            out[cur].append(line)
    return out


def main(folder):
    bad = 0
    src, dst = hashesOf("Miyabi"), hashesOf("MiyabiBlossom")
    rows = vgRows()
    for root, _, files in os.walk(folder):
        for f in files:
            if not f.lower().endswith(".ini") or f.upper().startswith("DISABLED"):
                continue
            path = os.path.join(root, f)
            text = open(path, encoding = "utf-8", errors = "replace").read()
            if "Remap" not in text:
                continue
            secs = sections(text)
            checked = 0
            for name, body in secs.items():
                for line in body:
                    m = re.match(r"\s*filename\s*=\s*(.+?)\s*$", line, re.I)
                    if m:
                        checked += 1
                        p = os.path.join(root, m.group(1).replace("\\", os.sep))
                        if not os.path.isfile(p):
                            print(f"  DANGLING {f} [{name}] {m.group(1)}"); bad += 1
                if any("NotFound" in l for l in body):
                    print(f"  NOTFOUND {f} [{name}]"); bad += 1
                for s, t in Routes.items():
                    if f"MiyabiBlossom{t}Remap" in name and name.startswith("TextureOverride"):
                        for line in body:
                            m = re.match(r"\s*hash\s*=\s*([0-9a-fA-F]{8})", line)
                            if m and m.group(1).lower() not in dst[t]:
                                print(f"  HASH {f} [{name}] {m.group(1)} is not MiyabiBlossom {t}'s"); bad += 1
            print(f"{os.path.relpath(path, folder)}: {checked} filename lines checked")
    # blends: every remapped blend against its source with the row applied
    for root, _, files in os.walk(folder):
        for f in files:
            m = re.match(r"(.+?)MiyabiBlossom(\w+?)RemapBlend\.buf$", f)
            if not m or "Texcoord" in f:
                continue
            # <prefix>MiyabiBlossom<target>RemapBlend.buf is made from <prefix>Blend.buf, whatever the prefix
            # (MiyabiLegs..., or a mod's own Legs...)
            comp = next((s for s, t in Routes.items() if t == m.group(2)), None)
            orig = os.path.join(root, m.group(1) + "Blend.buf")
            if comp is None or not os.path.isfile(orig):
                print(f"  UNCHECKED {f}: no {m.group(1)}Blend.buf beside it")
                continue
            a, b = open(orig, "rb").read(), open(os.path.join(root, f), "rb").read()
            if len(a) != len(b):
                print(f"  BLEND {f}: size {len(b)} vs source {len(a)}"); bad += 1; continue
            row, wrong, outOfRange = rows[comp], 0, 0
            for i in range(0, len(a), 32):
                wa, wb = a[i:i + 16], b[i:i + 16]
                ia, ib = struct.unpack_from("<4I", a, i + 16), struct.unpack_from("<4I", b, i + 16)
                weights = struct.unpack_from("<4f", a, i)
                want = tuple(row.get(x, x) if w > 0 else None for x, w in zip(ia, weights))
                got = tuple(y if w > 0 else None for y, w in zip(ib, weights))
                if wa != wb or want != got:
                    wrong += 1
                outOfRange += sum(1 for y, w in zip(ib, weights) if w > 0 and y >= Palette[Routes[comp]])
            print(f"{f}: {len(a) // 32} vertices, {wrong} differ from the row, {outOfRange} indices outside the target palette")
            bad += bool(wrong) + bool(outOfRange)
    print("ALL CLEAN" if not bad else f"{bad} PROBLEM(S)")
    return bad


if __name__ == "__main__":
    sys.exit(1 if main(sys.argv[1]) else 0)
