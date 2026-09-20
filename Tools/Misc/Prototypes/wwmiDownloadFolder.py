#
# ===== wwmiDownloadFolder =====
#
# Builds a Wuthering Waves character's Data/Mod Downloads/WuWa/<Name>/<version> folder from its ASSET
# folder (WWMI-Assets' PlayerCharacterData/<Name>, or what wwmiExtractDump.py extracted from a frame
# dump for a character WWMI-Assets does not have). The layout is Data/Mod Downloads/WuWa/README.md's:
#
#   <Name>Index.buf, Position, Blend, Vector, Color, Texcoord, ShapeKeyOffset / VertexId / VertexOffset
#       -- the character's identity mod's Meshes/ buffers, byte for byte (wwmiIdentityMod.py builds them)
#   <Name>BlendRemapVertexVG / Forward / Reverse.buf -- only for a merged skeleton past 256 bones
#       (Chisa): WWMI's blend remap, the full 16-bit bone ids the 8-bit Blend.buf cannot hold
#   <Name>Texture<hash>.dds   -- every 'Components-... t=<hash>.dds' of the asset folder, copied
#   <Name>Metadata.json, <Name>TextureUsage.json -- the asset folder's manifests, copied
#
#   py -3.11 wwmiDownloadFolder.py <asset folder> <download folder> --name Chisa
#   py -3.11 wwmiDownloadFolder.py <asset folder> <download folder> --name Sanhua --check
#
# --check writes nothing: it builds into a temporary folder and compares every file against the
# download folder that is already there, which is how the pipeline is proved on a character whose
# folder is shipped before it is trusted on one whose folder is not (Creating Remaps' "Proving a NEW
# download folder").
#

import argparse
import filecmp
import os
import re
import shutil
import subprocess
import sys
import tempfile

Here = os.path.dirname(os.path.abspath(__file__))
IdentityMod = os.path.join(Here, "wwmiIdentityMod.py")

# identity mod Meshes/ file -> download folder suffix (the download folder spells Texcoord with a small c)
Buffers = {"Index.buf": "Index.buf", "Position.buf": "Position.buf", "Blend.buf": "Blend.buf", "Vector.buf": "Vector.buf", "Color.buf": "Color.buf",
           "TexCoord.buf": "Texcoord.buf", "ShapeKeyOffset.buf": "ShapeKeyOffset.buf", "ShapeKeyVertexId.buf": "ShapeKeyVertexId.buf",
           "ShapeKeyVertexOffset.buf": "ShapeKeyVertexOffset.buf"}
# only a character whose merged skeleton passes 256 bones has these (WWMI's blend remap; wwmiIdentityMod.py)
OptionalBuffers = {"BlendRemapVertexVG.buf": "BlendRemapVertexVG.buf", "BlendRemapForward.buf": "BlendRemapForward.buf", "BlendRemapReverse.buf": "BlendRemapReverse.buf"}
Manifests = ("Metadata.json", "TextureUsage.json")
TexturePattern = re.compile(r"^Components-[0-9-]+ t=(?P<hash>[0-9a-fA-F]{8})\.dds$")


def build(assets: str, out: str, name: str):
    os.makedirs(out, exist_ok = True)
    with tempfile.TemporaryDirectory() as tmp:
        r = subprocess.run([sys.executable, IdentityMod, assets, tmp, "--name", name, "--noTextures"], capture_output = True, text = True)
        print(r.stdout, end = "")
        if (r.returncode != 0):
            raise SystemExit(f"wwmiIdentityMod.py failed:\n{r.stderr}")
        for source, suffix in Buffers.items():
            shutil.copyfile(os.path.join(tmp, "Meshes", source), os.path.join(out, name + suffix))
        for source, suffix in OptionalBuffers.items():
            if (os.path.isfile(os.path.join(tmp, "Meshes", source))):
                shutil.copyfile(os.path.join(tmp, "Meshes", source), os.path.join(out, name + suffix))

    for manifest in Manifests:
        shutil.copyfile(os.path.join(assets, manifest), os.path.join(out, name + manifest))

    textures = 0
    for fileName in sorted(os.listdir(assets)):
        match = TexturePattern.match(fileName)
        if (match is not None):
            shutil.copyfile(os.path.join(assets, fileName), os.path.join(out, f"{name}Texture{match.group('hash').lower()}.dds"))
            textures += 1
    return textures


def main():
    parser = argparse.ArgumentParser(description = "a WuWa character's Data/Mod Downloads folder from its asset folder")
    parser.add_argument("assets", help = "the asset folder (Metadata.json, TextureUsage.json, Component N.fmt/.vb/.ib, Components-... t=<hash>.dds)")
    parser.add_argument("out", help = "the download folder to write, e.g. 'Data/Mod Downloads/WuWa/Chisa/3_0'")
    parser.add_argument("--name", required = True, help = "the character's ModType name, the prefix of every file")
    parser.add_argument("--check", action = "store_true", help = "write nothing; build into a temporary folder and compare it with the existing 'out'")
    args = parser.parse_args()

    if (not args.check):
        textures = build(args.assets, args.out, args.name)
        print(f"wrote {len(Buffers)} buffers (+ blend remap buffers if any), {len(Manifests)} manifests and {textures} textures to {args.out}")
        return

    with tempfile.TemporaryDirectory() as tmp:
        build(args.assets, tmp, args.name)
        built, shipped = set(os.listdir(tmp)), set(os.listdir(args.out))
        same = [f for f in sorted(built & shipped) if filecmp.cmp(os.path.join(tmp, f), os.path.join(args.out, f), shallow = False)]
        differ = sorted((built & shipped) - set(same))
        print(f"{len(same)} identical, {len(differ)} differ, {len(built - shipped)} only built, {len(shipped - built)} only shipped")
        for label, files in (("differ", differ), ("only built", sorted(built - shipped)), ("only shipped", sorted(shipped - built))):
            for f in files:
                print(f"  {label}: {f}")
        if (differ or built != shipped):
            sys.exit(1)


if (__name__ == "__main__"):
    main()
