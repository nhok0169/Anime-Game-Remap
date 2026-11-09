"""fuzzRun.py <fuzz root> [--jobs N] [--list <script>]: run a list script (default Tools/Misc/Prototypes/editList.py, its data
from AGREMAP_LIST_PAIR="Source,Target") over every folder iniFuzz.py made, each in its own process, and sort the outcomes:

  CRASH      the process died (a segfault, an uncaught exception)
  TIMEOUT    it ran past 90s
  SKIPPED    the fix raised; RemapService filed it under skipped, with its message
  WROTE      the run wrote a fix block into the folder
  NOTHING    the run wrote no fix

Writes fuzzRun.txt in the root, prints each result as it lands and a summary of the distinct failure messages. Run with
--jobs 1 for timings: fuzzed paths name files outside a folder (`..\\`, an absolute one), so parallel runs share files."""
import argparse
import collections
import concurrent.futures
import os
import re
import subprocess
import sys

LIST = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "Prototypes", "editList.py")


def runOne(folder):
    try:
        # stdin closed: on some inputs the run asks for a key, and an inherited stdin waits for one forever
        done = subprocess.run([sys.executable, LIST, folder], capture_output = True, text = True, timeout = 90,
                              encoding = "utf-8", errors = "replace", stdin = subprocess.DEVNULL)
    except subprocess.TimeoutExpired:
        return folder, "TIMEOUT", ""
    out = done.stdout + done.stderr
    if done.returncode != 0:
        tail = out.strip().splitlines()[-1:] or ["exit %d" % done.returncode]
        return folder, "CRASH", "exit %d: %s" % (done.returncode, tail[0])
    skipped = [line for line in out.splitlines() if line.startswith("SKIPPED")]
    if skipped:
        return folder, "SKIPPED", skipped[0].split(" ", 2)[-1]
    wrote = False
    for base, _, names in os.walk(folder):
        for name in names:
            if name.lower().endswith(".ini"):
                with open(os.path.join(base, name), encoding = "utf-8", errors = "replace") as f:
                    wrote = wrote or "RemapFix" in f.read()
    return folder, "WROTE" if wrote else "NOTHING", ""


def normalise(message):
    """A message with its numbers and paths folded, so one cause counts once"""
    message = re.sub(r"[A-Za-z]:\\[^ ,'\"]*", "<path>", message)
    return re.sub(r"\d+", "N", message)[:220]


def main():
    global LIST
    args = argparse.ArgumentParser()
    args.add_argument("root")
    args.add_argument("--jobs", type = int, default = 1)
    args.add_argument("--list", default = None)
    a = args.parse_args()
    LIST = a.list or LIST
    folders = sorted(os.path.join(a.root, name) for name in os.listdir(a.root) if name.startswith("fuzz"))
    results = []
    with concurrent.futures.ThreadPoolExecutor(a.jobs) as pool:
        for result in pool.map(runOne, folders):
            results.append(result)
            print("RESULT", os.path.basename(result[0]), result[1], result[2][:200], flush = True)
    with open(os.path.join(a.root, "fuzzRun.txt"), "w", encoding = "utf-8") as f:
        for folder, kind, message in results:
            f.write("%s\t%s\t%s\n" % (os.path.basename(folder), kind, message))
    print(collections.Counter(kind for _, kind, _ in results))
    causes = collections.defaultdict(list)
    for folder, kind, message in results:
        if kind in ("CRASH", "SKIPPED", "TIMEOUT"):
            causes[(kind, normalise(message))].append(os.path.basename(folder))
    for (kind, message), where in sorted(causes.items(), key = lambda item: -len(item[1])):
        print("%4d  %-8s %s   e.g. %s" % (len(where), kind, message, where[0]))


if __name__ == "__main__":
    main()
