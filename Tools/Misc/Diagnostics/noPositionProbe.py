r"""A mod that declares NO `[ResourcePositionBuffer]`: does the fix say so, or blame a missing file?

All 82 WWMI character `.ini` files in this corpus declare one, so the "the mod declared none" path is
unreachable from real data -- and a guard that has never run is a guard nobody has checked. This
copies a mod, deletes that one section, and reads the message.

Before: `positionFile_` started at the literal `Meshes/Position.buf`, so the run reported
    "its Position.buf is missing or too short to give a vertex count"
which sends the reader looking for a file the mod never named.

After: the member starts empty and `positionBufPath()` says what is actually wrong.

  py -3 noPositionProbe.py <mod folder name>
"""
import os
import pathlib
import re
import shutil
import subprocess
import sys

Launcher = (r"C:\Users\AlexX\Documents\Games\Mods\XXMI-Launcher-Portable-v1.8.5"
            r"\WWMI\Mods\FixRaidenBoss7.py")
Wwmi = pathlib.Path(r"C:\Users\AlexX\Documents\Games\Mods\XXMI-Launcher-Portable-v1.8.5\WWMI")
Work = pathlib.Path(__file__).parent / "noPositionProbe"

mod = sys.argv[1] if len(sys.argv) > 1 else "Chisa3"
sys.stdout.reconfigure(errors = "replace")

source = Wwmi / mod
if not source.is_dir():
    source = Wwmi / "Mods" / mod

assert source.is_dir(), "no such mod: " + mod

if Work.is_dir():
    shutil.rmtree(Work)

Work.mkdir(parents = True)
dest = Work / mod
shutil.copytree(source, dest)

# drop the one section, in every .ini that has it
dropped = 0
for ini in dest.rglob("*.ini"):
    raw = ini.read_bytes()
    crlf = b"\r\n" in raw
    text = raw.replace(b"\r\n", b"\n").decode("utf-8", errors = "replace")

    out, skipping = [], False
    for line in text.split("\n"):
        head = re.match(r"\s*\[([^\]]+)\]", line)
        if head:
            skipping = head.group(1).strip().lower() == "resourcepositionbuffer"
            if skipping:
                dropped += 1

        if not skipping:
            out.append(line)

    blob = "\n".join(out).encode("utf-8")
    ini.write_bytes(blob.replace(b"\n", b"\r\n") if crlf else blob)

assert dropped, "{} declares no [ResourcePositionBuffer] to drop -- pick another mod".format(mod)
print("dropped [ResourcePositionBuffer] from {} section(s)\n".format(dropped))

run = subprocess.run(["py", "-3", Launcher, "--src", str(dest), "--download", "disabled"],
                     capture_output = True, text = True, errors = "replace")
out = run.stdout + run.stderr

said = [l.strip() for l in out.split("\n") if "declares no" in l or "cannot fix" in l]
print("=== what the fix said ({}) ===".format(len(said)))
for line in said[:6]:
    print("   " + line)

if not said:
    print("   NOTHING -- the guard did not fire. The last 12 lines:")
    for line in [l for l in out.split("\n") if l.strip()][-12:]:
        print("      " + line.rstrip())
