# ===== bandHueStats =====
#
# For a FIXED mod: on each light map band a diffuse-gated band move reads, a histogram of the diffuse's (red - blue) --
# all texels, and the ones the fix MOVED (the fixed light map's alpha differs from the mod's). The question it answers
# is "which texels does my hue gate take, and which does it leave", before choosing a gate rather than after an
# in-game round: Aether10's grey-white mane sat at red - blue 0..3, a gate of "blue >= red + 4" left all 19 000 of
# those texels on the skin's warm band, and the mane shaded peach (Creating Remaps' "AETHER <-> AETHERHEAVEN" point 3).
#
#   py -3 bandHueStats.py <fixed mod folder> ... [--object Head] [--bands 118-135,150-175,240-255]
#                         [--diffuse <name> --fixed <name>]
#
# Per object it pairs <...><Object>LightMap.dds with <...><Object>LightMapRemapTex.dds and <...><Object>Diffuse.dds
# in each folder. For a toggled diffuse (a hair colour key) pass its FILE name as --diffuse and its own edited light map
# as --fixed: with GIMIComponentFixerConfig::lightMapPerDiffuse each diffuse gets a copy of its own
# (<Component><Object>LightMap<n>RemapTex...dds), and against the first diffuse's copy it would read as "0 moved".
# The three must be the same size; a set that is not is reported and skipped.
import argparse
import os
import sys

import numpy as np

Repo = os.environ.get("AG_REMAP_REPO") or os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", ".."))
ApiSrc = os.path.join(Repo, "Anime Game Remap (for all users)", "api", "src", "py")
sys.path.insert(0, ApiSrc)
if (hasattr(os, "add_dll_directory")):
    os.add_dll_directory(os.path.join(ApiSrc, "FixRaidenBoss2"))

import FixRaidenBoss2 as FRB     # noqa: E402

Edges = [-999, -40, -20, -10, -4, 0, 4, 8, 12, 20, 40, 999]


def pixels(path: str) -> np.ndarray:
    texture = FRB.TextureFile(path)
    texture.open()
    return np.frombuffer(bytes(texture.getPixels()), np.uint8).reshape(-1, 4).astype(int)


def findOne(folder: str, suffix: str):
    found = [f for f in os.listdir(folder) if f.endswith(suffix)]
    return os.path.join(folder, found[0]) if found else None


def main():
    parser = argparse.ArgumentParser(description = "red - blue of the diffuse on each light map band, all vs moved")
    parser.add_argument("mods", nargs = "+")
    parser.add_argument("--object", action = "append", help = "object name in the file names (default Head and Body)")
    parser.add_argument("--bands", default = "118-135,150-175,240-255", help = "light map alpha ranges, lo-hi,...")
    parser.add_argument("--diffuse", help = "the diffuse's FILE name to read instead of <...><Object>Diffuse.dds")
    parser.add_argument("--fixed", help = "the edited light map's FILE name instead of <...><Object>LightMapRemapTex.dds")
    args = parser.parse_args()

    bands = [tuple(int(v) for v in band.split("-")) for band in args.bands.split(",")]
    labels = [f"[{Edges[i]},{Edges[i + 1]})" for i in range(1, len(Edges) - 2)]
    print("red - blue bins:", " ".join(labels), "(plus the two open ends)")

    for mod in args.mods:
        for obj in (args.object or ["Head", "Body"]):
            lightMap = findOne(mod, f"{obj}LightMap.dds")
            fixed = os.path.join(mod, args.fixed) if args.fixed else findOne(mod, f"{obj}LightMapRemapTex.dds")
            diffuse = os.path.join(mod, args.diffuse) if args.diffuse else findOne(mod, f"{obj}Diffuse.dds")
            if (not (lightMap and fixed and diffuse and os.path.isfile(diffuse) and os.path.isfile(fixed))):
                continue

            L, R, D = pixels(lightMap), pixels(fixed), pixels(diffuse)
            if (not (len(L) == len(R) == len(D))):
                print(f"== {os.path.basename(mod)} {obj}: sizes differ ({len(L)} / {len(R)} / {len(D)} texels) -- skipped")
                continue

            redBlue = D[:, 0] - D[:, 2]
            print(f"== {os.path.basename(mod)} {obj}  (diffuse {os.path.basename(diffuse)})")
            for lo, hi in bands:
                onBand = (L[:, 3] >= lo) & (L[:, 3] <= hi) & (D[:, 3] > 0)
                if (not onBand.any()):
                    continue
                moved = onBand & (R[:, 3] != L[:, 3])
                kept = onBand & ~moved
                print(f"  band {lo}-{hi}: {onBand.sum()} texels, {moved.sum()} moved")
                print("     all  ", np.histogram(redBlue[onBand], Edges)[0].tolist())
                print("     moved", np.histogram(redBlue[moved], Edges)[0].tolist())
                if (kept.any()):
                    print("     kept: mean RGB", D[kept, :3].mean(0).round().astype(int).tolist())


if __name__ == "__main__":
    main()
