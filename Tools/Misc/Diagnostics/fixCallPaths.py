"""Does any EXECUTION PATH call a fix library twice over ONE SET OF BINDINGS?

    python fixCallPaths.py <fixed folder> [<fixed folder> ...]

``NNFix`` and ``ORFix`` are **involutions**. They do not set the ``ps-t`` registers, they re-slot
the ones already bound -- ``CommandListReferenceNoNormal`` reads the diffuse out of ``ps-t0`` and
the light map out of ``ps-t1``, and ``CommandListLDX`` writes them back the other way round, while
``CommandListClear`` nulls only the ``Resource*`` refs and leaves the swap standing. Call one twice
over one set of bindings and you are back where you started, with the LIGHT MAP sampled as the
albedo: the model renders **flat green**.

Three units, and only the third is the real invariant:

  * **per section** is what is tempting to write and it OVER-reports. A chain of ``if`` /
    ``else if`` / ``else`` branches is exclusive -- exactly one branch runs -- so two calls in two
    branches of one chain is one call per path, which is right.
  * **per path** is closer, and separate ``if ... endif`` blocks are INDEPENDENT and do stack,
    which is the shape that renders green. But it over-reports too, on this library's own output.
  * **per path per BINDING GENERATION** is the invariant. A call is undone by the next call only
    while the registers underneath it have not moved, so **re-binding a ``ps-t`` resets the
    count**. Where several source objects merge onto one target and need different textures, the
    fix emits one ``ps-t0`` / ``ps-t1`` / ``run = NNFix`` / ``drawindexed`` block per member -- two
    calls on one path, both correct, because the second acts on registers the first never saw.

So the worst path through a section is, since the last rebinding::

    top-level calls  +  sum over chains of (max over that chain's branches)

Two counting mistakes this makes it easy to avoid, both made here first. GIMI spells the chain
``if`` / ``else if`` / ``endif`` -- **two words, no ``elif``** -- so a tally looking for ``elif``
reports every chain as a single branch. And the per-SECTION count reports every exclusive chain as
a violation.

A unit test cannot see this: it needs real mods with real toggles. Fix scratch COPIES of a library
(``runCompiled.py`` per folder) and point this at the output.

Exits non-zero on any violation, and on finding no fixed ``.ini`` at all -- a missing input is not
a pass. Needs nothing but the ``.ini`` files -- no API, no build."""
import os
import re
import sys

Section = re.compile(r"^\[(.+)\]$")
FixCall = re.compile(r"^run\s*=.*[\\/](NNFix|ORFix)\s*$", re.IGNORECASE)
Binding = re.compile(r"^ps-t\d+\s*=", re.IGNORECASE)
Marker = "Remap ---"

worst = []
sections = 0
files = 0


def worstPath(block):
    """Yields (section name, calls on the worst path since the last rebinding)."""
    name, top, chains, chain, high = None, 0, [], None, 0

    def closeChain():
        nonlocal chain
        if (chain is not None):
            chains.append(max(chain))
            chain = None

    def mark():
        """The running worst, kept because a rebinding clears the counters after it."""
        nonlocal high
        high = max(high, top + sum(chains) + (max(chain) if chain else 0))

    # The sentinel must MATCH Section -- "[]" does not (the pattern needs a name), which silently
    # dropped the last section of every file and so passed a single-section test case outright.
    for line in block.splitlines() + ["[end of block]"]:
        s = line.strip()
        match = Section.match(s)
        if (match is not None):
            closeChain()
            mark()
            if (name is not None and name.lower().startswith("textureoverride")):
                yield name, high
            name, top, chains, chain, high = match.group(1), 0, [], None, 0
            continue

        low = s.lower()
        if (low.startswith("if ")):
            closeChain()
            chain = [0]                                # a new independent block
        elif (low.startswith("else if ") or low.startswith("elif ") or low == "else"):
            if (chain is not None):
                chain.append(0)                        # another BRANCH of the same chain
        elif (low == "endif"):
            closeChain()
        elif (Binding.match(s)):
            # A new binding generation: everything before it can no longer be undone by a later
            # call, so bank the worst so far and start counting again.
            mark()
            if (chain is not None):
                chain[-1] = 0
            else:
                top, chains = 0, []
        elif (FixCall.match(s)):
            if (chain is not None):
                chain[-1] += 1
            else:
                top += 1


for target in sys.argv[1:]:
    for root, _, names in os.walk(target):
        for f in names:
            if (not f.lower().endswith(".ini")):
                continue

            path = os.path.join(root, f)
            try:
                text = open(path, encoding = "utf-8", errors = "replace").read()
            except OSError as error:
                print(f"  UNREADABLE {path}: {error}")
                continue

            if (Marker not in text):
                continue
            files += 1

            for name, calls in worstPath(text.replace("\r\n", "\n").split(Marker, 1)[1]):
                sections += 1
                if (calls > 1):
                    worst.append((os.path.relpath(path, target), name, calls))

print(f"remapped sections checked : {sections}   across {files} fixed .ini files")
print(f"sections where SOME PATH calls a fix library twice over one set of bindings: {len(worst)}")
for path, name, calls in worst[:40]:
    print(f"  {calls} calls on one path   [{name}]   {path}")
if (len(worst) > 40):
    print(f"  ... and {len(worst) - 40} more")

if (not files):
    print("\nNO FIXED .ini FILES FOUND -- nothing was checked, which is not a pass")
    sys.exit(1)
sys.exit(1 if worst else 0)
