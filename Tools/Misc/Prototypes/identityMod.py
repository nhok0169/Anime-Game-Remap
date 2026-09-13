#
# ===== identityMod =====
#
# Builds a character's IDENTITY mod: the game's own model, spelled out as a GIMI mod from the
# character's asset folder (GI-Model-Importer-Assets' PlayerCharacterData/<Name>: hash.json, the
# *-vb0=<hash>.txt / *-ib=<hash>.txt dumps and the .dds textures). The result is what a remap sees
# when the "mod" is the original model -- every object the character has, drawn exactly as the game
# draws it -- so a remap prototype can be checked against the whole skin at once instead of against
# whatever a downloaded mod happens to use (a china dress uses no jacket bones, a Fontaine outfit no
# dress).
#
#   python identityMod.py <asset folder> <mod folder>            e.g. PlayerCharacterData/Yelan  Mods/Yelan4/Yelan
#   python identityMod.py <asset folder> <mod folder> --noFix    leave the ORFix / NNFix run lines out
#
# Runs on Windows or Linux / WSL (a Windows-form path is translated to /mnt/<drive>/... on Linux). The
# buffers come out of the API's own dump readers (VbFile / IbFile.readDumpStr, the same path the
# Tools/DumpToModConverter notebook uses): the dump's 92-byte vertex is the Position.buf (40:
# POSITION / NORMAL / TANGENT), the Blend.buf (32: BLENDWEIGHT / BLENDINDICES) and the Texcoord.buf
# (20: COLOR / TEXCOORD / TEXCOORD1) laid end to end, and each object's ib is written as R32_UINT
# whatever the game's own format was, which is what every GIMI mod declares.
#

import argparse
import json
import os
import shutil
import sys

import numpy as np

OnWindows = (sys.platform == "win32")


def winToPosix(path: str) -> str:
    if (OnWindows or (len(path) < 2) or (path[1] != ":") or (not path[0].isalpha())):
        return path
    return "/mnt/" + path[0].lower() + path[2:].replace("\\", "/")


Repo = os.environ.get("AG_REMAP_REPO") or winToPosix(r"E:\Computer\Games\Genshin\Repos\Repos\Fix-Raiden-Boss")
APISrc = os.path.join(Repo, "Anime Game Remap (for all users)", "api", "src", "py")
if (not os.path.isdir(APISrc)):
    raise SystemExit(f"the API's package folder is not at {APISrc}; set AG_REMAP_REPO to the repo's path on this OS")
sys.path.insert(0, APISrc)
if (hasattr(os, "add_dll_directory")):
    os.add_dll_directory(os.path.join(APISrc, "FixRaidenBoss2"))

import FixRaidenBoss2 as FRB     # noqa: E402

# which .buf file each dump element belongs to, in the order GIMI lays them out
BufOf = {"POSITION": "Position", "NORMAL": "Position", "TANGENT": "Position",
         "BLENDWEIGHT": "Blend", "BLENDWEIGHTS": "Blend", "BLENDINDICES": "Blend",
         "COLOR": "Texcoord", "TEXCOORD": "Texcoord"}
Strides = {"Position": 40, "Blend": 32, "Texcoord": 20}
NNFix = "CommandList\\global\\ORFix\\NNFix"


def readText(path: str) -> str:
    with open(path, "r", encoding = "utf-8") as f:
        return f.read()


def bufsFromDump(vbPath: str):
    """The three .buf byte blocks out of one vb0 dump, keyed Position / Blend / Texcoord, plus the vertex count"""
    vb = FRB.VbFile(b"", [])
    vb.readDumpStr(readText(vbPath))
    ranges = {}
    offset = 0
    for element in vb.elements:
        name = element.name.rstrip("0123456789")       # TEXCOORD1 -> TEXCOORD
        part = BufOf.get(name)
        if (part is None):
            raise SystemExit(f"{os.path.basename(vbPath)}: element {element.name} belongs to no .buf file")
        r = ranges.setdefault(part, [offset, offset])
        if (r[1] != offset):
            raise SystemExit(f"{os.path.basename(vbPath)}: {part} elements are not contiguous ({element.name} at {offset})")
        r[1] = offset + element.size
        offset += element.size
    data = np.frombuffer(bytes(vb.data), dtype = np.uint8)
    n = vb.getVertexCount()
    lines = data.reshape(n, vb.bytesPerLine)
    out = {}
    for part, (a, b) in ranges.items():
        if (b - a != Strides[part]):
            raise SystemExit(f"{os.path.basename(vbPath)}: {part} spans {b - a} bytes, GIMI's stride is {Strides[part]}")
        out[part] = lines[:, a:b].tobytes()
    return out, n


def ibFromDump(ibPath: str) -> np.ndarray:
    """One object's indices, as uint32"""
    ib = FRB.IbFile(b"")
    ib.readDumpStr(readText(ibPath))
    raw = bytes(ib.data)
    count = ib.getIndexCount() if hasattr(ib, "getIndexCount") else None
    width = len(raw) // count if count else (2 if ("R16" in readText(ibPath)[:400]) else 4)
    return np.frombuffer(raw, dtype = np.uint16 if (width == 2) else np.uint32).astype(np.uint32)


def main():
    parser = argparse.ArgumentParser(description = "a character's identity mod from its asset folder")
    parser.add_argument("assets", help = "the asset folder (hash.json, *-vb0=*.txt, *-ib=*.txt, *.dds)")
    parser.add_argument("mod", help = "the mod folder to write (created)")
    parser.add_argument("--name", default = None, help = "the character name used in the file and section names (default: the asset folder's name)")
    parser.add_argument("--noFix", action = "store_true", help = "leave the ORFix / NNFix run lines out of the object sections")
    args = parser.parse_args()
    assets = winToPosix(args.assets)
    modFolder = winToPosix(args.mod)
    name = args.name or os.path.basename(os.path.normpath(assets))

    hashes = json.load(open(os.path.join(assets, "hash.json"), encoding = "utf-8"))
    main_ = next(h for h in hashes if h.get("position_vb"))
    face = next((h for h in hashes if h.get("component_name") == "Face"), None)
    objects = list(main_["object_classifications"])
    firstIndices = list(main_["object_indexes"])
    if (main_.get("component_name")):
        raise SystemExit(f"{name} is a character of several components ({main_['component_name']}); this script builds single-component identities")

    os.makedirs(modFolder, exist_ok = True)
    # ---- buffers (every object's vb0 dump is the same buffer; the first one serves) ----
    vbPath = os.path.join(assets, f"{name}{objects[0]}-vb0={main_['position_vb']}.txt")
    bufs, vertexCount = bufsFromDump(vbPath)
    for part, data in bufs.items():
        with open(os.path.join(modFolder, f"{name}{part}.buf"), "wb") as f:
            f.write(data)
    ibCounts = {}
    for obj in objects:
        ib = ibFromDump(os.path.join(assets, f"{name}{obj}-ib={main_['ib']}.txt"))
        if (ib.size and ib.max() >= vertexCount):
            raise SystemExit(f"{obj}: index {ib.max()} beyond the {vertexCount} vertices")
        ib.tofile(os.path.join(modFolder, f"{name}{obj}.ib"))
        ibCounts[obj] = ib.size
    # ---- textures ----
    textures = {}
    for obj, texList in zip(objects, main_["texture_hashes"]):
        for kind, ext, _ in texList:
            if (ext.lower() != ".dds"):
                continue
            src = os.path.join(assets, f"{name}{obj}{kind}{ext}")
            if (os.path.exists(src)):
                shutil.copy2(src, os.path.join(modFolder, os.path.basename(src)))
                textures.setdefault(obj, {})[kind] = os.path.basename(src)
    faceDiffuse = None
    if (face):
        src = os.path.join(assets, f"{name}FaceHeadDiffuse.dds")
        if (os.path.exists(src)):
            shutil.copy2(src, os.path.join(modFolder, os.path.basename(src)))
            faceDiffuse = os.path.basename(src)

    # ---- the .ini, in the shape GIMI generates ----
    L = [f"; {name}", "", "; Constants -------------------------", "", "; Overrides -------------------------", ""]
    L += [f"[TextureOverride{name}Position]", f"hash = {main_['position_vb']}", f"vb0 = Resource{name}Position", ""]
    L += [f"[TextureOverride{name}Blend]", f"hash = {main_['blend_vb']}", f"vb1 = Resource{name}Blend", "handling = skip", f"draw = {vertexCount},0", ""]
    L += [f"[TextureOverride{name}Texcoord]", f"hash = {main_['texcoord_vb']}", f"vb1 = Resource{name}Texcoord", ""]
    L += [f"[TextureOverride{name}VertexLimitRaise]", f"hash = {main_['draw_vb']}", ""]
    L += [f"[TextureOverride{name}IB]", f"hash = {main_['ib']}", "handling = skip", "drawindexed = auto", ""]
    for obj, first in zip(objects, firstIndices):
        L += [f"[TextureOverride{name}{obj}]", f"hash = {main_['ib']}", f"match_first_index = {first}", f"ib = Resource{name}{obj}IB"]
        for slot, kind in (("ps-t0", "Diffuse"), ("ps-t1", "LightMap")):
            if (kind in textures.get(obj, {})):
                L.append(f"{slot} = Resource{name}{obj}{kind}")
        if (not args.noFix):
            L.append(f"run = {NNFix}")
        L.append("")
    if (faceDiffuse and face["texture_hashes"][0]):
        faceHash = next((h for kind, _, h in face["texture_hashes"][0] if kind == "Diffuse"), None)
        if (faceHash):
            L += [f"[TextureOverride{name}FaceHeadDiffuse]", f"hash = {faceHash}", f"ps-t0 = Resource{name}FaceHeadDiffuse", ""]
    L += ["", "; CommandList -----------------------", "", "; Resources -------------------------", ""]
    for part in ("Position", "Blend", "Texcoord"):
        L += [f"[Resource{name}{part}]", "type = Buffer", f"stride = {Strides[part]}", f"filename = {name}{part}.buf", ""]
    for obj in objects:
        L += [f"[Resource{name}{obj}IB]", "type = Buffer", "format = DXGI_FORMAT_R32_UINT", f"filename = {name}{obj}.ib", ""]
    for obj in objects:
        for kind, file in textures.get(obj, {}).items():
            L += [f"[Resource{name}{obj}{kind}]", f"filename = {file}", ""]
    if (faceDiffuse):
        L += [f"[Resource{name}FaceHeadDiffuse]", f"filename = {faceDiffuse}", ""]
    L += ["", f"; the identity mod of {name}: the game's own model out of its asset folder ({os.path.basename(os.path.normpath(assets))}), built by identityMod.py", ""]
    with open(os.path.join(modFolder, f"{name}.ini"), "w", encoding = "utf-8", newline = "\r\n") as f:
        f.write("\n".join(L))

    print(f"{name}: {vertexCount} vertices; " + ", ".join(f"{obj} {ibCounts[obj] // 3} triangles from index {first}" for obj, first in zip(objects, firstIndices)))
    print(f"  textures: " + ", ".join(f"{obj} {'/'.join(t)}" for obj, t in textures.items()) + (f"; face {faceDiffuse}" if faceDiffuse else ""))
    print(f"  written to {modFolder}")


if (__name__ == "__main__"):
    main()
