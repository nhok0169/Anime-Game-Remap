r"""Orbit the overworld camera around the player and capture at every angle, as one contact sheet.

A single screenshot judges a remap from the one side it happens to face, and several of this pair's
defects were only visible from another: a seam, a stretched flap, a part wearing the wrong texture.
This turns "look at every angle" into a command.

Two things it exists to get right, both learned the hard way on 2026-09-28:

* **The camera follows plain mouse MOTION in the overworld, so it needs a relative delta.** Driving
  it by moving the cursor between two absolute positions gives `+d` then `-d`, which cancels -- an
  orbit written that way wobbles in place and reads as "the camera is stuck". `GameView look dx dy`
  is the relative primitive. Negative `dy` looks UP; the pitch drifts to overhead on its own, so
  level it before a run (`--level`) or the whole set is a top-down view of the character's hair.

* **WWMI hands back the LAST frame it drew while it is not focused.** Two captures minutes apart
  came back pixel-IDENTICAL, which reads exactly like "the change did nothing" -- every verdict
  taken from them was void. Every capture here goes through `--stay --settle`, and `--verify`
  re-captures one angle to prove consecutive frames differ before trusting any of them.

  py -3 wwmiOrbit.py <name> [--steps 12] [--dx 200] [--level] [--verify]

`look 600 0` turned roughly 90-120 degrees on the machine this was written on, so the default 200 x
12 covers about one turn; check the sheet and adjust rather than assuming.
"""
import argparse
import glob
import os
import subprocess
import sys

GameView = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "GameView")
Shots = os.path.expanduser(r"~\.claude\gameview\shots")


def gameView(*args):
    return subprocess.run([sys.executable, "main.py", *args], capture_output = True, text = True,
                          cwd = GameView).stdout.strip()


def capture(name):
    out = gameView("screenshot", "--stay", "--settle", "2", "--name", name)
    for line in out.splitlines():
        if line.startswith("view:"):
            return line.split("view:", 1)[1].split(" (")[0].strip()

    return None


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("name")
    ap.add_argument("--steps", type = int, default = 12)
    ap.add_argument("--dx", type = int, default = 200)
    ap.add_argument("--pitch", type = int, default = 0,
                    help = "a RELATIVE pitch nudge before the orbit: negative looks up, positive down. "
                           "There is no absolute reading to level against, so set it per run and look")
    ap.add_argument("--verify", action = "store_true", help = "prove the game is drawing new frames first")
    ap.add_argument("--crop", nargs = 4, type = float, default = [0.26, 0.10, 0.50, 0.88],
                    help = "the fraction of the frame to keep per tile: x y w h")
    args = ap.parse_args()

    if args.pitch:
        gameView("look", "0", str(args.pitch))

    if args.verify:
        from PIL import Image, ImageChops           # noqa: PLC0415 -- only needed for the check
        import numpy as np                          # noqa: PLC0415

        a, b = capture(args.name + "_v0"), capture(args.name + "_v1")
        diff = np.asarray(ImageChops.difference(Image.open(a).convert("RGB"),
                                                Image.open(b).convert("RGB"))).sum()
        if diff == 0:
            sys.exit("the game is not drawing: two captures are pixel-identical. Focus it and retry.")

        print("live: two consecutive captures differ")

    shots = []
    for i in range(args.steps):
        if i:
            gameView("look", str(args.dx), "0")

        shot = capture(f"{args.name}{i:02d}")
        if shot:
            shots.append(shot)

    from PIL import Image                           # noqa: PLC0415

    x, y, w, h = args.crop
    cols = min(6, max(1, len(shots)))
    rows = (len(shots) + cols - 1) // cols
    tiles = []
    for path in shots:
        im = Image.open(path).convert("RGB")
        W, H = im.size
        tiles.append(im.crop((int(W * x), int(H * y), int(W * (x + w)), int(H * (y + h))))
                       .resize((280, 430)))

    sheet = Image.new("RGB", (280 * cols, 430 * rows))
    for i, tile in enumerate(tiles):
        sheet.paste(tile, ((i % cols) * 280, (i // cols) * 430))

    out = os.path.join(Shots, args.name + "_orbit.png")
    sheet.save(out)
    print(f"{len(shots)} angle(s) -> {out}")


if __name__ == "__main__":
    main()
