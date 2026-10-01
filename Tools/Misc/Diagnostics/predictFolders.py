r"""Which `.ini` files should change where the fix writes, once the mod ROOT can win the tally?

Replays `readTextureFolder`'s arithmetic both ways over every mod `.ini`:

    old   count only a declared file whose path HAS a slash; most wins; ties to the lowest path
    new   count the root as a folder too

and reports every `.ini` whose winner moves. That is the prediction the corpus sweep has to match --
a sweep that shows MORE than this changed is a surprise to explain, and one that shows less means the
fix did not reach where it was supposed to.
"""
import collections
import os
import pathlib
import re
import sys

Wwmi = pathlib.Path(r"C:\Users\AlexX\Documents\Games\Mods\XXMI-Launcher-Portable-v1.8.5\WWMI")
Skip = re.compile(r"^(FrameAnalysis|Shader(Cache|Fixes|Backups)|Core|Mods)$", re.I)
sys.stdout.reconfigure(errors = "replace")


def winner(folders):
    """most wins; a tie goes to the lowest path -- std::map order, strictly-greater keeps the first"""
    best, at = 0, ""
    for name in sorted(folders):
        if folders[name] > best:
            best, at = folders[name], name

    return at


moved = []
for root in (Wwmi, Wwmi / "Mods"):
    if not root.is_dir():
        continue

    for name in sorted(os.listdir(root)):
        at = root / name
        if not at.is_dir() or (root == Wwmi and Skip.match(name)):
            continue

        mod = name if root == Wwmi else "Mods/" + name
        for ini in sorted(at.rglob("*.ini")):
            if "remapfix" in ini.name.lower():
                continue

            old, new = collections.Counter(), collections.Counter()
            for line in ini.read_text(encoding = "utf-8", errors = "replace").split("\n"):
                m = re.match(r"\s*filename\s*=\s*(.+?)\s*$", line, re.I)
                if not m or "Remap" in m.group(1):
                    continue

                rel = m.group(1).strip().replace("\\", "/")
                if re.match(r"^[a-zA-Z]:/", rel):
                    continue                              # absolute: not a folder of this mod

                rel = rel.lstrip("./")
                slash = rel.rfind("/")
                folder = rel[:slash] if slash > 0 else ""
                if folder:
                    old[folder] += 1

                new[folder] += 1

            a, b = winner(old), winner(new)
            if a != b:
                moved.append((mod, ini.name, a or "(root)", b or "(root)",
                              dict(new.most_common(3))))

print("{} .ini file(s) whose write folder moves\n".format(len(moved)))
for mod, ini, a, b, counts in moved:
    print("   {:<34} {:<26} {} -> {}".format(mod, ini[:26], a, b))
    print("      counts: " + ", ".join("{}={}".format(k or "(root)", v) for k, v in counts.items()))
