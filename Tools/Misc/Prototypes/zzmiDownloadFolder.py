#
# ===== zzmiDownloadFolder =====
#
# Builds a Zenless Zone Zero character's Data/Mod Downloads/ZZZ/<Name>/<X_Y> folder straight from a
# FRAME DUMP -- the first ZZZ folders (Miyabi / MiyabiBlossom, 2026-10-15) had no asset repo to start
# from, so the dump is the source, the way the Lumine / Chisa folders were made.
#
# A ZZMI character is drawn the GIMI way: one PRE-SKIN pass per component on the root vertex shader
# (e8425f64cfb887cd at 3.2) binds the component's three mod-facing buffers --
#
#   vb0 = Position (40 bytes: POSITION, NORMAL, TANGENT)
#   vb1 = Texcoord (stride MEASURED per component: 24 on Miyabi's Body, 20 on her others, 48 on the face)
#   vb2 = Blend    (32 bytes: 4 x float weight, 4 x uint index)
#
# -- and every later draw of that component binds the same vb1 next to its ib. 3DMigoto dumps each
# bound buffer WHOLE, so the three .buf files of the pre-skin pass are the download's buffers byte for
# byte; nothing here re-encodes them. Each object is one (first index, index count) range of the ib,
# lettered A, B, C ... in first-index order -- ZZMI's `match_first_index` objects -- and written as
# R32_UINT, as every GIMI download .ib is, whatever the game's own format.
#
# Textures are copied per object from the draw on that object's range whose vertex shader the role
# table below knows, named <Name><Comp><Obj><Role>.dds. Only .dds is accepted: 3DMigoto's default
# dump_tex writes some textures as .jpg (the shared normal map ebac056e among them), so pass a second
# dump taken with `dump --options "dump_tex dds"` through --texDumps when one is reported missing.
#
#   py -3 zzmiDownloadFolder.py <dump> <out folder> --name Miyabi --comp Body=8ecb10b3 --comp Hair=f9b9b064 ...
#   py -3 zzmiDownloadFolder.py <dump> <out folder> ... --texDumps <dds dump> --verify
#
# --comp <Name>=<position hash> names each component (the dump has no names; mods disagree on them).
# --verify re-derives every buffer and ib a SECOND way -- from the dump's .txt listings, parsed and
# packed here, never from the .buf files -- and compares (Creating Remaps' "Proving a NEW download
# folder": verify a new folder a second way, not the same way twice).
#
# It also writes <Name>Hash.json: per component the hashes a HashData / IndexData / VertexCountData row
# needs (ib, draw_vb, position_vb, blend_vb, texcoord_vb, object first indices and counts, texture
# hashes by role), in the field names of a GIMI asset folder's hash.json.
#

import argparse
import collections
import filecmp
import json
import os
import re
import shutil
import struct
import sys
import tempfile

RootVs = "e8425f64cfb887cd"

# vertex shader -> {ps-t slot: role}, read off the 3.2 dumps (the mods' own Resource\ZZMI\<Role> names
# agree: Diffuse 09a2bbd1 at t3, NormalMap ebac056e at t4, LightMap fd289380 at t5, MaterialMap 450770fd
# at t6 on Miyabi's body).
RoleTables = {
    "c280f6945b23a42a": {3: "Diffuse", 4: "NormalMap", 5: "LightMap", 6: "MaterialMap"},   # body / clothes / hair
    "864fd078e6447942": {3: "Diffuse"},                                                    # face
    "1e40ef6806fcf348": {3: "Diffuse"},                                                    # face, its third object
    "bcddceab76ee10ce": {2: "Diffuse", 3: "LightMap", 6: "MaterialMap"},                   # hair shadow
}

FileRe = re.compile(r"^(\d{6})-([a-z0-9-]+)=(?:!\w!=)?([0-9a-f]{8})-vs=([0-9a-f]{16})(?:-ps=([0-9a-f]{16}))?\.(\w+)$")


class Dump:
    def __init__(self, folder: str):
        self.folder = folder
        self.draws = collections.defaultdict(list)    # draw id -> [(slot, hash, vs, ps, ext, file)]
        for name in os.listdir(folder):
            m = FileRe.match(name)
            if (m):
                self.draws[m.group(1)].append((m.group(2), m.group(3), m.group(4), m.group(5), m.group(6), name))

    def slot(self, draw: str, slot: str, ext: str = None):
        for s, h, vs, ps, e, name in self.draws[draw]:
            if (s == slot and (ext is None or e == ext)):
                return h, os.path.join(self.folder, name)
        return None, None

    def vs(self, draw: str):
        return self.draws[draw][0][2] if self.draws[draw] else None


def readHeader(path: str) -> dict:
    result = {}
    with open(path, encoding = "utf-8", errors = "replace") as f:
        for line in f:
            if (not line.strip() or line.startswith("element[") or line.startswith("vertex-data")):
                break
            if (":" in line):
                key, value = line.split(":", 1)
                result[key.strip()] = value.strip()
    return result


def findTexture(dumps, texHash: str):
    for dump in dumps:
        for draw, files in dump.draws.items():
            for s, h, vs, ps, e, name in files:
                if (h == texHash and e == "dds" and s.startswith("ps-t")):
                    return os.path.join(dump.folder, name)
    return None


def collect(dump: Dump, posHash: str):
    """The pre-skin draw and every ib range drawn with the component's texcoord buffer"""
    pre = [d for d in sorted(dump.draws) if dump.vs(d) == RootVs and dump.slot(d, "vb0", "buf")[0] == posHash]
    if (not pre):
        raise SystemExit(f"no pre-skin draw (vs={RootVs}) binds vb0={posHash}")
    pre = pre[0]
    texcoordHash = dump.slot(pre, "vb1", "buf")[0]

    objects = {}      # first index -> (count, [draw ids])
    ibHash = drawVb = None
    for draw in sorted(dump.draws):
        if (draw == pre or dump.slot(draw, "vb1", "buf")[0] != texcoordHash):
            continue
        h, txt = dump.slot(draw, "ib", "txt")
        if (h is None):
            continue
        header = readHeader(txt)
        if ("first index" not in header):     # a later pass re-binding the ib without drawing from it
            continue
        if (ibHash not in (None, h)):
            raise SystemExit(f"component vb0={posHash} draws two ibs: {ibHash} and {h}")
        ibHash = h
        drawVb = drawVb or dump.slot(draw, "vb0", "buf")[0]
        first, count = int(header["first index"]), int(header["index count"])
        if (first in objects and objects[first][0] != count):
            raise SystemExit(f"ib {h}: first index {first} drawn with two counts")
        objects.setdefault(first, (count, []))[1].append(draw)
    if (not objects):
        raise SystemExit(f"component vb0={posHash}: no ib draw binds its texcoord {texcoordHash}")
    return pre, texcoordHash, ibHash, drawVb, dict(sorted(objects.items()))


def build(dump: Dump, texDumps, out: str, name: str, comps, report = True):
    os.makedirs(out, exist_ok = True)
    written, notes, hashData = [], [], []

    for comp, posHash in comps:
        pre, texcoordHash, ibHash, drawVb, objects = collect(dump, posHash)
        entry = {"component_name": comp, "root_vs": RootVs, "draw_vb": drawVb, "position_vb": posHash,
                 "blend_vb": dump.slot(pre, "vb2", "buf")[0], "texcoord_vb": texcoordHash, "ib": ibHash}

        for slot, part in (("vb0", "Position"), ("vb1", "Texcoord"), ("vb2", "Blend")):
            h, buf = dump.slot(pre, slot, "buf")
            header = readHeader(dump.slot(pre, slot, "txt")[1])
            stride, vertexCount = int(header["stride"]), int(header["vertex count"])
            data = open(buf, "rb").read()
            if (len(data) != stride * vertexCount):
                raise SystemExit(f"{comp} {part}: {len(data)} bytes is not {vertexCount} x {stride}")
            dst = f"{name}{comp}{part}.buf"
            open(os.path.join(out, dst), "wb").write(data)
            written.append(dst)
            entry[f"{part.lower()}_stride"] = stride
            entry["vertex_count"] = vertexCount

        _, ibBuf = dump.slot(objects[next(iter(objects))][1][0], "ib", "buf")
        ibHeader = readHeader(dump.slot(objects[next(iter(objects))][1][0], "ib", "txt")[1])
        ibData = open(ibBuf, "rb").read()
        fmt = {"DXGI_FORMAT_R16_UINT": "H", "DXGI_FORMAT_R32_UINT": "I"}[ibHeader["format"]]
        allIndices = struct.unpack(f"<{len(ibData) // struct.calcsize(fmt)}{fmt}", ibData)

        entry["object_indexes"], entry["object_counts"], entry["object_classifications"], entry["texture_hashes"] = [], [], [], []
        for letter, (first, (count, draws)) in zip("ABCDEFGHIJ", objects.items()):
            indices = allIndices[first:first + count]
            dst = f"{name}{comp}{letter}.ib"
            open(os.path.join(out, dst), "wb").write(struct.pack(f"<{len(indices)}I", *indices))
            written.append(dst)
            entry["object_indexes"].append(first)
            entry["object_counts"].append(count)
            entry["object_classifications"].append(letter)

            roleDraw = next((d for d in draws if dump.vs(d) in RoleTables), None)
            textures = []
            if (roleDraw is None):
                notes.append(f"{comp}{letter}: no draw on a vertex shader the role table knows ({sorted({dump.vs(d) for d in draws})})")
            else:
                for slot, role in RoleTables[dump.vs(roleDraw)].items():
                    texHash = dump.slot(roleDraw, f"ps-t{slot}")[0]
                    if (texHash is None):
                        notes.append(f"{comp}{letter}{role}: ps-t{slot} unbound")
                        continue
                    textures.append([role, ".dds", texHash])
                    src = findTexture([dump] + texDumps, texHash)
                    if (src is None):
                        notes.append(f"{comp}{letter}{role}: {texHash} is in no dump as .dds (re-dump with dump_tex dds)")
                        continue
                    dst = f"{name}{comp}{letter}{role}.dds"
                    shutil.copyfile(src, os.path.join(out, dst))
                    written.append(dst)
            entry["texture_hashes"].append(textures)
        hashData.append(entry)

    with open(os.path.join(out, f"{name}Hash.json"), "w", encoding = "utf-8", newline = "\n") as f:
        json.dump(hashData, f, indent = 4)
        f.write("\n")
    written.append(f"{name}Hash.json")

    if (report):
        for entry in hashData:
            print(f"{entry['component_name']:>10}  ib {entry['ib']}  draw_vb {entry['draw_vb']}  position {entry['position_vb']}  "
                  f"blend {entry['blend_vb']}  texcoord {entry['texcoord_vb']} (stride {entry['texcoord_stride']})  "
                  f"{entry['vertex_count']} vertices  objects {list(zip(entry['object_classifications'], entry['object_indexes'], entry['object_counts']))}")
        print(f"wrote {len(written)} files -> {out}")
        for note in notes:
            print("NOTE:", note)
    return hashData, notes


# ------------------------------------------------------------------ --verify: the second way

def halfBits(text: str) -> int:
    """An R16_FLOAT value as 3DMigoto lists it, back to its 16 bits. A half DENORMAL is listed wrongly:
    its mantissa shifted into a float32's bits without renormalising (half 0x0001 prints as
    1.1479437e-41, float32 bits 0x00002000), so such a value is inverted from those bits rather than
    rounded -- Miyabi's Legs texcoord has 36 of them."""
    value = float(text)
    if (value != 0 and abs(value) < 1.17549435e-38):
        bits = struct.unpack("<I", struct.pack("<f", value))[0]
        return ((bits >> 16) & 0x8000) | ((bits & 0x7FFFFF) >> 13)
    return struct.unpack("<H", struct.pack("<e", value))[0]


Packers = {
    "R32G32B32_FLOAT": ("<3f", float), "R32G32B32A32_FLOAT": ("<4f", float), "R32G32_FLOAT": ("<2f", float),
    "R16G16_FLOAT": ("<2H", halfBits), "R32G32B32A32_UINT": ("<4I", int), "R32G32B32A32_SINT": ("<4i", int),
    "R8G8B8A8_UNORM": ("<4B", lambda v: int(round(float(v) * 255))), "R32_FLOAT": ("<f", float),
}


def bytesFromTxt(path: str) -> bytes:
    """A vertex buffer rebuilt from 3DMigoto's text listing alone"""
    lines = open(path, encoding = "utf-8", errors = "replace").read().splitlines()
    header = readHeader(path)
    stride, count = int(header["stride"]), int(header["vertex count"])
    formats, current = {}, {}
    for line in lines:
        line = line.strip()
        if (line.startswith("SemanticName:")):
            current = {"name": line.split(":", 1)[1].strip()}
        elif (line.startswith("SemanticIndex:")):
            current["index"] = int(line.split(":", 1)[1])
        elif (line.startswith("Format:")):
            key = current["name"] + (str(current["index"]) if current.get("index") else "")
            formats.setdefault(key, line.split(":", 1)[1].strip().replace("DXGI_FORMAT_", ""))
    out = bytearray(stride * count)
    covered = collections.defaultdict(list)
    for line in lines:
        m = re.match(r"vb\d+\[(\d+)\]\+(\d+) (\w+): (.*)$", line.strip())
        if (not m):
            continue
        vertex, offset, sem, values = int(m.group(1)), int(m.group(2)), m.group(3), m.group(4)
        fmt, conv = Packers[formats[sem]]
        size = struct.calcsize(fmt)
        if (any(offset < end and start < offset + size for start, end in covered[vertex])):
            continue        # an alias over bytes another element already wrote (vb0's TEXCOORD4..7)
        covered[vertex].append((offset, offset + size))
        struct.pack_into(fmt, out, vertex * stride + offset, *[conv(v) for v in values.split(",")])
    return bytes(out)


def verify(dump: Dump, out: str, name: str, comps) -> int:
    bad = checked = 0
    for comp, posHash in comps:
        pre, texcoordHash, ibHash, drawVb, objects = collect(dump, posHash)
        for slot, part in (("vb0", "Position"), ("vb1", "Texcoord"), ("vb2", "Blend")):
            mine = open(os.path.join(out, f"{name}{comp}{part}.buf"), "rb").read()
            again = bytesFromTxt(dump.slot(pre, slot, "txt")[1])
            checked += 1
            if (mine != again):
                bad += 1
                diff = sum(a != b for a, b in zip(mine, again))
                print(f"MISMATCH {name}{comp}{part}.buf: {diff} bytes differ ({len(mine)} vs {len(again)})")
        for letter, (first, (count, draws)) in zip("ABCDEFGHIJ", objects.items()):
            text = open(dump.slot(draws[0], "ib", "txt")[1], encoding = "utf-8").read().split("\n\n", 1)[1]
            indices = [int(v) for v in text.split()]
            mine = open(os.path.join(out, f"{name}{comp}{letter}.ib"), "rb").read()
            checked += 1
            if (len(indices) != count or mine != struct.pack(f"<{len(indices)}I", *indices)):
                bad += 1
                print(f"MISMATCH {name}{comp}{letter}.ib")
    print(f"verify: {checked} files checked against the dump's .txt listings, {bad} mismatched")
    return bad


def main():
    parser = argparse.ArgumentParser(description = "a ZZZ download folder from a frame dump")
    parser.add_argument("dump")
    parser.add_argument("out")
    parser.add_argument("--name", required = True)
    parser.add_argument("--comp", action = "append", required = True, help = "<Component>=<position vb hash>")
    parser.add_argument("--texDumps", nargs = "*", default = [], help = "more dumps to take .dds textures from")
    parser.add_argument("--verify", action = "store_true")
    parser.add_argument("--check", action = "store_true", help = "build into a temp folder and compare with <out>")
    args = parser.parse_args()

    comps = [tuple(c.split("=", 1)) for c in args.comp]
    dump = Dump(args.dump)
    texDumps = [Dump(d) for d in args.texDumps]

    if (args.check):
        with tempfile.TemporaryDirectory() as tmp:
            build(dump, texDumps, tmp, args.name, comps, report = False)
            names = sorted(set(os.listdir(tmp)) | set(os.listdir(args.out)))
            match, mismatch, errors = filecmp.cmpfiles(tmp, args.out, names, shallow = False)
            print(f"check: {len(match)} identical, {len(mismatch)} different {mismatch}, {len(errors)} on one side only {errors}")
            sys.exit(1 if (mismatch or errors) else 0)

    build(dump, texDumps, args.out, args.name, comps)
    if (args.verify):
        sys.exit(1 if verify(dump, args.out, args.name, comps) else 0)


if __name__ == "__main__":
    main()
