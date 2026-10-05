"""
FixModHashes.py -- a launcher for the repo's ModHashFixer tool, run from this WWMI folder.

    py -3 FixModHashes.py Chisa16                    report what is stale; writes NOTHING
    py -3 FixModHashes.py Chisa16 --apply            write it (a backup is kept per .ini)
    py -3 FixModHashes.py Chisa16 --undo             put the backups back
    py -3 FixModHashes.py Chisa16 --maps table.json --apply     your own old -> new hashes
    py -3 FixModHashes.py --help                     every option the tool takes

WHAT IT IS FOR: a WuWa mod whose textures look "broken" is usually a mod whose HASHES have gone
stale. WWMI binds textures by hash, the game rehashes them between versions, and once the hash the
author exported stops matching anything the game emits the override never fires -- so the surface
draws with the GAME's own art instead of the mod's. Nothing is corrupt. This resolves each stale
hash back to the role it played and forward to that role's current hash, out of the history the
library carries, and needs no frame dump.

A bare mod name is taken as a folder under this one, so `Chisa16` means `Mods\\Chisa16`; anything
else is passed through untouched, so an absolute path works too.

Nothing is fixed by this file itself: it finds a checkout whose BUILT API knows Wuthering Waves and
hands the command line to that checkout's `Tools/ModHashFixer/main.py`. Set AG_REMAP_REPO to force a
particular checkout.
"""
import os
import runpy
import sys

Here = os.path.dirname(os.path.abspath(__file__))
MainRepo = r"C:\Users\AlexX\Documents\Games\Mods\Repos\Anime-Game-Remap"


def knowsWuWa(repo: str) -> bool:
    """Does this checkout's BUILT api carry Wuthering Waves?

    Asked of the compiled module rather than of the source, and it is not a formality: the main
    checkout here sits on a release branch whose `core.*.pyd` predates WWMIBuilder entirely, so
    taking the first checkout that merely EXISTS gets an API that imports fine and then has no WuWa
    character in it.
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


def repos():
    if (os.environ.get("AG_REMAP_REPO")):
        return [os.environ["AG_REMAP_REPO"]]
    found = [MainRepo]
    worktrees = os.path.join(MainRepo, ".claude", "worktrees")
    if (os.path.isdir(worktrees)):
        found += [os.path.join(worktrees, name) for name in sorted(os.listdir(worktrees))]
    return found


if (__name__ == "__main__"):
    # BOTH conditions, not either: the main checkout here has a built API that knows WuWa (a
    #   Sanhua-era build) and does NOT have the tool, while the worktree carrying the tool is the one
    #   whose API is current. Taking the first checkout that satisfies one of them picks neither.
    candidates = repos()
    repo = next((c for c in candidates
                 if (knowsWuWa(c) and os.path.isfile(os.path.join(c, "Tools", "ModHashFixer", "main.py")))), None)
    if (repo is None):
        lines = []
        for c in candidates:
            hasTool = os.path.isfile(os.path.join(c, "Tools", "ModHashFixer", "main.py"))
            lines.append(f"    {c}\n      built API knows WuWa: {knowsWuWa(c)}   has the tool: {hasTool}")
        raise SystemExit("no checkout here has BOTH a built API that knows Wuthering Waves and the ModHashFixer\n"
                         "tool. Build one, or set AG_REMAP_REPO to one that has both.\n" + "\n".join(lines))

    tool = os.path.join(repo, "Tools", "ModHashFixer")
    entry = os.path.join(tool, "main.py")

    args = sys.argv[1:]
    # a bare mod name means a folder beside this launcher, which is how every other script here is used
    if (args and not args[0].startswith("-")):
        local = os.path.join(Here, args[0])
        if (os.path.isdir(local)):
            args[0] = local

    print(f"using {repo}\n")
    sys.path.insert(0, tool)
    sys.argv = [entry] + args
    runpy.run_path(entry, run_name = "__main__")
