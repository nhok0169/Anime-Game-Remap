import argparse
import sys

from ModHashFixer.ModHashFixer import ModHashFixer


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description = "Fixes a WuWa mod whose textures have 'broken' by bringing its stale hashes onto the current game version",
        epilog = "The game REHASHES a texture between versions, and a WWMI mod binds its textures by hash\n"
                 "([TextureOverrideTexture<N>] hash = <h>). Once that hash no longer matches anything the game\n"
                 "emits, the override never fires and the surface draws with the GAME's own art -- which looks\n"
                 "like the mod's textures are broken. Nothing is corrupt; the mod is addressing textures that no\n"
                 "longer exist under those names. This is the WuWa counterpart of what ORFix does for GI.\n"
                 "\n"
                 "It reports and writes nothing unless --apply is passed, and keeps one backup per .ini.\n"
                 "\n"
                 "Examples:\n"
                 "  python3 main.py \"WWMI/Mods/SomeChisaMod\"\n"
                 "  python3 main.py \"WWMI/Mods/SomeChisaMod\" --apply\n"
                 "  python3 main.py \"WWMI/Mods/SomeChisaMod\" --character Chisa --version 3.6 --apply\n"
                 "  python3 main.py \"WWMI/Mods/SomeChisaMod\" --undo",
        formatter_class = argparse.RawDescriptionHelpFormatter
    )

    parser.add_argument("mod", type = str, help = "the mod folder -- every .ini under it, DISABLED ones skipped")
    parser.add_argument("-c", "--character", type = str, default = None,
                        help = "the WuWa character the mod is FOR; detected from the hashes themselves when omitted")
    parser.add_argument("-v", "--version", type = str, default = None,
                        help = "the game version to bring the hashes to (default: the newest the library knows)")
    parser.add_argument("-a", "--apply", action = "store_true",
                        help = "write the change; without it nothing is written and the run only reports")
    parser.add_argument("-u", "--undo", action = "store_true", help = "restore the .ini files this tool backed up")
    parser.add_argument("-m", "--maps", nargs = "*", default = [], metavar = "JSON",
                        help = "your own old -> new hash tables (json, any nesting -- the community fixers' "
                               "hash_maps.json shape). These WIN over the library, and are how you fix a mod of a "
                               "character the library does not carry")
    parser.add_argument("-g", "--geometry", action = "store_true",
                        help = "also retarget vb0 / cb4 / the shape-key pair. Only for a mod that does not draw AT ALL: "
                               "on a mod that draws, rewriting these breaks what works")

    args = parser.parse_args()

    try:
        extra = ModHashFixer.loadHashMaps(*args.maps) if (args.maps) else {}
        if (extra):
            print(f"{len(extra)} hash pair(s) loaded from your table(s); these win over the library\n")
        fixer = ModHashFixer(args.mod, character = args.character, version = args.version,
                             geometry = args.geometry, extraHashes = extra)
    except Exception as e:
        print(f"error: {e}", file = sys.stderr)
        sys.exit(1)

    if (not fixer.files):
        print(f"error: no .ini found under {args.mod}", file = sys.stderr)
        sys.exit(1)

    if (args.undo):
        restored = fixer.undo()
        for path in restored:
            print(f"  restored {path}")
        print(f"{len(restored)} file(s) restored" if restored else "no backups from this tool were found")
        sys.exit(0)

    known = ModHashFixer.characters()
    if (args.character and args.character not in known):
        print(f"error: {args.character} is not a registered WuWa character ({', '.join(sorted(known))})", file = sys.stderr)
        sys.exit(1)

    name, scores = (args.character, None) if (args.character) else fixer.detect()
    if (name is None):
        print("error: no hash in this mod resolves to any registered WuWa character. Either it is not one of\n"
              "       theirs, or the hash history in the library does not reach back to the version it was made for.",
              file = sys.stderr)
        sys.exit(1)

    print(f"character: {name}" + ("" if (args.character or not scores) else
          "   (detected; " + ", ".join(f"{k} explained {v}" for k, v in scores.items() if (k != name)) + ")"))
    print(f"target version: {args.version or 'the newest the library knows'}\n")

    changes, counts, unrecognised, newBytes = fixer.plan(known[name])
    for rel, section, role, old, new in changes:
        print(f"  {rel} [{section}]  {role:22s} {old} -> {new}")

    print(f"\n{len(changes)} hash(es) to update, {counts['current']} already current, "
          f"{counts['geometry']} geometry left alone, {sum(unrecognised.values())} unrecognised")
    if (unrecognised):
        print("  unrecognised -- not this character's, or older than the recorded history (left alone):")
        for value, n in unrecognised.most_common(12):
            print(f"    {value} x{n}")

    if (not changes):
        print("\nnothing to do.")
        sys.exit(0)
    if (not args.apply):
        print("\nnothing written. Pass --apply to write it (one backup is kept per .ini).")
        sys.exit(0)

    for path in fixer.write(newBytes):
        print(f"  wrote {path}")
    print(f"{len(newBytes)} file(s) written. `--undo` restores them.")
