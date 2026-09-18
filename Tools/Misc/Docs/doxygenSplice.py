"""Splices chosen compounds (a class, a struct, a header) of a freshly generated Doxygen XML tree into
the tracked core/xml, leaving every other file -- and every other compound in index.xml -- exactly
as it is. The Doxygen counterpart of pyiSplice.py.

    python doxygenSplice.py --run <scratch dir>                        run Doxygen into <scratch dir>/xml
    python doxygenSplice.py <generated xml dir>                        list the compounds that differ
    python doxygenSplice.py <generated xml dir> --show NAME ...        which files / index blocks NAME touches
    python doxygenSplice.py <generated xml dir> --apply NAME ...       splice them in

NAME is a compound's name as Doxygen gives it (`AGRemapCore::GraphInherit`, `GraphInherit.h`) or its
refid (`class_a_g_remap_core_1_1_graph_inherit`). Naming a class does not bring its header along --
name both when the header changed too.

Why not copy the regenerated tree, or the regenerated index.xml: the committed core/xml lags the
headers (every session that did not regenerate left drift behind), so a whole regeneration is a
diff of hundreds of files that are not yours, and a new index.xml names compounds whose files you
did not copy -- Breathe then dies with `Cannot find file` on a class you never touched. Splicing
only your compounds' `<compound>` blocks into the TRACKED index keeps it consistent with the files
that are actually there.

--run never touches core/xml: it pipes the Doxyfile to `doxygen -` with OUTPUT_DIRECTORY replaced,
so the Doxyfile's relative INPUT still resolves (it runs from core/), and writes doxygen.log beside
the output. It prints the warnings that name a header, since a run's exit code says little
(Documentation guide). Use Doxygen 1.17.0, the version the committed XML is stamped with.

A compound only in the generated tree (a new class) is inserted into index.xml before the first
compound that follows it in the generated index and exists in the tracked one. Line endings of the
tracked files are kept (CRLF).
"""
import argparse
import os
import re
import subprocess
import sys

REPO = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", ".."))
CORE = os.path.join(REPO, "Anime Game Remap (for all users)", "api", "src", "cpp", "core")
TRACKED = os.path.join(CORE, "xml")

COMPOUND = re.compile(r'  <compound refid="([^"]+)" kind="([^"]+)"><name>([^<]*)</name>.*?</compound>\n', re.S)


def read(path):
    with open(path, "rb") as f:
        data = f.read()
    return data.decode("utf-8").replace("\r\n", "\n"), b"\r\n" in data


def write(path, txt, crlf):
    data = (txt.replace("\n", "\r\n") if crlf else txt).encode("utf-8")
    with open(path, "wb") as f:
        f.write(data)


def compounds(indexTxt):
    """refid -> (kind, name, block), in index order"""
    return {m.group(1): (m.group(2), m.group(3), m.group(0)) for m in COMPOUND.finditer(indexTxt)}


def resolve(names, genCompounds):
    result = []
    for name in names:
        matches = [refid for refid, (kind, cname, block) in genCompounds.items() if name in (refid, cname)]
        if not matches:
            sys.exit(f"no compound named {name!r} in the generated index")
        result += matches
    return result


def run(scratch):
    os.makedirs(scratch, exist_ok=True)
    doxyfile, _ = read(os.path.join(CORE, "Doxyfile"))
    doxyfile = re.sub(r"(?m)^OUTPUT_DIRECTORY\s*=.*$", "", doxyfile) + f"\nOUTPUT_DIRECTORY = {scratch}\n"
    logPath = os.path.join(scratch, "doxygen.log")
    with open(logPath, "w", encoding="utf-8") as log:
        proc = subprocess.run(["doxygen", "-"], input=doxyfile, text=True, cwd=CORE, stdout=log, stderr=subprocess.STDOUT)

    index = os.path.join(scratch, "xml", "index.xml")
    print(f"doxygen exit {proc.returncode}, log {logPath}")
    print(f"index.xml {'exists' if os.path.isfile(index) else 'MISSING -- the run failed'}")
    # Doxygen's progress text is interleaved into the same lines ("Generating docs for compound
    # AGReE:/.../X.h:12: warning: ..."), so each warning is cut out from its path onwards.
    warningPattern = re.compile(r"([A-Za-z]:[/\\]|/)[^:]*\.(?:h|tpp):\d+: warning: .*")
    with open(logPath, encoding="utf-8", errors="replace") as log:
        warnings = []
        for line in log:
            m = warningPattern.search(line)
            if m:
                warnings.append(os.path.relpath(m.group(0), CORE) if m.group(0).startswith(CORE.replace("\\", "/")) else m.group(0))
    print(f"{len(warnings)} warnings naming a header (grep the log for yours):")
    for line in warnings[:20]:
        print("  " + line)


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("generated", help="the generated xml folder, or with --run the scratch folder to generate into")
    parser.add_argument("--run", action="store_true", help="run Doxygen into the given scratch folder")
    parser.add_argument("--show", nargs="+", metavar="NAME")
    parser.add_argument("--apply", nargs="+", metavar="NAME")
    parser.add_argument("--tracked", default=TRACKED, help="the xml folder to splice into (default: the repo's core/xml)")
    args = parser.parse_args()

    if args.run:
        run(os.path.abspath(args.generated))
        return

    genIndex, _ = read(os.path.join(args.generated, "index.xml"))
    trackedIndex, indexCrlf = read(os.path.join(args.tracked, "index.xml"))
    genCompounds = compounds(genIndex)
    trackedCompounds = compounds(trackedIndex)

    def fileDiffers(refid):
        genPath = os.path.join(args.generated, refid + ".xml")
        trackedPath = os.path.join(args.tracked, refid + ".xml")
        if not os.path.isfile(genPath):
            return False
        return not os.path.isfile(trackedPath) or read(genPath)[0] != read(trackedPath)[0]

    if not args.show and not args.apply:
        differ = [refid for refid in genCompounds if refid in trackedCompounds and
                  (fileDiffers(refid) or genCompounds[refid][2] != trackedCompounds[refid][2])]
        onlyGen = [refid for refid in genCompounds if refid not in trackedCompounds]
        onlyTracked = [refid for refid in trackedCompounds if refid not in genCompounds]
        print(f"{len(differ)} compounds differ (most are other sessions' drift -- splice only yours):")
        for refid in differ:
            print(f"  {genCompounds[refid][1]}  [{refid}]")
        print(f"only in generated (new): {[genCompounds[r][1] for r in onlyGen]}")
        print(f"only in tracked (gone from the headers): {[trackedCompounds[r][1] for r in onlyTracked]}")
        return

    refids = resolve(args.show or args.apply, genCompounds)
    for refid in refids:
        kind, name, block = genCompounds[refid]
        state = "new" if refid not in trackedCompounds else ("differs" if block != trackedCompounds[refid][2] else "same")
        print(f"{name} [{kind}]: {refid}.xml {'differs' if fileDiffers(refid) else 'same'}, index block {state}")

    if not args.apply:
        return

    genOrder = list(genCompounds)
    for refid in refids:
        genPath = os.path.join(args.generated, refid + ".xml")
        trackedPath = os.path.join(args.tracked, refid + ".xml")
        if os.path.isfile(genPath):
            crlf = read(trackedPath)[1] if os.path.isfile(trackedPath) else indexCrlf
            write(trackedPath, read(genPath)[0], crlf)

        block = genCompounds[refid][2]
        if refid in trackedCompounds:
            trackedIndex = trackedIndex.replace(trackedCompounds[refid][2], block)
        else:
            after = [r for r in genOrder[genOrder.index(refid) + 1:] if r in trackedCompounds]
            anchor = trackedCompounds[after[0]][2] if after else "</doxygenindex>"
            assert trackedIndex.count(anchor) == 1, f"insertion anchor for {refid} is not unique"
            trackedIndex = trackedIndex.replace(anchor, block + anchor)
        trackedCompounds = compounds(trackedIndex)

    write(os.path.join(args.tracked, "index.xml"), trackedIndex, indexCrlf)
    print(f"spliced {len(refids)} compound(s) into {args.tracked}")


if __name__ == "__main__":
    main()
