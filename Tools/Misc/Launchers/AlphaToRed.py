"""
AlphaToRed.py -- make a texture's ALPHA channel visible as RED.

    py -3 AlphaToRed.py "Chisa16\\...\\Textures\\Components-5 t=0cc6f756.dds"
    py -3 AlphaToRed.py <texture> --suffix Alpha      # name it ...Alpha.dds instead of ...Copy.dds

Writes a new `.dds` beside the source, with `Copy` before the extension, in which every pixel is
`(x, 0, 0, 255)` where `x` is that pixel's ALPHA in the source. Green, blue and alpha are constant,
so the file reads as a pure map of the source's alpha.

WHAT IT IS FOR: alpha is invisible. Most viewers either ignore it or composite it away, so a texture
whose alpha carries real information -- an opacity mask, a blend weight, a packed profile -- looks
like ordinary art and its alpha is never inspected. Moving alpha into red makes it an ordinary
picture you can open, crop and measure, and it can also be BOUND in place of the original to see in
game which surfaces the alpha actually drives.

TWO THINGS IT DELIBERATELY DOES NOT DO, because both would corrupt the values it exists to show:

* it never COMPRESSES the output. BC7 is lossy, and this file's whole point is that red equals the
  source's alpha exactly -- a couple of levels of block-compression error would make a measurement
  taken from it wrong in a way nothing downstream could detect.
* it clears the GAMMA. A source whose DX10 header carries the sRGB bit is opened with gamma 1/2.2,
  and saving would then apply that curve to values that are not colour at all; 178 would come back
  as 219. The copy is written linear and untagged.

It verifies both after writing, by reading the result back and comparing red against the source's
alpha, and it fails loudly rather than leaving a file that merely looks right.
"""
import argparse
import os
import sys

Here = os.path.dirname(os.path.abspath(__file__))
MainRepo = r"C:\Users\AlexX\Documents\Games\Mods\Repos\Anime-Game-Remap"


def knowsWuWa(repo: str) -> bool:
    """Does this checkout's BUILT api carry Wuthering Waves?

    Asked of the compiled module rather than the source: a checkout can sit on a branch whose
    `core.*.pyd` predates WWMIBuilder, and it imports perfectly well before failing later.
    """
    package = os.path.join(repo, "Anime Game Remap (for all users)", "api", "src", "py", "FixRaidenBoss2")
    if (not os.path.isdir(package)):
        return False
    try:
        for name in os.listdir(package):
            if (name.startswith("core.") and name.endswith(".pyd")):
                with open(os.path.join(package, name), "rb") as f:
                    if (b"WWMIBuilder" in f.read()):
                        return True
    except OSError:
        return False
    return False


def apiPath() -> str:
    if (os.environ.get("AG_REMAP_REPO")):
        candidates = [os.environ["AG_REMAP_REPO"]]
    else:
        candidates = [MainRepo]
        worktrees = os.path.join(MainRepo, ".claude", "worktrees")
        if (os.path.isdir(worktrees)):
            candidates += [os.path.join(worktrees, n) for n in sorted(os.listdir(worktrees))]
    for repo in candidates:
        if (knowsWuWa(repo)):
            return os.path.abspath(os.path.join(repo, "Anime Game Remap (for all users)", "api", "src", "py"))
    raise SystemExit("no checkout here has a built API that knows Wuthering Waves.\n"
                     "Set AG_REMAP_REPO to one that does.\n  tried: " + "\n         ".join(candidates))


def main():
    parser = argparse.ArgumentParser(description = "write a copy of a texture whose RED is the source's ALPHA")
    parser.add_argument("texture", help = "the .dds to read (absolute, or relative to this folder)")
    parser.add_argument("--suffix", default = "Copy", help = "what to put before the extension (default: Copy)")
    parser.add_argument("-f", "--force", action = "store_true", help = "overwrite the output if it exists")
    args = parser.parse_args()

    src = args.texture if (os.path.isabs(args.texture)) else os.path.join(Here, args.texture)
    src = os.path.normpath(src)
    if (not os.path.isfile(src)):
        raise SystemExit(f"no such file: {src}")

    stem, ext = os.path.splitext(src)
    dest = f"{stem}{args.suffix}{ext}"
    if (os.path.exists(dest) and not args.force):
        raise SystemExit(f"{dest} already exists; pass --force to overwrite")

    sys.path.insert(0, apiPath())
    import numpy as np
    import FixRaidenBoss2 as FRB

    texture = FRB.TextureFile(src)
    texture.open()
    px = np.frombuffer(texture.getPixels(), dtype = np.uint8).reshape(texture.height, texture.width, 4)
    alpha = px[..., 3].copy()

    out = np.zeros_like(px)
    out[..., 0] = alpha
    out[..., 3] = 255

    # written through a COPY of the source so the container and its header come along, and saved
    #   uncompressed and ungamma'd so red comes back exactly equal to the source's alpha
    import shutil
    shutil.copyfile(src, dest)
    written = FRB.TextureFile(dest)
    written.open()
    try:
        written.gamma = None
    except Exception:
        pass                                    # older builds expose it read-only; the check below catches it
    written.setPixels(out.tobytes(), out.shape[1], out.shape[0])
    written.save(compress = False)

    # ACCEPTANCE: read the file back and require red to BE the source's alpha. A copy that merely
    #   looks plausible is worthless here, because every later measurement is taken off it.
    check = FRB.TextureFile(dest)
    check.open()
    got = np.frombuffer(check.getPixels(), dtype = np.uint8).reshape(check.height, check.width, 4)
    if (got.shape[:2] != px.shape[:2]):
        raise SystemExit(f"the written file is {got.shape[1]}x{got.shape[0]}, the source {px.shape[1]}x{px.shape[0]}")
    worst = int(np.abs(got[..., 0].astype(int) - alpha.astype(int)).max())
    print(f"source : {src}")
    print(f"written: {dest}")
    print(f"   {px.shape[1]} x {px.shape[0]}, alpha mean {alpha.mean():.1f}  min {alpha.min()}  max {alpha.max()}")
    print(f"   red vs the source's alpha: worst difference {worst}")
    if (worst):
        raise SystemExit("   FAILED: red does not equal the source's alpha, so this copy cannot be measured from.")
    print("   exact.")


if (__name__ == "__main__"):
    main()
