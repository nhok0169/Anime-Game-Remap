"""
Scores the finder against every hand-made draft in ``Data/RemapDrafts/`` whose two characters both
have dumps available, so a change to the matching can be judged on all of them at once rather than
on the one character it was tuned on.

Usage::

    python3 benchmark.py <PlayerCharacterData folder> [--drafts <folder>] [--metric ...] [--mode ...] [--only Ganyu Klee]

The dumps are read once and cached as pickles under ``--cache`` (default: a ``.benchmarkCache``
folder next to this script), since reading them is most of the run time.
"""

import argparse
import os
import pickle
import sys
from typing import Dict, List, Optional, Tuple

from src.VGRemapFinder.DraftWriter import DraftWriter
from src.VGRemapFinder.DumpMod import DumpMod
from src.VGRemapFinder.VGMatcher import VGMatcher
from src.VGRemapFinder.VertexGroups import VertexGroups
from src.VGRemapFinder.constants.Paths import RepoPath


# Where a draft's header names differ from the dump folders' names
FolderAliases = {
    "Ayaka": "KamisatoAyaka",
    "AyakaSpringBloom": "AyakaSpringbloom",
    "Ningguang Orchid": "NingguangOrchid",
    "Hutao": "HuTao",
}


def loadMod(dumpsFolder: str, name: str, cacheFolder: Optional[str]) -> Optional[DumpMod]:
    folder = os.path.join(dumpsFolder, FolderAliases.get(name, name))
    if (not os.path.isdir(folder)):
        return None

    cachePath = None
    if (cacheFolder is not None):
        os.makedirs(cacheFolder, exist_ok = True)
        cachePath = os.path.join(cacheFolder, f"{os.path.basename(folder)}.pkl")
        if (os.path.isfile(cachePath) and os.path.getmtime(cachePath) >= max(os.path.getmtime(os.path.join(folder, f)) for f in os.listdir(folder))):
            try:
                with open(cachePath, "rb") as f:
                    return pickle.load(f)
            except Exception:
                pass    # a stale cache (eg. pickled by an older layout of this tool): re-read the dumps below

    mod = DumpMod.fromFolder(folder, name = name, silent = True)
    if (cachePath is not None):
        with open(cachePath, "wb") as f:
            pickle.dump(mod, f)
    return mod


def draftSheets(draftsFolder: str, skipped: List[str]) -> List[Tuple[str, str, str]]:
    """Every (workbook, fromName, toName) sheet of the hand-made drafts in the folder; a workbook this
    tool itself wrote (see DraftWriter.isProposal) is not a draft and goes into 'skipped' instead"""

    import openpyxl

    result = []
    for fileName in sorted(os.listdir(draftsFolder)):
        if (not fileName.lower().endswith(".xlsx") or fileName.startswith("~$")):
            continue

        path = os.path.join(draftsFolder, fileName)
        if (DraftWriter.isProposal(path)):
            skipped.append(f"{fileName} (a proposal written by this tool, not a hand-made draft)")
            continue

        workbook = openpyxl.load_workbook(path, read_only = True)
        seen = set()
        for sheet in workbook.worksheets:
            header = next(sheet.iter_rows(min_row = 1, max_row = 1, values_only = True), None)
            if (header is None or len(header) < 2 or header[0] is None or header[1] is None):
                continue

            if (DraftWriter.isProposedSheet(sheet)):
                skipped.append(f"{fileName} / {sheet.title} (a sheet this tool proposed, not hand-made)")
                continue

            fromName, toName = str(header[0]).strip(), str(header[1]).strip()
            if (fromName == toName or (fromName, toName) in seen):
                continue
            seen.add((fromName, toName))
            result.append((path, fromName, toName))

    return result


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description = "Scores the finder against every hand-made draft whose characters both have dumps")
    parser.add_argument("dumpsFolder", type = str, help = "the PlayerCharacterData folder of GI-Model-Importer-Assets")
    parser.add_argument("--drafts", type = str, default = os.path.join(RepoPath, "Data", "RemapDrafts"), help = "the folder of draft workbooks (default: the repo's Data/RemapDrafts)")
    parser.add_argument("--cache", type = str, default = os.path.join(os.path.dirname(os.path.abspath(__file__)), ".benchmarkCache"), help = "where to cache the read dumps ('' to not cache)")
    parser.add_argument("--only", type = str, nargs = "*", default = None, help = "only score sheets whose source name is one of these")
    parser.add_argument("-m", "--metric", type = str, default = VGMatcher.MetricGaussian, choices = VGMatcher.Metrics)
    parser.add_argument("--mode", type = str, default = VGMatcher.ModeChains, choices = VGMatcher.Modes)
    parser.add_argument("--stayCost", type = float, default = VGMatcher.DefaultStayCost)
    parser.add_argument("--skipCost", type = float, default = VGMatcher.DefaultSkipCost)
    parser.add_argument("--jumpCost", type = float, default = VGMatcher.DefaultJumpCost)
    parser.add_argument("-u", "--unweighted", action = "store_true")
    parser.add_argument("--verbose", action = "store_true", help = "list every disagreement")
    args = parser.parse_args()

    cacheFolder = args.cache if (args.cache) else None
    groupsByName: Dict[str, Optional[VertexGroups]] = {}

    def groupsFor(name: str) -> Optional[VertexGroups]:
        if (name not in groupsByName):
            mod = loadMod(args.dumpsFolder, name, cacheFolder)
            groupsByName[name] = None if (mod is None) else VertexGroups(mod, weighted = not args.unweighted)
        return groupsByName[name]

    totalAgree = 0
    totalScored = 0
    skipped: List[str] = []

    print(f"metric: {args.metric}, mode: {args.mode}, stay/skip/jump: {args.stayCost}/{args.skipCost}/{args.jumpCost}, weighted: {not args.unweighted}")
    for path, fromName, toName in draftSheets(args.drafts, skipped):
        if (args.only is not None and fromName not in args.only):
            continue

        fromGroups, toGroups = groupsFor(fromName), groupsFor(toName)
        if (fromGroups is None or toGroups is None):
            skipped.append(f"{fromName} -> {toName} (no dumps for {'both' if (fromGroups is None and toGroups is None) else fromName if (fromGroups is None) else toName})")
            continue

        expected = DraftWriter.readSheet(path, fromName, toName)
        known = {i: t for i, t in expected.items() if (t is not None)}
        if (not known or max(known) >= len(fromGroups) or max(known.values()) >= len(toGroups)):
            skipped.append(f"{fromName} -> {toName} (the draft's indices do not fit the dumps: draft up to {max(known) if known else '-'} -> {max(known.values()) if known else '-'}, dumps {len(fromGroups)} -> {len(toGroups)} groups)")
            continue

        matcher = VGMatcher(fromGroups, toGroups, metric = args.metric, mode = args.mode,
                            stayCost = args.stayCost, skipCost = args.skipCost, jumpCost = args.jumpCost)
        matches = matcher.match()

        agree = sum(1 for match in matches if (match.fromIndex in known and match.toIndex == known[match.fromIndex]))
        scored = len(known)
        totalAgree += agree
        totalScored += scored

        wrong = [(match.fromIndex, match.toIndex, known[match.fromIndex]) for match in matches if (match.fromIndex in known and match.toIndex != known[match.fromIndex])]
        line = f"  {fromName:>24s} -> {toName:<24s} {agree:4d} / {scored:<4d} ({100.0 * agree / scored:5.1f}%)"
        if (args.verbose):
            line += f"  wrong (from, proposed, expected): {wrong}"
        print(line)

    if (totalScored):
        print(f"TOTAL: {totalAgree} / {totalScored} ({100.0 * totalAgree / totalScored:.1f}%)")
    for line in skipped:
        print(f"  skipped: {line}")

    sys.exit(0 if (totalScored) else 1)
