#
# ===== wwmiPassCoverage =====
#
# Every pass the TARGET draws each slot on, against the passes a WuWa fix binds textures for. A
# remapped section fires per DRAW (its hash plus index window), but the texture command list inside
# it is gated on `ps == <filter>` -- so a pass the config does not name draws the MOD's geometry
# with the GAME's textures, silently, and only for that pass. Chisa's hair ribbon came out in
# ChisaParfait's pink that way (2026-09-20): her accessory slot draws on THREE art passes and the
# config named one, so two of the three draws used the skin's own art.
#
#   python wwmiPassCoverage.py <wwmiDrawTable --json output> [--config <fix prototype .py>]
#
# The last column separates art from globals without needing a role table: a texture bound on one or
# two of the target's slots is that slot's own art, one bound on most of them is a global the fix
# should leave alone. A pass whose art column is empty is an outline or shadow pass and needs no
# entry; a pass with art and no coverage is a gap.
#
# --config reads the `SlotPasses` literal out of a prototype (the shape in chisaParfaitFix.py /
# sanhuaExorcistFix.py) so the two cannot drift; without it, pass `--covered 3:hash,hash`.
#

import argparse
import ast
import collections
import json
import re
import sys


def dictAssignment(text, name, path):
    """The AST of the dict assigned to `name` at the top level, or None.

    Parsed rather than brace-scanned: a comment or a nested brace inside a string defeats scanning,
    and the previous version needed a note explaining which line it had got wrong.
    """
    try:
        tree = ast.parse(text)
    except SyntaxError as e:
        raise SystemExit(f"cannot parse {path}: {e}")
    for node in tree.body:
        if (not isinstance(node, ast.Assign) or not isinstance(node.value, ast.Dict)):
            continue
        for target in node.targets:
            if (isinstance(target, ast.Name) and target.id == name):
                return node.value
            # `X: Dict[int, List[str]] = {...}` parses as AnnAssign, handled below
    for node in tree.body:
        if (isinstance(node, ast.AnnAssign) and isinstance(node.value, ast.Dict)
                and isinstance(node.target, ast.Name) and node.target.id == name):
            return node.value
    return None


def keysOf(node, path, name):
    """[(key, valueNode)] of a dict AST, evaluating ONLY the keys.

    The values are deliberately left as AST: `ExtraPassRegs` holds tuples and names that mean
    nothing outside the prototype's own namespace, and this tool needs none of them.
    """
    out = []
    for key, value in zip(node.keys, node.values):
        try:
            out.append((ast.literal_eval(key), value))
        except ValueError:
            raise SystemExit(f"{name} in {path} has a key that is not a literal")
    return out


def literalAfter(text, name, path):
    """{slot: [pass, ...]} from a table whose values may be lists of passes OR dicts keyed by pass"""
    node = dictAssignment(text, name, path)
    if (node is None):
        return None
    out = {}
    for slot, value in keysOf(node, path, name):
        if (isinstance(value, ast.Dict)):                       # {pass: {register: role}}
            out[slot] = [p for p, _ in keysOf(value, path, name)]
        elif (isinstance(value, (ast.List, ast.Tuple))):        # [pass, ...]
            out[slot] = [ast.literal_eval(e) for e in value.elts]
        else:
            raise SystemExit(f"{name}[{slot!r}] in {path} is neither a list of passes nor a dict keyed by pass")
    return out


def coveredFromConfig(path):
    """Both tables: the slot's primary passes, and the ones given their own register map."""
    with open(path, "r", encoding = "utf-8") as f:
        text = f.read()
    primary = literalAfter(text, "SlotPasses", path)
    if (primary is None):
        raise SystemExit(f"no SlotPasses literal in {path}")
    covered = {slot: list(passes) for slot, passes in primary.items()}
    extra = literalAfter(text, "ExtraPassRegs", path) or {}
    for slot, passes in extra.items():
        covered.setdefault(slot, []).extend(passes)
    return covered


def main():
    parser = argparse.ArgumentParser(description = "the target's passes per slot, against the passes a fix binds textures for")
    parser.add_argument("draws", help = "the --json output of wwmiDrawTable over the TARGET's frame dump")
    parser.add_argument("--config", default = None, help = "a fix prototype to read SlotPasses out of")
    parser.add_argument("--covered", nargs = "*", default = [], metavar = "SLOT:HASH,HASH", help = "the covered passes per slot, when there is no config to read")
    args = parser.parse_args()

    covered = coveredFromConfig(args.config) if args.config else {}
    for entry in args.covered:
        slot, _, passes = entry.partition(":")
        covered[int(slot)] = passes.split(",")

    with open(args.draws, "r", encoding = "utf-8") as f:
        draws = json.load(f)

    slotsOf = collections.defaultdict(set)
    for row in draws:
        for tex in (row.get("ps-t") or {}).values():
            slotsOf[tex.split("(")[0]].add(row.get("component"))

    byComp = collections.OrderedDict()
    for row in draws:
        byComp.setdefault(row.get("component"), collections.OrderedDict()).setdefault(row["ps"], row)

    gaps = 0
    for comp in sorted(byComp, key = lambda c: (c is None, c)):
        print(f"\nslot {comp}:")
        for ps, row in byComp[comp].items():
            art = [f"{reg}={tex.split('(')[0]}" for reg, tex in (row.get("ps-t") or {}).items()
                   if len(slotsOf[tex.split("(")[0]]) <= 2]
            isCovered = ps in covered.get(comp, [])
            if (art and not isCovered):
                gaps += 1
            mark = "covered" if isCovered else ("NOT COVERED" if art else "not covered")
            print(f"   {ps}  {mark:11s}  art: {', '.join(art) if art else '(globals only)'}")

    print(f"\n{gaps} pass(es) bind art and are not covered")
    return 1 if gaps else 0


if (__name__ == "__main__"):
    sys.exit(main())
