# ===== cardSeries =====
#
# One sheet per mod of the same character on TWO outfit cards (the original's and the remap's), shot in game through
# Tools/GameView: per card a front shot, then a TIMED series of back shots (3 s apart, so drape and swing show) and a
# back shot from below -- or, with --view head, a front / side / back / below set framed on the head. Rows are the two
# cards, columns the shots, every crop at the same place, so a drape fault is judged across frames and never off one.
#
# For the quick between-epoch check of a few variants (eg. three values of a push), not a replacement for the game
# agent's epoch: it shoots no toggle you do not pass, no 3/4 view and no zoom.
#
#   py -3 cardSeries.py <staging folder> <out folder> "<mod>|<keys>" ... [--view body|head]
#                       [--cardA X Y] [--cardB X Y]
#
# <keys> is a GameView `do` fragment run on card A before its shots (eg. "key 5", or "key --vk VK_RIGHT"); a
# toggle persists, so card B shows the same state. --cardA / --cardB are the two outfit cards' VIEW coordinates
# (defaults: Aether's Dressing Room, his default card and the skin's). Each mod is loaded with
# `mods GIMI only <mod> --from <staging>` and reloaded first; a later `only` parks it back.
import argparse
import os
import re
import subprocess
import sys

from PIL import Image, ImageDraw

GameView = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "GameView", "main.py")

Views = {
    # (shot names, crop box as fractions of the full shot, the sequence after the front shot)
    "body": (("f", "b0", "b1", "b2", "bb"), (0.42, 0.30, 0.58, 0.82),
             "move 600 500; drag 600 500 1000 500 --duration 1.2; wait 2; {s_b0}; wait 3; {s_b1}; wait 3; {s_b2}; "
             "drag 790 550 790 300 --duration 1.2; wait 2; {s_bb}"),
    "head": (("f", "s", "b0", "bb"), (0.36, 0.04, 0.64, 0.60),
             "move 600 500; drag 600 500 800 500 --duration 1.2; wait 2; {s_s}; "
             "drag 600 500 800 500 --duration 1.2; wait 2; {s_b0}; drag 790 550 790 300 --duration 1.2; wait 2; {s_bb}"),
}


def gameView(*args) -> str:
    result = subprocess.run([sys.executable, GameView, *args], capture_output = True, text = True,
                            encoding = "utf-8", errors = "replace")
    return result.stdout + result.stderr


def main():
    parser = argparse.ArgumentParser(description = "timed shot series of mods on two outfit cards, one sheet per mod")
    parser.add_argument("staging", help = "the folder the mods are loaded from")
    parser.add_argument("out", help = "where the sheets are written")
    parser.add_argument("mods", nargs = "+", help = "\"<mod>|<keys>\", keys optional")
    parser.add_argument("--view", choices = list(Views), default = "body")
    parser.add_argument("--cardA", nargs = 2, default = ["1071", "130"], help = "card A's view coordinates (row 1)")
    parser.add_argument("--cardB", nargs = 2, default = ["1173", "130"], help = "card B's view coordinates (row 2)")
    args = parser.parse_args()

    names, box, after = Views[args.view]
    os.makedirs(args.out, exist_ok = True)

    for spec in args.mods:
        mod, keys = (spec.split("|") + [""])[:2]
        print(gameView("mods", "GIMI", "only", mod, "--from", args.staging).strip().splitlines()[-1])
        gameView("reload", "--mod", mod)

        rows = []
        for label, card in (("A", " ".join(args.cardA)), ("B", " ".join(args.cardB))):
            toggle = f"{keys}; wait 2; " if (keys and label == "A") else ""
            shot = {f"s_{n}": f"screenshot --name {mod}_{label}_{n}" for n in names}
            sequence = (f"move {card}; wait 0.5; click {card}; wait 9; {toggle}scroll -20; wait 1; {shot['s_f']}; "
                        + after.format(**shot))
            output = gameView("do", sequence)
            shots = {n: p for p, n in re.findall(r"full: (\S+?_(" + "|".join(names) + r"))\.png", output)}

            row = []
            for n in names:
                if (n not in shots):
                    raise SystemExit(f"{mod} card {label}: no '{n}' shot in GameView's output:\n{output[-800:]}")
                image = Image.open(shots[n] + ".png").convert("RGB")
                width, height = image.size
                image = image.crop((int(width * box[0]), int(height * box[1]), int(width * box[2]), int(height * box[3])))
                draw = ImageDraw.Draw(image)
                draw.rectangle((0, 0, 150, 20), fill = (0, 0, 0))
                draw.text((3, 3), f"{label} {n}", fill = (255, 255, 0))
                row.append(image)
            rows.append(row)

        width, height = rows[0][0].size
        sheet = Image.new("RGB", (width * len(names), height * 2))
        for y, row in enumerate(rows):
            for x, image in enumerate(row):
                sheet.paste(image, (x * width, y * height))
        path = os.path.join(args.out, mod + ".png")
        sheet.save(path)
        print(mod, "->", path)


if __name__ == "__main__":
    main()
