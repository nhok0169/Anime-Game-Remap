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
# ---- A character of SEVERAL components (2026-09-13) ----
#
# Newer skins are not one mesh: YelanTranquil is a Body (draw slots A / B / C), a Bang and an Eye,
# each with its OWN position / blend / texcoord / index buffers and hashes, listed in hash.json as
# separate entries with a `component_name`. Every entry with a `position_vb` is such a component and
# gets its own five buffer sections and its own object sections; the asset files of one are named
# <Name><Component><Object>..., which is <Name><Object>... for the older single-component skins
# because their component name is the empty string -- so ONE loop builds both, and a single-component
# character's output is byte-identical to what this script wrote before the components existed.
#
# Two things the component shape brought that a single-component skin never needed:
#
#   * The TEXTURE LAYOUT is per object, not per character. An object whose hash.json lists a
#     NormalMap is drawn by the normal-map shader family and is bound in GIMI's three-register
#     convention under ORFix (ps-t0 normal map, ps-t1 diffuse, ps-t2 light map -- ORFix's
#     CommandListReference reads exactly those and re-slots them to what the shader actually wants,
#     LND: light map, normal map, diffuse); an object without one is the two-register NNFix layout
#     (ps-t0 diffuse, ps-t1 light map -> LDX: light map, diffuse). Both were read off a real frame
#     dump of YelanTranquil (draws 44 / 45 normal-map, 46 not) and off ORFix.ini's own command
#     lists, not guessed from the file names.
#   * A component can have NO textures of its own and READ ANOTHER'S -- hash.json says so by
#     listing an empty texture list, and the dump says which: YelanTranquil's Bang and Eye both
#     draw the Body's slot-A textures. Point them at it with
#     `--textureFrom Bang=Body:A --textureFrom Eye=Body:A:plain`. The layout stays the BORROWER's:
#     the Bang is drawn by the normal-map shader family and the Eye is not, so the Eye takes only
#     the diffuse and the light map -- which is what the `:plain` says, because nothing in
#     hash.json does. Without a --textureFrom such an object is written with its geometry only and
#     NO `run =` line, which is deliberate: ORFix / NNFix re-slot whatever is bound whether the
#     section bound it or not, so running one over the game's own already-correctly-slotted
#     textures scrambles them.
#
# And `--faceRegister`: GI 6.x swapped the face's diffuse and light map registers, so on a 6.x skin
# the face diffuse is bound at ps-t1 and a section that overrides ps-t0 replaces the LIGHT MAP with
# it. The default stays ps-t0 (what the older identity mods were built with); pass `ps-t1` for a
# skin whose dump shows the swap, as YelanTranquil's does.
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
BufOrder = ("Position", "Blend", "Texcoord")
# the Position and the Blend buffer are fixed by GIMI's convention (POSITION + NORMAL + TANGENT,
# BLENDWEIGHTS + BLENDINDICES) and a layout that disagrees is not one this script can write. The
# TEXCOORD buffer is NOT fixed: it is 20 with a second UV set and 12 without, and one component of a
# skin can differ from another -- YelanTranquil's Eye carries no TEXCOORD1 where her Body and Bang
# do -- so its stride is measured off the dump and declared per component in the resource section.
FixedStrides = {"Position": 40, "Blend": 32}
ORFix = "CommandList\\global\\ORFix\\ORFix"
NNFix = "CommandList\\global\\ORFix\\NNFix"

# the two register layouts a GIMI mod declares, keyed by whether the object's shader reads a normal
# map. ORFix / NNFix read these and re-slot them to what the shader wants (see the header).
NormalMapLayout = (("ps-t0", "NormalMap"), ("ps-t1", "Diffuse"), ("ps-t2", "LightMap"))
PlainLayout = (("ps-t0", "Diffuse"), ("ps-t1", "LightMap"))


def readText(path: str) -> str:
    with open(path, "r", encoding = "utf-8") as f:
        return f.read()


def bufsFromDump(vbPath: str):
    """The three .buf byte blocks out of one vb0 dump, keyed Position / Blend / Texcoord, their
    strides, and the vertex count"""
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
    strides = {}
    for part, (a, b) in ranges.items():
        if (part in FixedStrides and (b - a) != FixedStrides[part]):
            raise SystemExit(f"{os.path.basename(vbPath)}: {part} spans {b - a} bytes, GIMI's stride is {FixedStrides[part]}")
        out[part] = lines[:, a:b].tobytes()
        strides[part] = b - a
    return out, strides, n


def ibFromDump(ibPath: str) -> np.ndarray:
    """One object's indices, as uint32"""
    ib = FRB.IbFile(b"")
    ib.readDumpStr(readText(ibPath))
    raw = bytes(ib.data)
    count = ib.getIndexCount() if hasattr(ib, "getIndexCount") else None
    width = len(raw) // count if count else (2 if ("R16" in readText(ibPath)[:400]) else 4)
    return np.frombuffer(raw, dtype = np.uint16 if (width == 2) else np.uint32).astype(np.uint32)


def parseTextureFrom(values):
    """--textureFrom Bang=Body:A -> {"Bang": ("Body", "A", None)}, Eye=Body:A:plain -> (..., "plain")

    The layout is the BORROWER's, not the lender's: YelanTranquil's Bang and Eye both read her Body
    slot A's textures, but the Bang is drawn by the normal-map shader family and the Eye is not, so
    the Eye binds only the diffuse and the light map and runs NNFix. Nothing in hash.json says which
    -- read it off the frame dump (the draw's ps= hash against ORFix.ini's ShaderOverride list) and
    say so here. Left off, the lender's own layout is used.
    """
    out = {}
    for value in (values or []):
        parts = value.split("=", 1)
        if ((len(parts) != 2) or (":" not in parts[1])):
            raise SystemExit(f"--textureFrom wants <component>=<component>:<object>[:normalMap|plain], got {value!r}")
        component, source = parts
        fields = source.split(":")
        if (len(fields) not in (2, 3)):
            raise SystemExit(f"--textureFrom wants <component>=<component>:<object>[:normalMap|plain], got {value!r}")
        layout = fields[2] if (len(fields) == 3) else None
        if (layout not in (None, "normalMap", "plain")):
            raise SystemExit(f"--textureFrom's layout is normalMap or plain, got {layout!r}")
        out[component] = (fields[0], fields[1], layout)
    return out


def main():
    parser = argparse.ArgumentParser(description = "a character's identity mod from its asset folder")
    parser.add_argument("assets", help = "the asset folder (hash.json, *-vb0=*.txt, *-ib=*.txt, *.dds)")
    parser.add_argument("mod", help = "the mod folder to write (created)")
    parser.add_argument("--name", default = None, help = "the character name used in the file and section names (default: the asset folder's name)")
    parser.add_argument("--noFix", action = "store_true", help = "leave the ORFix / NNFix run lines out of the object sections")
    parser.add_argument("--faceRegister", default = "ps-t0", help = "the register the face diffuse is bound to (GI 6.x swapped it to ps-t1 on some skins; default ps-t0)")
    parser.add_argument("--textureFrom", action = "append", default = None, metavar = "COMP=COMP:OBJ[:LAYOUT]",
                        help = "a component with no textures of its own reads another component object's, in the borrower's own normalMap / plain layout (e.g. Bang=Body:A, Eye=Body:A:plain); repeatable")
    args = parser.parse_args()
    assets = winToPosix(args.assets)
    modFolder = winToPosix(args.mod)
    name = args.name or os.path.basename(os.path.normpath(assets))
    textureFrom = parseTextureFrom(args.textureFrom)

    hashes = json.load(open(os.path.join(assets, "hash.json"), encoding = "utf-8"))
    components = [h for h in hashes if h.get("position_vb")]        # every entry with buffers is a component
    face = next((h for h in hashes if h.get("component_name") == "Face"), None)
    if (not components):
        raise SystemExit(f"{name}: hash.json has no entry with a position_vb")
    componentNames = [c.get("component_name", "") for c in components]
    for component in textureFrom:
        if (component not in componentNames):
            raise SystemExit(f"--textureFrom names {component!r}, which is not one of this character's components ({', '.join(componentNames)})")

    os.makedirs(modFolder, exist_ok = True)

    # ---- the geometry and the textures, per component ----
    # `comp` is "" for the older single-component skins, so <name><comp><obj> is their old file naming
    built = {}
    for entry in components:
        comp = entry.get("component_name", "")
        objects = list(entry["object_classifications"])
        firstIndices = list(entry["object_indexes"])

        # every object's vb0 dump is the component's whole vertex buffer; the first one serves
        vbPath = os.path.join(assets, f"{name}{comp}{objects[0]}-vb0={entry['position_vb']}.txt")
        bufs, strides, vertexCount = bufsFromDump(vbPath)
        for part, data in bufs.items():
            with open(os.path.join(modFolder, f"{name}{comp}{part}.buf"), "wb") as f:
                f.write(data)

        ibCounts = {}
        for obj in objects:
            ib = ibFromDump(os.path.join(assets, f"{name}{comp}{obj}-ib={entry['ib']}.txt"))
            if (ib.size and ib.max() >= vertexCount):
                raise SystemExit(f"{comp}{obj}: index {ib.max()} beyond the {vertexCount} vertices")
            ib.tofile(os.path.join(modFolder, f"{name}{comp}{obj}.ib"))
            ibCounts[obj] = ib.size

        textures = {}
        for obj, texList in zip(objects, entry["texture_hashes"]):
            for kind, ext, _ in texList:
                if (ext.lower() != ".dds"):
                    continue
                src = os.path.join(assets, f"{name}{comp}{obj}{kind}{ext}")
                if (os.path.exists(src)):
                    shutil.copy2(src, os.path.join(modFolder, os.path.basename(src)))
                    textures.setdefault(obj, {})[kind] = os.path.basename(src)

        built[comp] = {"entry": entry, "objects": objects, "firstIndices": firstIndices,
                       "vertexCount": vertexCount, "strides": strides, "ibCounts": ibCounts, "textures": textures}

    # a component with no textures of its own draws another's (--textureFrom); the registers and the
    # fix call then follow the SOURCE object's layout, since those are the textures being bound
    def boundTextures(comp: str, obj: str):
        own = built[comp]["textures"].get(obj, {})
        if (own):
            return comp, obj, own
        srcComp, srcObj, layout = textureFrom.get(comp, (None, None, None))
        if (srcComp is None):
            return comp, obj, {}
        borrowed = dict(built[srcComp]["textures"].get(srcObj, {}))
        if (layout == "plain"):
            borrowed.pop("NormalMap", None)
        return srcComp, srcObj, borrowed

    faceDiffuse = None
    faceHash = None
    if (face):
        faceObj = list(face["object_classifications"])[0]
        src = os.path.join(assets, f"{name}Face{faceObj}Diffuse.dds")
        if (os.path.exists(src)):
            shutil.copy2(src, os.path.join(modFolder, os.path.basename(src)))
            faceDiffuse = os.path.basename(src)
            faceHash = next((h for kind, _, h in face["texture_hashes"][0] if kind == "Diffuse"), None)

    # ---- the .ini, in the shape GIMI generates ----
    L = [f"; {name}", "", "; Constants -------------------------", "", "; Overrides -------------------------", ""]
    unbound = []
    for comp, b in built.items():
        entry, objects = b["entry"], b["objects"]
        L += [f"[TextureOverride{name}{comp}Position]", f"hash = {entry['position_vb']}", f"vb0 = Resource{name}{comp}Position", ""]
        L += [f"[TextureOverride{name}{comp}Blend]", f"hash = {entry['blend_vb']}", f"vb1 = Resource{name}{comp}Blend", "handling = skip", f"draw = {b['vertexCount']},0", ""]
        L += [f"[TextureOverride{name}{comp}Texcoord]", f"hash = {entry['texcoord_vb']}", f"vb1 = Resource{name}{comp}Texcoord", ""]
        L += [f"[TextureOverride{name}{comp}VertexLimitRaise]", f"hash = {entry['draw_vb']}", ""]
        L += [f"[TextureOverride{name}{comp}IB]", f"hash = {entry['ib']}", "handling = skip", "drawindexed = auto", ""]
        for obj, first in zip(objects, b["firstIndices"]):
            L += [f"[TextureOverride{name}{comp}{obj}]", f"hash = {entry['ib']}", f"match_first_index = {first}", f"ib = Resource{name}{comp}{obj}IB"]
            srcComp, srcObj, bound = boundTextures(comp, obj)
            layout = NormalMapLayout if ("NormalMap" in bound) else PlainLayout
            for slot, kind in layout:
                if (kind in bound):
                    L.append(f"{slot} = Resource{name}{srcComp}{srcObj}{kind}")
            if (not bound):
                # no textures to bind, so no fix call either: ORFix / NNFix re-slot whatever is
                # bound, and the game's own textures are already in the slots the shader wants
                unbound.append(f"{comp}{obj}")
            elif (not args.noFix):
                L.append(f"run = {ORFix if ('NormalMap' in bound) else NNFix}")
            L.append("")
    if (faceDiffuse and faceHash):
        L += [f"[TextureOverride{name}FaceHeadDiffuse]", f"hash = {faceHash}", f"{args.faceRegister} = Resource{name}FaceHeadDiffuse", ""]
    L += ["", "; CommandList -----------------------", "", "; Resources -------------------------", ""]
    for comp, b in built.items():
        for part in BufOrder:
            L += [f"[Resource{name}{comp}{part}]", "type = Buffer", f"stride = {b['strides'][part]}", f"filename = {name}{comp}{part}.buf", ""]
    for comp, b in built.items():
        for obj in b["objects"]:
            L += [f"[Resource{name}{comp}{obj}IB]", "type = Buffer", "format = DXGI_FORMAT_R32_UINT", f"filename = {name}{comp}{obj}.ib", ""]
    for comp, b in built.items():
        for obj in b["objects"]:
            for kind, file in b["textures"].get(obj, {}).items():
                L += [f"[Resource{name}{comp}{obj}{kind}]", f"filename = {file}", ""]
    if (faceDiffuse):
        L += [f"[Resource{name}FaceHeadDiffuse]", f"filename = {faceDiffuse}", ""]
    L += ["", f"; the identity mod of {name}: the game's own model out of its asset folder ({os.path.basename(os.path.normpath(assets))}), built by identityMod.py", ""]
    with open(os.path.join(modFolder, f"{name}.ini"), "w", encoding = "utf-8", newline = "\r\n") as f:
        f.write("\n".join(L))

    for comp, b in built.items():
        print(f"{name}{comp}: {b['vertexCount']} vertices (texcoord stride {b['strides']['Texcoord']}); " + ", ".join(f"{obj} {b['ibCounts'][obj] // 3} triangles from index {first}" for obj, first in zip(b["objects"], b["firstIndices"])))
        drawn = []
        for obj in b["objects"]:
            srcComp, srcObj, bound = boundTextures(comp, obj)
            if (bound):
                borrowed = "" if ((srcComp, srcObj) == (comp, obj)) else f" <- {srcComp}{srcObj}"
                drawn.append(f"{obj} {'/'.join(bound)}{borrowed}")
        print("  textures: " + (", ".join(drawn) or "none"))
    if (faceDiffuse):
        print(f"  face: {faceDiffuse} on {args.faceRegister}")
    if (unbound):
        print(f"  NO textures bound (geometry only, no fix call): {', '.join(unbound)} -- point them at another component's with --textureFrom")
    print(f"  written to {modFolder}")


if (__name__ == "__main__"):
    main()
