"""What a mod's toggles are, and the keystroke that moves each one.

A remap is not tested until its toggles are, because a `$swapvar` selects a different index buffer
and a different texture set -- the shapes that have broken before (the merge's branch pairing, a
variant carrying stale hashes, a resource list that only the second branch names). The default
state exercises exactly one combination of them.

3DMigoto declares a toggle as a `[Key...]` section with a `key =` binding and a `$var` it cycles.
This prints one row per toggle -- its variable, its key, how many states it has, and any
`toggle.txt` the author shipped -- and, with `--keys`, just the keystrokes in a form
`Tools/GameView`'s `do` accepts, so a sweep is one command:

    py -3 wwmiToggles.py <mod folder> --keys
    py -3 main.py do "key up; wait 1; key up; wait 1"

`$swapvar` cycling is modulo its state count, so pressing one key N times returns to where it
started: a sweep needs no undo, and leaving a mod mid-cycle is what makes the NEXT mod's screenshot
lie.
"""
import argparse
import os
import re
import sys
from collections import OrderedDict


def iniFiles(folder):
    for root, _dirs, names in os.walk(folder):
        for name in sorted(names):
            if (name.lower().endswith(".ini")):
                yield os.path.join(root, name)


def toggles(folder):
    """[{section, key, var, states, file}], in declaration order, de-duplicated by key + var"""
    found = OrderedDict()
    for path in iniFiles(folder):
        try:
            with open(path, "r", encoding = "utf-8", errors = "replace") as f:
                text = f.read()
        except OSError:
            continue
        # a [Key...] section runs to the next section header
        for m in re.finditer(r"^\[(Key[^\]]*)\]\s*$(.*?)(?=^\[|\Z)", text, re.MULTILINE | re.DOTALL):
            name, body = m.group(1), m.group(2)
            key = None
            var = None
            states = None
            for line in body.splitlines():
                s = line.strip()
                km = re.match(r"^key\s*=\s*(.+?)\s*$", s, re.IGNORECASE)
                if (km):
                    key = km.group(1)
                vm = re.match(r"^\$(\w+)\s*=\s*", s)
                if (vm and var is None):
                    var = vm.group(1)
                tm = re.match(r"^type\s*=\s*(.+?)\s*$", s, re.IGNORECASE)
                if (tm and tm.group(1).lower() == "cycle"):
                    states = None
                cm = re.match(r"^\$\w+\s*=\s*(.+?)\s*$", s)
                if (cm and "," in cm.group(1)):
                    states = len([p for p in cm.group(1).split(",") if p.strip()])
            if (key is None):
                continue
            ident = (key.lower(), var or name)
            if (ident not in found):
                found[ident] = {"section": name, "key": key, "var": var,
                                "states": states, "file": os.path.relpath(path, folder)}
    return list(found.values())


def readme(folder):
    for root, _dirs, names in os.walk(folder):
        for name in names:
            if (name.lower() in ("toggle.txt", "toggles.txt", "readme.txt", "keys.txt")):
                try:
                    with open(os.path.join(root, name), "r", encoding = "utf-8", errors = "replace") as f:
                        return os.path.relpath(os.path.join(root, name), folder), f.read().strip()
                except OSError:
                    pass
    return None, None


#: 3DMigoto writes a chord as space-separated parts (`key = VK_CONTROL 0`), and names the modifiers
#: by their virtual-key names. GameView's `key` step wants them joined with `+` under the short
#: names. A chord that is passed through with its space intact is read as a COLUMN BREAK by anything
#: parsing this table, which is how a mod with seven toggles reported none.
Modifiers = {"vk_control": "ctrl", "vk_lcontrol": "ctrl", "vk_rcontrol": "ctrl",
             "vk_shift": "shift", "vk_lshift": "shift", "vk_rshift": "shift",
             "vk_menu": "alt", "vk_lmenu": "alt", "vk_rmenu": "alt"}


def toDo(key):
    """3DMigoto's key spelling -> what GameView's `key` step accepts"""
    parts = [p for p in key.strip().lower().replace("no_modifiers", "").split() if p]
    return "+".join(Modifiers.get(p, p) for p in parts)


parser = argparse.ArgumentParser()
parser.add_argument("folder")
parser.add_argument("--keys", action = "store_true", help = "print only the keystrokes, one per line")
args = parser.parse_args()

folder = os.path.abspath(args.folder)
if (not os.path.isdir(folder)):
    raise SystemExit(f"not a folder: {folder}")

rows = toggles(folder)
if (args.keys):
    # `<keystroke> <states>` per line: whitespace-free fields, so a caller splits rather than
    #   matching columns. A chord's own space is gone by here (see Modifiers).
    for r in rows:
        print(f"{toDo(r['key'])} {r['states'] or 2}")
    sys.exit(0)

print(f"{os.path.basename(folder)}: {len(rows)} toggle(s)\n")
if (rows):
    print(f"{'key':22s} {'variable':22s} {'states':>6}  section")
    for r in rows:
        print(f"{r['key']:22s} {str(r['var']):22s} {str(r['states'] or '?'):>6}  {r['section']}")
name, text = readme(folder)
if (text):
    print(f"\nthe author's own notes ({name}):\n")
    for line in text.splitlines():
        if (line.strip()):
            print(f"  {line.strip()}")
