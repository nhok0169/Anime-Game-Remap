#
# ===== wwmiIdentityMod =====
#
# Builds a Wuthering Waves character's IDENTITY mod: the game's own model, spelled out as a WWMI mod
# from the character's WWMI-Assets folder (WWMI-Assets' PlayerCharacterData/<Name>: Metadata.json, one
# 'Component N.fmt' / '.vb' / '.ib' triple per component, the .dds textures). It is the WuWa sibling
# of identityMod.py (GI): every component, every bone, every shape key and every texture of the real
# skin in one mod, so a remap prototype can be checked against the whole character instead of against
# whatever a downloaded mod happens to use.
#
#   python wwmiIdentityMod.py <asset folder> <mod folder>                e.g. PlayerCharacterData/Sanhua  WWMI/SanhuaIdentity
#   python wwmiIdentityMod.py <asset folder> <mod folder> --name SanhuaExorcist --noTextures
#
# Needs only numpy: the .fmt says where every element of the interleaved .vb sits, and the mod's
# separate buffers are those bytes copied out in the layout Metadata.json's 'export_format' declares
# (the same layout WWMI Tools writes from Blender, which is what every WWMI mod carries).
#
# ---- How a WWMI mod differs from a GIMI one, as far as this script is concerned (2026-09-19) ----
#
#   * ONE mesh, several draw slots. The game draws a character as 'Component N' index ranges of one
#     vertex buffer (hash = Metadata's vb0_hash), and the mod replaces all of them at once: one
#     Index.buf (R32_UINT, 3 per triangle, the components' local indices offset by their
#     'vertex_offset' and laid end to end), one Position / Vector / Color / TexCoord / Blend buffer
#     each holding every component's vertices in that order, and a [TextureOverrideComponentN]
#     per component matching on (vb0_hash, index_offset, index_count) that draws its own range.
#   * ONE merged skeleton. Each component's draw brings its own bone list in vs-cb4; WWMI's
#     SkeletonMerger copies component N's bones into a merged buffer at that component's
#     'vg_offset', so the merged skeleton is the components' bone lists concatenated. A bone shared
#     by several components (the head is in five of Sanhua's seven) therefore has several slots with
#     the same matrix, and Metadata's per-component 'vg_map' says which slot WWMI Tools chose for
#     each local bone (the first component's, so the model has one vertex group per bone). The
#     Blend.buf written here uses the vg_map -- the space real mods, Tools/VGRemapFinder and the
#     maintainer's drafts all use. A merged slot no vg_map reaches is a duplicate and no vertex
#     refers to it.
#   * Shape keys are SPARSE and per key. The dump carries them as per-vertex SHAPEKEY<k>
#     R16G16B16_FLOAT delta elements (only the face-side components have them: Sanhua's keys 19-85
#     on component 2, 0-13 on component 3, 14-18 on component 6). WWMI's ShapeKeyLoader wants three
#     buffers instead: ShapeKeyOffset.buf, 128 uint32s where entry k is where key k's vertices start
#     in the list (and every entry past the last key is the total); ShapeKeyVertexId.buf, the mod
#     vertex ids of every (key, vertex) with a non-zero delta, key by key; and
#     ShapeKeyVertexOffset.buf, six halfs per entry, the position delta and three zeros (real mods
#     leave the second triple zero). The proof this reading is right: Metadata's 'checksum' is the
#     sum of the first four offsets, and the offsets built here reproduce it (3175 for Sanhua, 2376
#     for her Exorcist skin), as they do 'dispatch_y' (the entry count in groups of 32) and the first
#     offsets of the maintainer's own Blender-exported Sanhua mod.
#   * Textures are overridden by hash, not bound to registers: a [TextureOverrideTexture<N>]
#     per .dds with 'this = ResourceTexture<N>', guarded by $object_detected. The identity ships every
#     'Components-... t=<hash>.dds' of the asset folder (a no-op in game, since it is the game's own
#     texture, but it gives a remap's texture edits something to act on), and no register lines at
#     all -- the slot layout of each draw is the game's, recorded in TextureUsage.json.
#   * The .ini is the WWMI BETA-2 template WWMI Tools 1.3.4 generates (required WWMI 0.91), copied
#     from a real mod; only the numbers, the hashes and the texture list are this character's.
#

import argparse
import json
import os
import re
import shutil
import sys

import numpy as np

OnWindows = (sys.platform == "win32")


def winToPosix(path: str) -> str:
    if (OnWindows or (len(path) < 2) or (path[1] != ":") or (not path[0].isalpha())):
        return path
    return "/mnt/" + path[0].lower() + path[2:].replace("\\", "/")


# the DXGI formats the .fmt may declare for the elements read here: numpy dtype and channel count
Formats = {
    "R32G32B32A32_FLOAT": ("<f4", 4), "R32G32B32_FLOAT": ("<f4", 3), "R32G32_FLOAT": ("<f4", 2), "R32_FLOAT": ("<f4", 1),
    "R16G16B16A16_FLOAT": ("<f2", 4), "R16G16B16_FLOAT": ("<f2", 3), "R16G16_FLOAT": ("<f2", 2), "R16_FLOAT": ("<f2", 1),
    "R8G8B8A8_UINT": ("u1", 4), "R8G8B8A8_UNORM": ("u1", 4), "R8G8B8A8_SNORM": ("i1", 4), "R8G8B8_SNORM": ("i1", 3), "R8_SNORM": ("i1", 1), "R8_UINT": ("u1", 1),
    "R16G16B16A16_UINT": ("<u2", 4), "R16G16B16A16_UNORM": ("<u2", 4), "R16G16_UNORM": ("<u2", 2), "R16_UINT": ("<u2", 1),
    "R32G32B32A32_UINT": ("<u4", 4), "R32_UINT": ("<u4", 1),
}

# the export buffers a WWMI mod is made of, in the order Metadata's export_format lists them, and the
# file / resource each becomes. Index is built from the .ib files, the ShapeKey* three from the
# SHAPEKEY elements; the rest are byte copies of vb elements
BufferFiles = {"Index": "Index.buf", "Position": "Position.buf", "Blend": "Blend.buf", "Vector": "Vector.buf", "Color": "Color.buf", "TexCoord": "TexCoord.buf",
               "ShapeKeyOffset": "ShapeKeyOffset.buf", "ShapeKeyVertexId": "ShapeKeyVertexId.buf", "ShapeKeyVertexOffset": "ShapeKeyVertexOffset.buf"}
ShapeKeySlots = 128
TexturePattern = re.compile(r"^Components-[0-9-]+ t=(?P<hash>[0-9a-fA-F]{8})\.dds$")


def formatOf(name: str):
    key = name.strip()
    if (key.upper().startswith("DXGI_FORMAT_")):
        key = key[len("DXGI_FORMAT_"):]
    try:
        return Formats[key.upper()]
    except KeyError:
        raise SystemExit(f"the format '{name}' is not one this script decodes (it knows: {', '.join(sorted(Formats))})") from None


def readFmt(path: str):
    """The .fmt's top-level keys (stride, topology, format) and its elements, in order"""
    header = {}
    elements = []
    current = None
    with open(path, "r", encoding = "utf-8") as f:
        for line in f:
            line = line.rstrip("\r\n")
            if (not line.strip()):
                continue
            if (line.startswith("element[")):
                current = {}
                elements.append(current)
                continue
            key, separator, value = line.partition(":")
            if (not separator):
                continue
            if (line[0] in " \t" and current is not None):
                current[key.strip()] = value.strip()
            else:
                header[key.strip()] = value.strip()
    return header, elements


class Component:
    """One 'Component N' of the asset folder: its interleaved vertex rows and its index buffer"""

    def __init__(self, folder: str, index: int, entry: dict):
        self.index = index
        self.entry = entry
        base = os.path.join(folder, f"Component {index}")
        for ext in (".fmt", ".vb", ".ib"):
            if (not os.path.isfile(base + ext)):
                raise SystemExit(f"'{base + ext}' is missing for component {index} of Metadata.json")
        self.header, self.elements = readFmt(base + ".fmt")
        self.stride = int(self.header.get("stride", "0"))
        raw = np.fromfile(base + ".vb", dtype = np.uint8)
        if (self.stride <= 0 or raw.size % self.stride != 0):
            raise SystemExit(f"'Component {index}.vb' is {raw.size} bytes, not a whole number of {self.stride}-byte vertices")
        self.rows = raw.reshape(-1, self.stride)
        self.vertexCount = self.rows.shape[0]
        if (int(entry.get("vertex_count", self.vertexCount)) != self.vertexCount):
            raise SystemExit(f"'Component {index}.vb' holds {self.vertexCount} vertices but Metadata.json declares {entry.get('vertex_count')}")
        ibDtype, _ = formatOf(self.header.get("format", "R16_UINT"))
        self.indices = np.fromfile(base + ".ib", dtype = ibDtype).astype(np.uint32)
        if (int(entry.get("index_count", self.indices.size)) != self.indices.size):
            raise SystemExit(f"'Component {index}.ib' holds {self.indices.size} indices but Metadata.json declares {entry.get('index_count')}")
        if (self.indices.size and int(self.indices.max()) >= self.vertexCount):
            raise SystemExit(f"'Component {index}.ib' references vertex {int(self.indices.max())} of {self.vertexCount}")
        self.vertexOffset = int(entry["vertex_offset"])
        self.indexOffset = int(entry["index_offset"])

    def element(self, name: str, semanticIndex: int = 0):
        for candidate in self.elements:
            if (candidate.get("SemanticName", "").upper() == name.upper() and int(candidate.get("SemanticIndex", "0")) == semanticIndex):
                return candidate
        return None

    def elementBytes(self, name: str, semanticIndex: int, width: int) -> np.ndarray:
        """The first `width` bytes of an element, per vertex"""
        element = self.element(name, semanticIndex)
        if (element is None):
            raise SystemExit(f"Component {self.index}: the .fmt declares no {name}{semanticIndex} element")
        dtype, count = formatOf(element.get("Format", ""))
        available = np.dtype(dtype).itemsize * count
        if (width > available):
            raise SystemExit(f"Component {self.index}: {name}{semanticIndex} is {available} bytes in the .fmt, {width} wanted")
        offset = int(element.get("AlignedByteOffset", "0"))
        return self.rows[:, offset:offset + width]

    def shapeKeys(self):
        """{key: (localVertexIds, deltas)} for every SHAPEKEY element with a non-zero delta somewhere"""
        result = {}
        for element in self.elements:
            if (element.get("SemanticName", "").upper() != "SHAPEKEY"):
                continue
            key = int(element.get("SemanticIndex", "0"))
            dtype, count = formatOf(element.get("Format", ""))
            if (count != 3):
                raise SystemExit(f"Component {self.index}: SHAPEKEY{key} is {element.get('Format')}, expected three channels")
            offset = int(element.get("AlignedByteOffset", "0"))
            width = np.dtype(dtype).itemsize * 3
            deltas = np.ascontiguousarray(self.rows[:, offset:offset + width]).view(dtype).reshape(-1, 3)
            used = np.nonzero((deltas != 0).any(axis = 1))[0]
            if (used.size):
                result[key] = (used, deltas[used].astype("<f2"))
        return result


def exportSemantics(exportFormat: dict, bufferName: str):
    entry = exportFormat.get(bufferName)
    if (entry is None):
        raise SystemExit(f"Metadata.json's export_format has no '{bufferName}' buffer")
    return [(s["name"], int(s.get("index", 0)), s["format"], int(s["stride"])) for s in entry["semantics"]]


def buildVertexBuffer(components, semantics) -> bytes:
    """One export buffer: for every component's vertices, the listed semantics' bytes side by side"""
    parts = []
    for component in components:
        columns = []
        for name, semanticIndex, fmt, stride in semantics:
            if (name.upper() == "BITANGENTSIGN" and component.element(name, semanticIndex) is None):
                # WWMI Tools splits the dump's four-byte NORMAL into a three-byte normal and the
                #   bitangent sign; the sign is the NORMAL element's fourth byte
                normal = component.elementBytes("NORMAL", 0, 4)
                columns.append(normal[:, 3:4])
                continue
            columns.append(component.elementBytes(name, semanticIndex, stride))
        parts.append(np.ascontiguousarray(np.concatenate(columns, axis = 1)))
    return b"".join(part.tobytes() for part in parts)


def buildBlend(components, semantics, useVgMap: bool) -> bytes:
    """The Blend buffer, the bone indices sent through each component's vg_map"""
    parts = []
    for component in components:
        columns = []
        for name, semanticIndex, fmt, stride in semantics:
            data = np.ascontiguousarray(component.elementBytes(name, semanticIndex, stride)).copy()
            if (name.upper() == "BLENDINDICES"):
                vgMap = component.entry.get("vg_map") or {}
                if (useVgMap and vgMap):
                    table = np.array([int(vgMap[str(local)]) for local in range(len(vgMap))], dtype = np.int64)
                    if (int(data.max()) >= len(table)):
                        raise SystemExit(f"Component {component.index} uses bone {int(data.max())} but its vg_map has {len(table)} entries")
                    mapped = table[data.astype(np.int64)]
                else:
                    mapped = data.astype(np.int64) + int(component.entry.get("vg_offset", 0))
                if (int(mapped.max()) > 255):
                    raise SystemExit(f"Component {component.index}: a merged bone index of {int(mapped.max())} does not fit the 8-bit Blend buffer")
                data = mapped.astype(np.uint8)
            columns.append(data)
        parts.append(np.ascontiguousarray(np.concatenate(columns, axis = 1)))
    return b"".join(part.tobytes() for part in parts)


def buildIndex(components) -> np.ndarray:
    return np.concatenate([component.indices + np.uint32(component.vertexOffset) for component in components]).astype("<u4")


def buildShapeKeys(components):
    """(offsets, vertexIds, deltas, perKeyCounts): the three shape key buffers' contents"""
    perKey = {}
    for component in components:
        for key, (used, deltas) in component.shapeKeys().items():
            perKey.setdefault(key, []).append((used.astype(np.uint32) + np.uint32(component.vertexOffset), deltas))
    if (perKey and max(perKey) >= ShapeKeySlots - 1):
        raise SystemExit(f"shape key {max(perKey)} does not fit WWMI's {ShapeKeySlots} slots")

    ids = []
    deltas = []
    counts = [0] * ShapeKeySlots
    for key in sorted(perKey):
        keyIds = np.concatenate([u for u, _ in perKey[key]])
        keyDeltas = np.concatenate([d for _, d in perKey[key]])
        order = np.argsort(keyIds, kind = "stable")
        ids.append(keyIds[order])
        deltas.append(keyDeltas[order])
        counts[key] = int(keyIds.size)

    total = sum(counts)
    offsets = np.zeros(ShapeKeySlots, dtype = "<u4")
    running = 0
    for key in range(ShapeKeySlots):
        offsets[key] = running
        running += counts[key]
    vertexIds = np.concatenate(ids).astype("<u4") if (ids) else np.zeros(0, dtype = "<u4")
    six = np.zeros((total, 6), dtype = "<f2")
    if (total):
        six[:, :3] = np.concatenate(deltas)
    return offsets, vertexIds, six, counts


def iniText(name: str, author: str, metadata: dict, components, vertexCount: int, indexCount: int, shapeKeyCount: int, textures) -> str:
    sk = metadata.get("shapekeys") or {}
    L = []
    L += ["; WWMI BETA-2 INI", "", "; Mod State -------------------------", "", "[Constants]",
          "global $required_wwmi_version = 0.91", f"global $object_guid = {indexCount}", f"global $mesh_vertex_count = {vertexCount}",
          f"global $shapekey_vertex_count = {shapeKeyCount}", "global $mod_id = -1000", "global $state_id = 0", "global $mod_enabled = 0", "global $object_detected = 0", ""]
    L += ["[Present]", "if $object_detected", "    if $mod_enabled", "        post $object_detected = 0", "        run = CommandListUpdateMergedSkeleton", "    else",
          "        if $mod_id == -1000", "            run = CommandListRegisterMod", "        endif", "    endif", "endif", ""]
    L += ["[CommandListRegisterMod]", "$\\WWMIv1\\required_wwmi_version = $required_wwmi_version", "$\\WWMIv1\\object_guid = $object_guid",
          "Resource\\WWMIv1\\ModName = ref ResourceModName", "Resource\\WWMIv1\\ModAuthor = ref ResourceModAuthor", "Resource\\WWMIv1\\ModDesc = ref ResourceModDesc",
          "Resource\\WWMIv1\\ModLink = ref ResourceModLink", "Resource\\WWMIv1\\ModLogo = ref ResourceModLogo", "run = CommandList\\WWMIv1\\RegisterMod",
          "$mod_id = $\\WWMIv1\\mod_id", "if $mod_id >= 0", "    $mod_enabled = 1", "endif", ""]
    L += ["[CommandListUpdateMergedSkeleton]", "if $state_id", "    $state_id = 0", "else", "    $state_id = 1", "endif",
          "ResourceMergedSkeleton = copy ResourceMergedSkeletonRW", "ResourceExtraMergedSkeleton = copy ResourceExtraMergedSkeletonRW", ""]
    L += ["; Resources: Mod Info -------------------------", "", "[ResourceModName]", "type = Buffer", f'data = "{name} Identity"', "",
          "[ResourceModAuthor]", "type = Buffer", f'data = "{author}"', "", "[ResourceModDesc]", "type = Buffer",
          f'data = "The identity mod of {name}: the game\'s own model out of WWMI-Assets, built by wwmiIdentityMod.py"', "",
          "[ResourceModLink]", "; type = Buffer", '; data = "Empty Mod Link"', "", "[ResourceModLogo]", "; filename = Textures/Logo.dds", ""]
    L += ["; Shading: Draw Call Stacks Processing -------------------------", "", "[TextureOverrideMarkBoneDataCB]", f"hash = {metadata['cb4_hash']}",
          "match_priority = 0", "filter_index = 3381.7777", ""]
    L += ["[CommandListMergeSkeleton]", "$\\WWMIv1\\custom_mesh_scale = 1.00", "cs-cb8 = ref vs-cb4", "cs-u6 = ResourceMergedSkeletonRW",
          "run = CustomShader\\WWMIv1\\SkeletonMerger", "cs-cb8 = ref vs-cb3", "cs-u6 = ResourceExtraMergedSkeletonRW", "run = CustomShader\\WWMIv1\\SkeletonMerger", ""]
    L += ["[CommandListTriggerResourceOverrides]"] + [f"CheckTextureOverride = ps-t{i}" for i in range(8)] + ["CheckTextureOverride = vs-cb3", "CheckTextureOverride = vs-cb4", ""]
    L += ["[CommandListOverrideSharedResources]", "ResourceBypassVB0 = ref vb0", "ib = ResourceIndexBuffer", "vb0 = ResourcePositionBuffer", "vb1 = ResourceVectorBuffer",
          "vb2 = ResourceTexcoordBuffer", "vb3 = ResourceColorBuffer", "vb4 = ResourceBlendBuffer", "if vs-cb3 == 3381.7777", "    vs-cb3 = ResourceExtraMergedSkeleton", "endif",
          "if vs-cb4 == 3381.7777", "    vs-cb4 = ResourceMergedSkeleton", "endif", ""]
    L += ["[CommandListCleanupSharedResources]", "vb0 = ref ResourceBypassVB0", ""]
    for component in components:
        i = component.index
        L += [f"[TextureOverrideComponent{i}]", f"hash = {metadata['vb0_hash']}", f"match_first_index = {component.indexOffset}", f"match_index_count = {component.indices.size}",
              "$object_detected = 1", "if $mod_enabled", f"    local $state_id_{i}", f"    if $state_id_{i} != $state_id", f"        $state_id_{i} = $state_id",
              f"        $\\WWMIv1\\vg_offset = {int(component.entry['vg_offset'])}", f"        $\\WWMIv1\\vg_count = {int(component.entry['vg_count'])}",
              "        run = CommandListMergeSkeleton", "    endif", "    if ResourceMergedSkeleton !== null", "        handling = skip",
              "        run = CommandListTriggerResourceOverrides", "        run = CommandListOverrideSharedResources", f"        ; Draw Component {i}",
              f"        drawindexed = {component.indices.size}, {component.indexOffset}, 0", "        run = CommandListCleanupSharedResources", "    endif", "endif", ""]
    L += ["; Shading: Textures -------------------------", ""]
    for n, (fileName, textureHash) in enumerate(textures):
        L += [f"[ResourceTexture{n}]", f"filename = Textures/{fileName}", "", f"[TextureOverrideTexture{n}]", f"hash = {textureHash}", "match_priority = 0",
              "if $object_detected", f"    this = ResourceTexture{n}", "endif", ""]
    L += ["; Skinning: Shape Keys Override -------------------------", "", "[TextureOverrideShapeKeyOffsets]", f"hash = {sk.get('offsets_hash', '')}", "match_priority = 0",
          "override_byte_stride = 24", "override_vertex_count = $mesh_vertex_count", "", "[TextureOverrideShapeKeyScale]", f"hash = {sk.get('scale_hash', '')}", "match_priority = 0",
          "override_byte_stride = 4", "override_vertex_count = $mesh_vertex_count", ""]
    L += ["[CommandListSetupShapeKeys]", f"$\\WWMIv1\\shapekey_checksum = {sk.get('checksum', 0)}", "cs-t33 = ResourceShapeKeyOffsetBuffer", "cs-u5 = ResourceCustomShapeKeyValuesRW",
          "cs-u6 = ResourceShapeKeyCBRW", "run = CustomShader\\WWMIv1\\ShapeKeyOverrider", ""]
    L += ["[CommandListLoadShapeKeys]", "$\\WWMIv1\\shapekey_vertex_count = $shapekey_vertex_count", "cs-t0 = ResourceShapeKeyVertexIdBuffer", "cs-t1 = ResourceShapeKeyVertexOffsetBuffer",
          "cs-u6 = ResourceShapeKeyCBRW", "run = CustomShader\\WWMIv1\\ShapeKeyLoader", ""]
    L += ["[TextureOverrideShapeKeyLoaderCallback]", f"hash = {sk.get('offsets_hash', '')}", "match_priority = 0", "if $mod_enabled",
          "    if cs == 3381.3333 && ResourceMergedSkeleton !== null", "        handling = skip", "        run = CommandListSetupShapeKeys", "        run = CommandListLoadShapeKeys",
          "    endif", "endif", ""]
    L += ["[CommandListMultiplyShapeKeys]", "$\\WWMIv1\\custom_vertex_count = $mesh_vertex_count", "run = CustomShader\\WWMIv1\\ShapeKeyMultiplier", ""]
    L += ["[TextureOverrideShapeKeyMultiplierCallback]", f"hash = {sk.get('offsets_hash', '')}", "match_priority = 0", "if $mod_enabled",
          "    if cs == 3381.4444 && ResourceMergedSkeleton !== null", "        handling = skip", "        run = CommandListMultiplyShapeKeys", "    endif", "endif", ""]
    L += ["; Resources: Shape Keys Override -------------------------", "", "[ResourceShapeKeyCBRW]", "type = RWBuffer", "format = R32G32B32A32_UINT", "array = 66", "",
          "[ResourceCustomShapeKeyValuesRW]", "type = RWBuffer", "format = R32G32B32A32_FLOAT", "array = 32", ""]
    L += ["; Resources: Skeleton Override -------------------------", "", "[ResourceMergedSkeleton]", "", "[ResourceMergedSkeletonRW]", "type = RWBuffer",
          "format = R32G32B32A32_FLOAT", "array = 768", "", "[ResourceExtraMergedSkeleton]", "", "[ResourceExtraMergedSkeletonRW]", "type = RWBuffer",
          "format = R32G32B32A32_FLOAT", "array = 768", ""]
    L += ["; Resources: Buffers -------------------------", "", "[ResourceBypassVB0]", ""]
    resources = [("ResourceIndexBuffer", "DXGI_FORMAT_R32_UINT", 12, "Index"), ("ResourcePositionBuffer", "DXGI_FORMAT_R32G32B32_FLOAT", 12, "Position"),
                 ("ResourceBlendBuffer", "DXGI_FORMAT_R8_UINT", 8, "Blend"), ("ResourceVectorBuffer", "DXGI_FORMAT_R8G8B8A8_SNORM", 8, "Vector"),
                 ("ResourceColorBuffer", "DXGI_FORMAT_R8G8B8A8_UNORM", 4, "Color"), ("ResourceTexCoordBuffer", "DXGI_FORMAT_R16G16_FLOAT", 16, "TexCoord"),
                 ("ResourceShapeKeyOffsetBuffer", "DXGI_FORMAT_R32G32B32A32_UINT", 16, "ShapeKeyOffset"), ("ResourceShapeKeyVertexIdBuffer", "DXGI_FORMAT_R32_UINT", 4, "ShapeKeyVertexId"),
                 ("ResourceShapeKeyVertexOffsetBuffer", "DXGI_FORMAT_R16_FLOAT", 2, "ShapeKeyVertexOffset")]
    for section, fmt, stride, bufferName in resources:
        L += [f"[{section}]", "type = Buffer", f"format = {fmt}", f"stride = {stride}", f"filename = Meshes/{BufferFiles[bufferName]}", ""]
    L += ["; Autogenerated -------------------------", "", f"; This mod.ini is the IDENTITY mod of {name}, generated by Anime Game Remap's Tools/Misc/Prototypes/wwmiIdentityMod.py from WWMI-Assets, "
          "in the shape WWMI Tools 1.3.4 generates (WWMI v0.9.1+)", ""]
    return "\n".join(L)


def main():
    parser = argparse.ArgumentParser(description = "a Wuthering Waves character's identity mod from its WWMI-Assets folder")
    parser.add_argument("assets", help = "the asset folder (Metadata.json, Component N.fmt/.vb/.ib, *.dds)")
    parser.add_argument("mod", help = "the mod folder to write (created; gets mod.ini, Meshes/ and Textures/)")
    parser.add_argument("--name", default = None, help = "the character name used in the mod's name (default: the asset folder's name)")
    parser.add_argument("--author", default = "Anime Game Remap", help = "the mod author WWMI shows")
    parser.add_argument("--noTextures", action = "store_true", help = "leave the textures out (geometry, skeleton and shape keys only)")
    parser.add_argument("--rawBones", action = "store_true", help = "write bone indices as vg_offset + local instead of through the vg_map (the merged skeleton's duplicate slots; every real mod uses the vg_map)")
    args = parser.parse_args()
    assets = winToPosix(args.assets)
    modFolder = winToPosix(args.mod)
    name = args.name or os.path.basename(os.path.normpath(assets))

    with open(os.path.join(assets, "Metadata.json"), "r", encoding = "utf-8") as f:
        metadata = json.load(f)
    exportFormat = metadata.get("export_format") or {}
    components = [Component(assets, i, entry) for i, entry in enumerate(metadata["components"])]
    if (not components):
        raise SystemExit(f"{name}: Metadata.json lists no components")

    offset = 0
    indexOffset = 0
    for component in components:
        if (component.vertexOffset != offset or component.indexOffset != indexOffset):
            raise SystemExit(f"Component {component.index}: Metadata's offsets ({component.vertexOffset}, {component.indexOffset}) do not follow the previous components ({offset}, {indexOffset})")
        offset += component.vertexCount
        indexOffset += component.indices.size
    vertexCount, indexCount = offset, indexOffset
    if (int(metadata.get("vertex_count", vertexCount)) != vertexCount or int(metadata.get("index_count", indexCount)) != indexCount):
        raise SystemExit(f"{name}: the components total {vertexCount} vertices / {indexCount} indices but Metadata.json says {metadata.get('vertex_count')} / {metadata.get('index_count')}")

    meshes = os.path.join(modFolder, "Meshes")
    os.makedirs(meshes, exist_ok = True)

    written = {}

    def write(bufferName: str, data: bytes):
        path = os.path.join(meshes, BufferFiles[bufferName])
        with open(path, "wb") as f:
            f.write(data)
        written[bufferName] = len(data)

    write("Index", buildIndex(components).tobytes())
    for bufferName in ("Position", "Vector", "Color", "TexCoord"):
        write(bufferName, buildVertexBuffer(components, exportSemantics(exportFormat, bufferName)))
    write("Blend", buildBlend(components, exportSemantics(exportFormat, "Blend"), useVgMap = not args.rawBones))

    offsets, vertexIds, deltas, counts = buildShapeKeys(components)
    write("ShapeKeyOffset", offsets.tobytes())
    write("ShapeKeyVertexId", vertexIds.tobytes())
    write("ShapeKeyVertexOffset", deltas.tobytes())
    shapeKeyCount = int(vertexIds.size)

    textures = []
    if (not args.noTextures):
        textureFolder = os.path.join(modFolder, "Textures")
        os.makedirs(textureFolder, exist_ok = True)
        for fileName in sorted(os.listdir(assets)):
            match = TexturePattern.match(fileName)
            if (match is None):
                continue
            shutil.copy2(os.path.join(assets, fileName), os.path.join(textureFolder, fileName))
            textures.append((fileName, match.group("hash").lower()))

    with open(os.path.join(modFolder, "mod.ini"), "w", encoding = "utf-8", newline = "\r\n") as f:
        f.write(iniText(name, args.author, metadata, components, vertexCount, indexCount, shapeKeyCount, textures))

    # ---- what was built, and the checks that say the reading is the game's ----
    sk = metadata.get("shapekeys") or {}
    checksum = int(offsets[:4].sum())
    dispatchY = -(-shapeKeyCount // 32)
    print(f"{name}: {len(components)} components, {vertexCount} vertices, {indexCount} indices ({indexCount // 3} triangles), hash {metadata['vb0_hash']}, skeleton cb4 {metadata['cb4_hash']}")
    for component in components:
        vgMap = component.entry.get("vg_map") or {}
        print(f"  Component {component.index}: {component.vertexCount} vertices from {component.vertexOffset}, {component.indices.size} indices from {component.indexOffset}, "
              f"{int(component.entry['vg_count'])} bones at merged slot {int(component.entry['vg_offset'])}"
              + (f" ({sum(1 for k, v in vgMap.items() if int(v) != int(component.entry['vg_offset']) + int(k))} of them another component's)" if (vgMap) else ""))
    print("  buffers: " + ", ".join(f"{BufferFiles[b]} {written[b]} B" for b in BufferFiles))
    keys = [k for k, c in enumerate(counts) if c]
    print(f"  shape keys: {len(keys)} keys ({keys[0]}-{keys[-1]}), {shapeKeyCount} (key, vertex) entries; first offsets {offsets[:4].tolist()} sum {checksum} "
          f"vs Metadata checksum {sk.get('checksum')} ({'OK' if checksum == sk.get('checksum') else 'MISMATCH'}); dispatch_y {dispatchY} vs {sk.get('dispatch_y')} "
          f"({'OK' if dispatchY == sk.get('dispatch_y') else 'MISMATCH'}); entries vs Metadata {sk.get('vertex_count')}")
    print(f"  textures: {len(textures)}" + ("" if textures else " (none)"))
    print(f"  written to {modFolder}")


if (__name__ == "__main__"):
    main()
