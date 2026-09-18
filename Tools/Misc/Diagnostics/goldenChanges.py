"""What each Integration Tester test PRODUCED: the files its expected_* golden adds, removes or
changes relative to the inputs it was copied from.

    python goldenChanges.py                 every test
    python goldenChanges.py modFixUndoed    only goldens whose folder name contains a filter

Read goldens this way rather than through `git diff`. Every golden is a full copy of its suite's
inputs, so one fixture edit shows up in every test's tree, while what a test actually DID -- a
RemapBlend.buf written, an .ini left fixed after an undo, a golden that changed nothing at all --
is a handful of lines here. An empty entry is a vacuous test.
"""
import os, filecmp, sys

TESTS = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "..", "Testing", "Integration Tester", "IntegrationTester", "Tests")

def files(root):
    out = set()
    for d, _, fs in os.walk(root):
        for f in fs:
            out.add(os.path.relpath(os.path.join(d, f), root).replace(os.sep, "/"))
    return out

for suite in ["APIDocsTests", "MixedModsTests"]:
    inputs = os.path.join(TESTS, suite, "inputs")
    inFiles = files(inputs)
    for name in sorted(os.listdir(os.path.join(TESTS, suite))):
        if not name.startswith("expected_"):
            continue
        if len(sys.argv) > 1 and not any(a in name for a in sys.argv[1:]):
            continue
        g = os.path.join(TESTS, suite, name)
        gFiles = files(g)
        added = sorted(gFiles - inFiles)
        removed = sorted(inFiles - gFiles)
        changed = sorted(f for f in gFiles & inFiles if not filecmp.cmp(os.path.join(g, f), os.path.join(inputs, f), shallow = False))
        print(f"=== {name}")
        for label, lst in [("+", added), ("-", removed), ("~", changed)]:
            for f in lst:
                print(f"   {label} {f}")
