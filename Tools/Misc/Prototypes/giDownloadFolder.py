#
# ===== giDownloadFolder =====
#
# Builds a Genshin character's Data/Mod Downloads/GI/<Name>/<X_Y> folder from its ASSET folder
# (GI-Model-Importer-Assets' PlayerCharacterData/<Name>: hash.json, the *-vb0=*.txt / *-ib=*.txt
# dumps and the .dds textures). The conversion is identityMod.py's, minus the .ini (Creating Remaps'
# "Proving a NEW download folder"):
#
#   <Prefix><Comp>Position.buf / Blend.buf / Texcoord.buf   -- the vb0 dump split 40 / 32 / measured
#   <Prefix><Comp><Obj>.ib                                  -- each object's indices, as R32_UINT
#   <Prefix><Comp><Obj><Kind>.dds                           -- every texture hash.json lists, copied
#   <Prefix>FaceDiffuse.dds                                 -- the "Face" entry's diffuse
#
# <Comp> is "" for a classic one-mesh character, so one loop writes both shapes. A texture-only entry
# other than "Face" (Citlali's "NatlanFx") keeps its own <Comp><Obj><Kind> name.
#
# Only SKINNED components (a position_vb AND a blend_vb) get buffers. A 6.x skin can carry unskinned
# face meshes (CitlaliWhisperofStars' Face / Mouth / Eyebrows: no BLENDWEIGHTS in the dump, the
# eyebrows not even a TANGENT), which have no GIMI buffer layout; they are reported, not written.
#
#   py -3 giDownloadFolder.py <asset folder> <download folder> --name <Prefix>
#   py -3 giDownloadFolder.py <asset folder> <download folder> --name <Prefix> --check
#
# --check writes nothing: it builds into a temporary folder and compares every file with the folder
# that is already there -- how the pipeline is proved on shipped characters before it is trusted on a
# new one. Textures are compared too, so a shipped folder whose .dds came from an older asset commit
# shows up as a texture mismatch, not a buffer one.
#

import argparse
import filecmp
import glob
import json
import os
import shutil
import sys
import tempfile

Here = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, Here)
from identityMod import bufsFromDump, ibFromDump, winToPosix     # noqa: E402


def findOne(assets: str, suffix: str, required: bool = True):
    """The asset file ending in `suffix`: asset names are globbed, never composed (GaMing's face is
    GamingFaceHeadDiffuse.dds, RosariaCN's is RosariaFaceHeadDiffuse.dds)"""
    hits = sorted(glob.glob(os.path.join(glob.escape(assets), "*" + suffix)))
    # a longer component name can end in the same suffix (FooBodyDiffuse vs FooBangBodyDiffuse): take
    # the shortest, and refuse a tie
    hits.sort(key = lambda p: len(os.path.basename(p)))
    if (not hits):
        if (required):
            raise SystemExit(f"no asset file ends in {suffix!r}")
        return None
    if ((len(hits) > 1) and (len(os.path.basename(hits[0])) == len(os.path.basename(hits[1])))):
        raise SystemExit(f"several asset files end in {suffix!r}: {', '.join(os.path.basename(h) for h in hits)}")
    return hits[0]


def build(assets: str, out: str, name: str, report: bool = True):
    hashes = json.load(open(os.path.join(assets, "hash.json"), encoding = "utf-8"))
    os.makedirs(out, exist_ok = True)
    written = []
    notes = []

    def copyTexture(suffix: str, dst: str):
        src = findOne(assets, suffix, required = False)
        if (src is None):
            notes.append(f"missing texture: *{suffix} (hash.json lists it, the asset folder has no file)")
            return
        shutil.copyfile(src, os.path.join(out, dst))
        written.append(dst)

    for entry in hashes:
        comp = entry.get("component_name", "")
        objects = list(entry["object_classifications"])

        if (entry.get("position_vb") and entry.get("blend_vb")):
            vbPath = findOne(assets, f"{comp}{objects[0]}-vb0={entry['position_vb']}.txt")
            bufs, strides, vertexCount = bufsFromDump(vbPath)
            for part, data in bufs.items():
                dst = f"{name}{comp}{part}.buf"
                with open(os.path.join(out, dst), "wb") as f:
                    f.write(data)
                written.append(dst)
            for obj in objects:
                ib = ibFromDump(findOne(assets, f"{comp}{obj}-ib={entry['ib']}.txt"))
                if (ib.size and (ib.max() >= vertexCount)):
                    raise SystemExit(f"{comp}{obj}: index {ib.max()} beyond the {vertexCount} vertices")
                dst = f"{name}{comp}{obj}.ib"
                ib.tofile(os.path.join(out, dst))
                written.append(dst)
            if (report):
                print(f"{name}{comp}: {vertexCount} vertices, texcoord stride {strides['Texcoord']}, objects {', '.join(objects)}")
        elif (entry.get("position_vb")):
            missing = ["blend_vb"] + ([] if entry.get("root_vs") else ["root_vs"])
            notes.append(f"unskinned component {comp!r} (objects {', '.join(objects)}): no buffers written -- hash.json has no {', '.join(missing)}")

        for obj, texList in zip(objects, entry.get("texture_hashes", [])):
            if ((not texList) and entry.get("position_vb")):
                notes.append(f"component {comp!r} object {obj!r}: hash.json lists NO textures")
            for kind, ext, _ in texList:
                if (ext.lower() != ".dds"):
                    continue
                if (comp == "Face"):
                    copyTexture(f"Face{obj}{kind}{ext}", f"{name}Face{kind}{ext}")
                else:
                    copyTexture(f"{comp}{obj}{kind}{ext}", f"{name}{comp}{obj}{kind}{ext}")

    if (report):
        for note in notes:
            print("  " + note)
    return written, notes


def main():
    parser = argparse.ArgumentParser(description = "a GI character's Data/Mod Downloads folder from its asset folder")
    parser.add_argument("assets", help = "the asset folder (hash.json, *-vb0=*.txt, *-ib=*.txt, *.dds)")
    parser.add_argument("out", help = "the download folder, Data/Mod Downloads/GI/<Name>/<X_Y>")
    parser.add_argument("--name", required = True, help = "the file prefix (take it from the files already in that folder, not from the folder's name)")
    parser.add_argument("--check", action = "store_true", help = "write nothing; build into a temporary folder and compare it with the existing 'out'")
    args = parser.parse_args()
    assets = winToPosix(args.assets)
    out = winToPosix(args.out)

    if (not args.check):
        build(assets, out, args.name)
        return

    with tempfile.TemporaryDirectory() as tmp:
        written, _ = build(assets, tmp, args.name, report = False)
        shipped = sorted(f for f in os.listdir(out) if os.path.isfile(os.path.join(out, f)))
        same, differ = [], []
        for f in sorted(set(written)):
            if (f not in shipped):
                continue
            (same if filecmp.cmp(os.path.join(tmp, f), os.path.join(out, f), shallow = False) else differ).append(f)
        onlyBuilt = sorted(set(written) - set(shipped))
        onlyShipped = sorted(set(shipped) - set(written))
        print(f"{args.name}: {len(same)} identical, {len(differ)} differ, {len(onlyBuilt)} only built, {len(onlyShipped)} only shipped")
        for label, files in (("DIFFER", differ), ("only built", onlyBuilt), ("only shipped", onlyShipped)):
            for f in files:
                print(f"  {label}: {f}")
        if (differ or onlyBuilt or onlyShipped):
            sys.exit(1)


if (__name__ == "__main__"):
    main()
