#
# ===== wwmiRemapLatch =====
#
# Does a fixed WWMI mod remap the merged skeleton ONCE A FRAME? On a target past 256 merged bones the
# fix writes its own skeleton remap (`CommandListRemapMergedSkeleton<Fix>`), and it must run at the
# frame's first fix section, before any of them merges, so every draw of the frame binds one skeleton.
# Run at the top of each slot's merge list instead, it reads a merge buffer whose windows differ in
# age, and in motion the parts of one body are drawn a frame apart -- a dark shell trailing her that a
# still never shows (Lynae1, 2026-10-06).
#
#   py -3 wwmiRemapLatch.py <fixed mod folder> [...]
#
# Checks, per .ini that carries the remap: every TextureOverride section that runs a fix merge list
# (`CommandListMergeSlot<N>...` or `CommandListMergeWindow...`) runs the remap exactly once, under
# `if $object_detected == 0`, before it sets `$object_detected = 1`; and no merge list runs it. A fix
# for a host whose [Present] does not reset `$object_detected` keeps the per-list remap on purpose,
# and this reports such a file as NOT LATCHED -- read the host's [Present] before calling it a fault.
#
# Exit code 0 only when something was checked and nothing failed.
#

import glob
import os
import re
import sys


def main():
    bad = 0
    checked = 0
    for folder in sys.argv[1:]:
        for path in glob.glob(os.path.join(folder, "**", "*.ini"), recursive = True):
            if ("BKUP" in os.path.basename(path)):
                continue
            text = open(path, encoding = "utf-8", errors = "replace").read()
            if ("CommandListRemapMergedSkeleton" not in text):
                continue

            for m in re.finditer(r"^\[([^\]]+)\]\n(.*?)(?=^\[|\Z)", text, re.S | re.M):
                name, body = m.groups()
                lines = body.splitlines()
                remaps = [i for i, l in enumerate(lines) if re.match(r"\s*run = CommandListRemapMergedSkeleton", l)]
                if (name.startswith("CommandListMergeSlot") and remaps):
                    print(f"  remap inside a merge list: {os.path.relpath(path, folder)} [{name}]")
                    bad += 1

                merges = any(re.match(r"\s*run = CommandListMerge(Slot\d+|Window)\w*RemapFix\s*$", l) for l in lines)
                if (not merges or not name.startswith("TextureOverride")):
                    continue

                checked += 1
                detected = [i for i, l in enumerate(lines) if l == "$object_detected = 1"]
                ok = (len(remaps) == 1 and detected and remaps[0] < detected[0]
                      and lines[remaps[0] - 1].strip() == "if $object_detected == 0")
                if (not ok):
                    print(f"  NOT LATCHED: {os.path.relpath(path, folder)} [{name}]")
                    bad += 1

    if (checked == 0):
        print("NOTHING WAS CHECKED -- no section here runs a fix merge list")
        sys.exit(1)

    print(f"{checked} merging section(s) checked, {bad} problem(s)")
    sys.exit(1 if bad else 0)


if (__name__ == "__main__"):
    main()
