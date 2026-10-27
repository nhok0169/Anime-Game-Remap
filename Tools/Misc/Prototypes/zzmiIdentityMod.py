#
# ===== zzmiIdentityMod =====
#
# Writes a Zenless Zone Zero character's IDENTITY MOD: the game's own model, every component, object and
# texture, as a ZZMI mod -- pipeline step 4 (Creating Remaps' "THE MAINTAINER'S REMAP PIPELINE"). It is
# the first mod every prototype is tested on, and the ground truth for the target's layout.
#
# The source is the character's download folder (Tools/Misc/Prototypes/zzmiDownloadFolder.py), which
# already holds the mod-facing files: <Name><Comp>Position / Texcoord / Blend.buf, one R32_UINT
# <Name><Comp><Obj>.ib per object and <Name><Comp><Obj><Role>.dds per texture, plus <Name>Hash.json.
# The .ini is the shape ZZMI's own exporter writes (a Blend override that redraws the pre-skin pass on
# DRAW_TYPE 1, a Texcoord override, a VertexLimitRaise on the post-skin buffer's hash, a skipped IB
# section and one section per object binding its textures through CommandList\ZZMI\SetTextures).
#
# The post-skin buffer's stride is not in the download folder, so it is read off the frame dump the
# folder was built from (Miyabi's Body draws at 96 bytes, the skin's at 40).
#
#   py -3 zzmiIdentityMod.py <download folder> <frame dump> <mod folder> --name Miyabi [--skip Face]
#

import argparse
import json
import os
import re
import shutil


def drawStrides(dump: str, hashes):
    """{post-skin vb0 hash: stride}, from the first .txt of each hash in the dump"""
    wanted, found = set(hashes), {}
    for name in sorted(os.listdir(dump)):
        m = re.match(r"\d{6}-vb0=([0-9a-f]{8})-.*\.txt$", name)
        if (not m or m.group(1) not in wanted or m.group(1) in found):
            continue
        with open(os.path.join(dump, name), encoding = "utf-8", errors = "replace") as f:
            for line in f:
                if (line.startswith("stride:")):
                    found[m.group(1)] = int(line.split(":")[1])
                    break
        if (len(found) == len(wanted)):
            break
    missing = wanted - set(found)
    if (missing):
        raise SystemExit(f"the dump has no draw binding vb0 = {sorted(missing)}")
    return found


def build(downloads: str, dump: str, out: str, name: str, skip):
    entries = [e for e in json.load(open(os.path.join(downloads, f"{name}Hash.json"))) if e["component_name"] not in skip]
    strides = drawStrides(dump, [e["draw_vb"] for e in entries])
    os.makedirs(out, exist_ok = True)

    overrides, objects, resources = [], [], []
    for e in entries:
        comp = name + e["component_name"]
        count = e["vertex_count"]
        overrides += [
            f"[TextureOverride{comp}Blend]", f"hash = {e['blend_vb']}", "handling = skip", f"vb2 = Resource{comp}Blend",
            "if DRAW_TYPE == 1", f"\tvb0 = Resource{comp}Position", f"\tdraw = {count}, 0", "endif", "$active = 1", "",
            f"[TextureOverride{comp}Texcoord]", f"hash = {e['texcoord_vb']}", f"vb1 = Resource{comp}Texcoord", "",
            f"[TextureOverride{comp}VertexLimitRaise]", f"hash = {e['draw_vb']}", f"override_vertex_count = {count}",
            f"override_byte_stride = {strides[e['draw_vb']]}", ""]
        objects += [f"[TextureOverride{comp}IB]", f"hash = {e['ib']}", "handling = skip", ""]
        for part, (stride, kind) in {"Position": (e["position_stride"], "Position"), "Blend": (e["blend_stride"], "Blend"),
                                     "Texcoord": (e["texcoord_stride"], "Texcoord")}.items():
            resources += [f"[Resource{comp}{part}]", "type = Buffer", f"stride = {stride}", f"filename = {comp}{part}.buf", ""]
            shutil.copyfile(os.path.join(downloads, f"{comp}{part}.buf"), os.path.join(out, f"{comp}{part}.buf"))

        for obj, first, n, textures in zip(e["object_classifications"], e["object_indexes"], e["object_counts"], e["texture_hashes"]):
            objects += [f"[TextureOverride{comp}{obj}]", f"hash = {e['ib']}", f"match_first_index = {first}", f"ib = Resource{comp}{obj}IB"]
            for role, ext, _ in textures:
                objects.append(f"Resource\\ZZMI\\{role} = ref Resource{comp}{obj}{role}")
                resources += [f"[Resource{comp}{obj}{role}]", f"filename = {comp}{obj}{role}{ext}", ""]
                shutil.copyfile(os.path.join(downloads, f"{comp}{obj}{role}{ext}"), os.path.join(out, f"{comp}{obj}{role}{ext}"))
            if (textures):
                objects.append("run = CommandList\\ZZMI\\SetTextures")
            objects += [f"drawindexed = {n}, 0, 0", ""]
            resources += [f"[Resource{comp}{obj}IB]", "type = Buffer", "format = DXGI_FORMAT_R32_UINT", f"filename = {comp}{obj}.ib", ""]
            shutil.copyfile(os.path.join(downloads, f"{comp}{obj}.ib"), os.path.join(out, f"{comp}{obj}.ib"))

    ini = [f"; {name} -- IDENTITY MOD: the game's own model as a mod (Tools/Misc/Prototypes/zzmiIdentityMod.py)", "",
           "; Constants -------------------------", "", "[Constants]", "global $active = 0", "",
           "[Present]", "post $active = 0", "",
           "; Overrides -------------------------", ""] + overrides + objects + ["; Resources -------------------------", ""] + resources
    with open(os.path.join(out, f"{name}.ini"), "w", encoding = "utf-8", newline = "\r\n") as f:
        f.write("\n".join(ini))
    print(f"wrote {out}: {len(entries)} components, {sum(len(e['object_classifications']) for e in entries)} objects")


def main():
    parser = argparse.ArgumentParser(description = "a ZZZ character's identity mod from its download folder")
    parser.add_argument("downloads")
    parser.add_argument("dump")
    parser.add_argument("out")
    parser.add_argument("--name", required = True)
    parser.add_argument("--skip", nargs = "*", default = [], help = "components to leave out")
    args = parser.parse_args()
    build(args.downloads, args.dump, args.out, args.name, set(args.skip))


if __name__ == "__main__":
    main()
