"""Runs the Unit Tester and writes the ID of every failure and error, one per line, so two runs can be
DIFFED by identity rather than compared by count.

    python unitTestIds.py <out.txt>                                   the whole suite, shared package
    python unitTestIds.py <out.txt> --only GIMIComponentBuilders ...  only test classes whose name contains a filter
    python unitTestIds.py <out.txt> --api /tmp/oldApi/api             against ANOTHER copy of the API

--api is how a new test is proved to FAIL on the build before your fix without copying an old .so
over the shared package (another agent may be using it, and cp over a loaded .so crashes whatever
has it mapped). Save the old module before rebuilding, then build a Linux-side copy of the package
around it:

    cp <pkg>/core.cpython-*.so ~/old_core.so                           # BEFORE linuxBuild.sh
    mkdir -p /tmp/oldApi/api/src/py
    rsync -a --exclude __pycache__ --exclude 'core.cpython-*.so' <pkg> /tmp/oldApi/api/src/py/
    cp ~/old_core.so /tmp/oldApi/api/src/py/FixRaidenBoss2/core.cpython-310-x86_64-linux-gnu.so

The copy is imported BEFORE any test module, because every test module runs
`sys.path.insert(1, <shared API>)` on import: a path merely put ahead of it at index 0 loses to that,
and the run silently tests the shared package (the first line printed says which one was loaded --
read it). The full log is written beside <out.txt> as <out.txt>.log.
"""
import argparse, os, sys, unittest

REPO = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", ".."))
UNIT_TESTER = os.path.join(REPO, "Testing", "Unit Tester")

parser = argparse.ArgumentParser(description = __doc__, formatter_class = argparse.RawDescriptionHelpFormatter)
parser.add_argument("out", help = "the file to write the failure / error IDs to")
parser.add_argument("--api", help = "the api/ folder of another copy of the API to test instead of the shared package")
parser.add_argument("--only", nargs = "+", default = [], help = "run only test classes whose name contains one of these")
args = parser.parse_args()
out = os.path.abspath(args.out)

if args.api:
    root = os.path.abspath(args.api)
    sys.path.insert(0, root)
    import src.py.FixRaidenBoss2
    sys.path.remove(root)

os.chdir(UNIT_TESTER)
sys.path.insert(0, UNIT_TESTER)
import UnitTester.Tests as T
import src.py.FixRaidenBoss2 as FRB
print("FixRaidenBoss2 from:", os.path.dirname(FRB.__file__), flush = True)

suite = unittest.TestSuite()
loader = unittest.TestLoader()
for name in T.__all__:
    obj = getattr(T, name)
    if not (isinstance(obj, type) and issubclass(obj, unittest.TestCase)):
        continue
    if args.only and not any(f in name for f in args.only):
        continue
    suite.addTests(loader.loadTestsFromTestCase(obj))

with open(out + ".log", "w", encoding = "utf-8") as log:
    result = unittest.TextTestRunner(stream = log, verbosity = 1).run(suite)

summary = f"ran {result.testsRun} failures {len(result.failures)} errors {len(result.errors)}"
with open(out, "w", encoding = "utf-8") as f:
    f.write(summary + "\n")
    for kind, items in (("FAIL", result.failures), ("ERROR", result.errors)):
        for test, _ in items:
            f.write(f"{kind} {test.id()}\n")
print(summary)
