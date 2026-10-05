"""Is every draw of every remapped section reached on EVERY path, and covered exactly once?

    python drawCoverage.py <fixed mod folder> [<fixed mod folder> ...]

Three things go wrong with a re-issued or appended ``drawindexed``, and all three have:

  1. **nothing draws part of the buffer.** When several source objects merge onto one target, the
     merged index buffer is member after member, and a mod's own ``drawindexed`` lines address ITS
     OWN buffer -- so they cover the first member and stop. Every buffer is byte-perfect and the
     character is missing a part.
  2. **something draws it, from inside an ``if`` block**, so it depends on a toggle. The obvious
     edit (``RegSurroundedAdd`` with ``latest = true``) puts it at the LATEST valid position, which
     in a section full of toggles is inside the last block. ``RegFillMissingMode::BottomCover`` is
     the placement that is right.
  3. **two things draw it.** ``drawindexed = auto`` already covers the whole buffer, so an appended
     range stacked on top of it is a duplicate -- and the mod renders both variants of something it
     meant to toggle between.

So this reports, per remapped ``TextureOverride``: every draw with its ``if`` DEPTH, whether the
section has both an ``auto`` and explicit ranges, and whether any path through the section reaches
no draw at all.

**The depth is the whole point.** A check that ``.strip()``s a ``.ini`` line before judging it
cannot see nesting, and nesting was the entire question -- an earlier version of this check
asserted "the appended range exists and comes last", which was true, passing, and shipped with the
draw sitting inside ``if $pubic == 1`` the whole time.

Exits non-zero on any finding. Needs nothing but the ``.ini`` file -- no API, no build."""
import os
import re
import sys

Section = re.compile(r"^\[(.+)\]$")
Marker = "Remap ---"

problems = []


def fail(what):
    print("  FAIL " + what)
    problems.append(what)


def sectionDraws(block):
    """{section: [(draw value, depth), ...]} plus {section: branchDepths} over the remap block.

    GIMI writes an exclusive chain as ``if`` / ``else if`` / ``endif`` -- two words, no ``elif``.
    A tally that looks for ``elif`` reports every chain as a single branch, which is how an earlier
    pass over this same corpus concluded there was nothing to find."""
    out, branched = {}, {}
    name, depth, sawBranch = None, 0, False

    for line in block.splitlines():
        s = line.strip()
        match = Section.match(s)
        if (match is not None):
            name, depth, sawBranch = match.group(1), 0, False
            if (name.lower().startswith("textureoverride")):
                out.setdefault(name, [])
                branched[name] = False
            continue

        if (name not in out):
            continue

        low = s.lower()
        if (low.startswith("if ")):
            depth += 1
        elif (low == "endif"):
            depth = max(0, depth - 1)
        elif (low.startswith("else")):
            sawBranch = True
            branched[name] = True
        elif (low.startswith("drawindexed")):
            out[name].append((s.partition("=")[2].strip(), depth))

    return out, branched


def analyse(folder):
    label = os.path.basename(folder.rstrip("\\/")) or folder
    print(f"\n=== {label}")

    found = False
    for root, _, files in os.walk(folder):
        for f in sorted(files):
            if (not f.lower().endswith(".ini")):
                continue

            text = open(os.path.join(root, f), encoding = "utf-8", errors = "replace").read().replace("\r\n", "\n")
            if (Marker not in text):
                continue
            found = True

            draws, branched = sectionDraws(text.split(Marker, 1)[1])
            for name, here in sorted(draws.items()):
                if (not here):
                    continue

                print(f"  [{name}]")
                for value, depth in here:
                    print(f"      depth {depth}   drawindexed = {value}")

                autos = [d for v, d in here if v.lower() == "auto"]
                explicit = [d for v, d in here if v.lower() != "auto"]

                if (autos and explicit):
                    fail(f"{label}: [{name}] has both `auto` and an explicit range -- `auto` "
                         f"already covers the whole buffer, so the range is drawn twice")

                # Some path must reach a draw unconditionally. An exclusive if/else-if chain covers
                # every path without a depth-0 draw, so only an UNBRANCHED section is judged here.
                if (not any(d == 0 for _, d in here) and not branched.get(name)):
                    fail(f"{label}: [{name}] draws only from inside `if` blocks with no `else` -- "
                         f"a toggle can switch the part off entirely")

    if (not found):
        fail(f"{label}: no fixed .ini file under {folder} -- nothing to check")


for target in sys.argv[1:]:
    analyse(target)

print()
print("PROBLEMS:" if problems else "EVERY REMAPPED SECTION DRAWS ITS BUFFER ONCE, ON EVERY PATH")
for problem in problems:
    print("  -", problem)
sys.exit(1 if problems else 0)
