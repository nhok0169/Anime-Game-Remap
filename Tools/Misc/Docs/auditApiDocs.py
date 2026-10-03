"""
Audits the Sphinx docs the way a NEW API USER would read them, in three checks:

  exports   every public name ``FixRaidenBoss2`` exports has an entry in ``Docs/src/api.rst``
            (imported through ``Docs/src/extensions/compiledStubs.py``, exactly as Read the Docs does,
            so it needs no compiled build)
  rendered  the RENDERED pages of a Sphinx build carry no date stamps and no history of the old
            pure-Python library ("the pure-Python original", "a port of", "the old script", ...).
            Doc comments and docstrings render, so a work log written into one ends up on the site
  links     every link into this repository from ``Docs/src/*.rst`` (and from the built pages, when a
            build is given) names a file or folder that exists in this checkout

Usage:
    python auditApiDocs.py                     # exports + links over the .rst sources
    python auditApiDocs.py --html <build dir>  # also the rendered-text and rendered-link checks

Build the docs first with (from Docs/):
    AGREMAP_DOCS_STUBS=force python -m sphinx -b html -E --keep-going src <build dir>

Exit code: 0 when every check passes, 1 otherwise. A "pure-Python" hit is not automatically wrong -- it is
fine when it names a Python class that still exists (eg. ``IniNamingTools``, ``BufFile``); read each one.
"""

import argparse
import enum
import glob
import html
import inspect
import os
import re
import sys
import urllib.parse

REPO = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))
DOCS_SRC = os.path.join(REPO, "Docs", "src")
API_SRC = os.path.join(REPO, "Anime Game Remap (for all users)", "api", "src")

# names exported by FixRaidenBoss2 that deliberately have no entry of their own
SKIP_EXPORTS = {"Path", "HashData", "IndexData"}

# "port" alone is not history here: a "ported mod" is a mod moved onto another character
HISTORY = re.compile(r"20\d\d-\d\d-\d\d|pure[- ]?python (original|implementation)|python original|"
                     r"\bC\+\+ port\b|\bport(ed)? (of|from) the (pure|python|original)|old script|was deleted|"
                     r"since removed|FixRaidenBoss[0-9]\.py|\bstill[- ]pure[- ]python", re.IGNORECASE)
REPO_LINK = re.compile(r"https://(?:github\.com|raw\.githubusercontent\.com)/nhok0169/Anime-Game-Remap/"
                       r"(?:blob/|tree/|raw/|refs/heads/)?([^/\s]+)/([^\s<>`\"]*)")
NOT_PATHS = {"releases", "actions", "issues", "pulls", "wiki"}


def checkExports() -> list:
    sys.path.insert(0, os.path.join(DOCS_SRC, "extensions"))
    sys.path.insert(0, os.path.join(API_SRC, "py"))
    os.environ["AGREMAP_DOCS_STUBS"] = "force"
    import compiledStubs
    list(compiledStubs.install(os.path.join(API_SRC, "py", "FixRaidenBoss2"), os.path.join(API_SRC, "cy", "src")))
    import FixRaidenBoss2

    rst = open(os.path.join(DOCS_SRC, "api.rst"), encoding="utf-8").read()
    documented = set(re.findall(r"^\.\. auto(?:class|function):: FixRaidenBoss2\.(\w+)", rst, re.M))
    public = {n for n in dir(FixRaidenBoss2) if not n.startswith("_") and callable(getattr(FixRaidenBoss2, n))}

    problems = [f"exported but not in api.rst: {n}" for n in sorted(public - documented - SKIP_EXPORTS)]
    problems += [f"in api.rst but not exported: {n}" for n in sorted(documented - public)]

    # autodoc reads an enum member's '.value', which CONSTRUCTS a DeferredEnum's object: document those without ':members:'
    for n in sorted(documented):
        obj = getattr(FixRaidenBoss2, n, None)
        if (inspect.isclass(obj) and issubclass(obj, FixRaidenBoss2.DeferredEnum) and obj is not FixRaidenBoss2.DeferredEnum
                and re.search(rf"^\.\. autoclass:: FixRaidenBoss2\.{n}\n    :members:", rst, re.M)):
            problems.append(f"DeferredEnum documented with ':members:' (autodoc would build its values): {n}")
    return problems


def repoPath(url: str):
    while url.endswith(")") and url.count("(") < url.count(")"):
        url = url[:-1]
    m = REPO_LINK.match(url.rstrip(".,;"))
    if m is None or m.group(1) in NOT_PATHS:
        return None
    return urllib.parse.unquote(m.group(2)).split("#")[0].split("?")[0].rstrip("/")


def checkLinks(texts: dict) -> list:
    problems = []
    for name, text in texts.items():
        for m in re.finditer(r"https://[^\s<>`\"]+", text):
            path = repoPath(html.unescape(m.group(0)))
            if path and not os.path.exists(os.path.join(REPO, path)):
                problems.append(f"{name}: link to a path that does not exist: {path}")
    return sorted(set(problems))


def visibleText(page: str) -> str:
    page = re.sub(r"<(script|style)\b.*?</\1>", " ", page, flags=re.S)
    return html.unescape(re.sub(r"<[^>]+>", "", re.sub(r"<(p|div|dd|dt|li|br|tr|h\d|pre)\b", "\n<", page)))


def checkRendered(buildDir: str) -> list:
    problems = []
    ids = {}
    pages = {os.path.basename(p): open(p, encoding="utf-8").read() for p in glob.glob(os.path.join(buildDir, "*.html"))}
    for name, page in pages.items():
        ids[name] = set(re.findall(r'\sid="([^"]+)"', page))
    for name, page in pages.items():
        if name in ("genindex.html", "search.html"):
            continue
        for line in visibleText(page).splitlines():
            if HISTORY.search(line):
                problems.append(f"{name}: {line.strip()[:160]}")
        for href in re.findall(r'href="(#[^"]+)"', page):
            if href[1:] not in ids[name]:
                problems.append(f"{name}: in-page link to a missing anchor: {href}")
    problems += checkLinks({f"{n} (rendered)": p for n, p in pages.items()})
    return problems


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--html", help="a Sphinx HTML build of Docs/src to check the rendered pages of")
    args = parser.parse_args()
    if hasattr(sys.stdout, "reconfigure"):
        sys.stdout.reconfigure(encoding="utf-8")

    sources = {os.path.basename(p): open(p, encoding="utf-8").read() for p in glob.glob(os.path.join(DOCS_SRC, "*.rst"))}
    results = {"exports": checkExports(), "links": checkLinks(sources)}
    if args.html:
        results["rendered"] = checkRendered(args.html)

    failed = False
    for check, problems in results.items():
        print(f"== {check}: {len(problems)} problem(s)")
        for p in problems:
            print(f"   {p}")
        failed |= bool(problems)
    if not args.html:
        print("== rendered: SKIPPED (pass --html <build dir> to check the rendered pages)")
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
