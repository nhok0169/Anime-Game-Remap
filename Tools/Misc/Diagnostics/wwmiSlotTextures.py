r"""What each remapped slot binds at each ps-t register, and where that texture comes from.

The prototype's per-slot table is the instrument for any WuWa texture report, and `GAME (mod has
none)` on a register is the first line to read: a role the mod ships no file for falls back to the
TARGET's own downloaded texture, whose UVs are not the mod's.

The identity mod renders correctly, so its table is the reference; a real mod's differences from it
are the candidates.

  py -3 slotTexTable.py [mod name]...
"""
import pathlib
import re
import sys

W = pathlib.Path(r"C:\Users\AlexX\Documents\Games\Mods\XXMI-Launcher-Portable-v1.8.5\WWMI")
names = sys.argv[1:] or ["ChisaParfaitIdentity", "ChisaParfait1", "ChisaParfait2", "ChisaParfait3"]

for name in names:
    root = next((c for c in (W / name, W / "Mods" / name) if c.is_dir()), None)
    if (root is None):
        print("%s: NOT FOUND\n" % name)
        continue

    text = ""
    for p in sorted(root.rglob("*.ini")):
        text += p.read_bytes().decode("utf-8", "replace").replace("\r\n", "\n") + "\n"

    # every resource the fix declares, and the file it names
    files = {}
    for block in re.finditer(r"^\[(Resource\w*)\]\n((?:(?!\[)[^\n]*\n)*)", text, re.M):
        got = re.search(r"^\s*filename\s*=\s*(.+?)\s*$", block.group(2), re.M)
        if (got):
            files[block.group(1).lower()] = got.group(1)

    print("=== %s ===" % name)
    for block in re.finditer(r"^\[(CommandList\w*Component(\d+)Textures\w*ChisaRemapFix)\]\n"
                             r"((?:(?!\[)[^\n]*\n)*)", text, re.M):
        slot = block.group(2)
        binds = re.findall(r"^\s*(ps-t\d)\s*=\s*(\S+)", block.group(3), re.M)
        if (not binds):
            continue
        cells = []
        for reg, res in binds:
            path = files.get(res.lower(), "")
            if (not path):
                where = "?"
            elif ("Mod Downloads" in path or "download" in path.lower()):
                where = "GAME (mod has none)"
            else:
                where = pathlib.Path(path.replace("\\", "/")).name
            cells.append("%s=%s" % (reg, where))
        print("  component %-2s  %s" % (slot, "  ".join(cells)))
    print()
