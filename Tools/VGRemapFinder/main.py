import argparse
import sys

from src.VGRemapFinder.VGMatcher import VGMatcher
from src.VGRemapFinder.VGRemapFinder import VGRemapFinder


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description = "Proposes the vertex group remap between two mods from their geometry (3dmigoto dumps, or a mod's raw .buf files): compares the vertex groups "
                      "by where their vertices sit and how far they spread, and aligns whole chains of bones at once",
        epilog = "Examples:\n"
                 "  python3 main.py \"PlayerCharacterData/Ganyu\" \"PlayerCharacterData/GanyuTwilight\" -o \"C:/scratch/GanyuRemapDraft.xlsx\" -v 4.4\n"
                 "  python3 main.py \"PlayerCharacterData/Ganyu\" \"PlayerCharacterData/GanyuTwilight\" -c \"Data/RemapDrafts/GanyuRemapDraft.xlsx\"\n"
                 "  python3 main.py \"PlayerCharacterData/Ganyu\" \"PlayerCharacterData/GanyuTwilight\" --metric center --mode nearest\n"
                 "  python3 main.py \"Data/Mod Downloads/GI/Ganyu/4_0\" \"PlayerCharacterData/GanyuTwilight\" -o \"C:/scratch/GanyuRemapDraft.xlsx\"",
        formatter_class = argparse.RawDescriptionHelpFormatter
    )

    parser.add_argument("fromFolder", type = str, help = "the geometry of the mod to be remapped: a dump folder (*-vb0=<hash>.txt and *-ib=<hash>.txt files), the folder of a mod's raw files (*Position.buf, *Blend.buf and *.ib), or a raw 3dmigoto frame analysis folder (with --fromHashes), told apart by what it holds")
    parser.add_argument("toFolder", type = str, help = "the geometry of the mod to remap onto, in any of those forms")
    parser.add_argument("-o", "--output", type = str, default = None, help = "the .xlsx to write the proposed remap into, in the Data/RemapDrafts format (sheets for the same directions in an existing workbook are replaced, other sheets are kept)")
    parser.add_argument("-c", "--compare", type = str, default = None, help = "an existing draft workbook to score the proposal against; the sheet is found by its header row")
    parser.add_argument("-C", "--compareLibrary", action = "store_true", help = "also score the proposal against the remap the AG Remap library ships for the pair (the mod names must be the library's own, eg. Keqing / KeqingOpulent)")
    parser.add_argument("--fromHashes", type = str, nargs = "+", default = None, metavar = "HASH", help = "when fromFolder is a raw 3dmigoto frame analysis: the character's position, blend and ib hashes (as in its hash.json), to pick its files out of the thousands there -- or the path of its hash.json, which is the only way to give a character of several components. Give a frame analysis without them to be shown which hashes it holds")
    parser.add_argument("--toHashes", type = str, nargs = "+", default = None, metavar = "HASH", help = "as --fromHashes, for toFolder")
    parser.add_argument("--fromName", type = str, default = None, help = "the name of the mod to be remapped (default: a dump folder's name, or what precedes 'Position.buf' in a mod's position file)")
    parser.add_argument("--toName", type = str, default = None, help = "the name of the mod to remap onto (default: as for --fromName)")
    parser.add_argument("-v", "--version", type = str, default = None, help = "the game version the remap targets, used in the sheet titles")
    parser.add_argument("-m", "--metric", type = str, default = VGMatcher.MetricGaussian, choices = VGMatcher.Metrics,
                        help = "how far apart two vertex groups are: 'gaussian' (default) compares each group's centre AND the spread of its vertices, 'center' compares the centres only")
    parser.add_argument("--mode", type = str, default = VGMatcher.ModeChains, choices = VGMatcher.Modes,
                        help = "how matches are chosen: 'chains' (default) aligns each run of consecutive source indices onto the target indices as a whole, 'nearest' maps every group independently onto its closest one")
    parser.add_argument("--stayCost", type = float, default = VGMatcher.DefaultStayCost, help = f"chains mode: the cost (in bone spacings) of consecutive sources landing on the same target (default: {VGMatcher.DefaultStayCost})")
    parser.add_argument("--skipCost", type = float, default = VGMatcher.DefaultSkipCost, help = f"chains mode: the cost of skipping one target index (default: {VGMatcher.DefaultSkipCost})")
    parser.add_argument("--jumpCost", type = float, default = VGMatcher.DefaultJumpCost, help = f"chains mode: the cost of any other step (default: {VGMatcher.DefaultJumpCost})")
    parser.add_argument("-u", "--unweighted", action = "store_true", help = "count every vertex a group touches equally when summarising it, instead of weighting each by its blend weight (the default, and the more accurate one)")
    parser.add_argument("--oneWay", action = "store_true", help = "only propose from -> to, not the reverse remap as well")
    parser.add_argument("-s", "--silent", action = "store_true", help = "don't print progress or the summary")

    args = parser.parse_args()

    finder = VGRemapFinder(args.fromFolder, args.toFolder, fromName = args.fromName, toName = args.toName,
                           version = args.version, weighted = not args.unweighted, bothWays = not args.oneWay,
                           metric = args.metric, mode = args.mode,
                           stayCost = args.stayCost, skipCost = args.skipCost, jumpCost = args.jumpCost,
                           fromHashes = args.fromHashes, toHashes = args.toHashes)

    try:
        finder.run(output = args.output, compareTo = args.compare, silent = args.silent, compareLibrary = args.compareLibrary)
    except (FileNotFoundError, ImportError, KeyError, ValueError) as e:
        print(f"error: {e}", file = sys.stderr)
        sys.exit(1)

    sys.exit(0)
