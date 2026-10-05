r"""Did a remapped section keep a line that UNDOES the remap?

A WWMI mod of a character past 256 bones carries three lines per component::

    ResourceBlendBufferOverride         = ref ...Component<N>
    ResourceMergedSkeletonOverride      = ref ...Component<N>
    ResourceExtraMergedSkeletonOverride = ref ...Component<N>

They point the draw at WWMI's blend remap of the SOURCE. In the mod's OWN sections that is right --
it is how the mod draws on its own character. Copied into a ``...RemapFix`` section they are an
inverse of the whole fix: the two shaders are exact inverses of each other, so the pair feeds the
draw the source's own merged index against the TARGET's skeleton. The components with a blend remap
collapse into a drape while the ones without render correctly, which reads in game as a secondary
jello body under an intact head.

The fix is supposed to strip them, and it does so by writing ``= null``. So inside a remapped
section:

    * ``= null``            the fix CLEARED it -- correct, and what a good run looks like
    * ``= ref <anything>``  CARRIED -- the bug

**The identity mod cannot show this.** It is the one mod whose own sections carry no such line, so
it has none to carry across and renders perfectly while every real mod of the character is broken.
That is how the regression of 2026-10-02 survived: the usual check passed throughout.

Two traps this got wrong before it got it right, both of which report a WORKING build as broken:

    * counting ``= null`` as a surviving line. Those are the fix doing its job, and on a correct
      build there are three of them per mod. The first draft reported "16 surviving override lines"
      over output that was entirely correct.
    * counting the fix's own CONDITION ``if ResourceBlendBufferOverride === null`` as an assignment.
      A loose ``Override\s*=`` match reads its ``==`` as a value and fires on nearly every mod.
      Matching the key EXACTLY, after splitting on the first ``=``, excludes it by construction --
      the key of that line is ``if ResourceBlendBufferOverride``, not the bare name.

``--prove`` copies a fixed folder, puts one carried line back into a remapped section, and requires
the check to FAIL on the copy. A check that has never failed is not a check (Overview habit 34);
this one was written against output that was already correct, so without ``--prove`` it had only
ever printed a pass.

Usage::

    py -3 Tools/Misc/Diagnostics/wwmiCarriedOverrides.py <fixed folder>... [--prove]

Exit 0 only when something was actually checked and every check passed. A folder with NO remapped
sections reports that it checked nothing and is not a pass (Overview habit 66).

Its companions, which ask the other structural questions of a fixed ``.ini`` and are worth running
together on any mod that renders wrong: ``check_dangling.py`` (every ``filename =`` resolves on
disk), ``check_sections.py`` (every referenced section is defined), ``editedButUnbound.py`` (every
texture the fix wrote is bound by a section that draws).
"""
import argparse
import os
import re
import shutil
import sys
import tempfile

#: The three shared-resource overrides WWMI's blend remap binds. Matched as whole keys.
OverrideKeys = ("resourceblendbufferoverride",
                "resourcemergedskeletonoverride",
                "resourceextramergedskeletonoverride")

#: A section this library generated. The fix names every section it writes with this.
RemapMark = "remapfix"

Section = re.compile(r"^\s*\[([^\]]+)\]\s*$")


def iniFiles(root):
    for dirpath, _dirs, files in os.walk(root):
        for name in sorted(files):
            if not name.lower().endswith(".ini"):
                continue
            if name.lower().startswith("disabled") or "BKUP" in name:
                continue

            yield os.path.join(dirpath, name)


def overrideLines(path):
    """(section, key, value, lineNo) for every line assigning one of the three override keys.

    The key is whatever precedes the FIRST `=`, stripped -- so the fix's own
    `if ResourceBlendBufferOverride === null` yields the key `if ResourceBlendBufferOverride`,
    which is not one of the three and is therefore never mistaken for an assignment.
    """
    section = ""
    with open(path, encoding="utf-8", errors="replace") as handle:
        for lineNo, line in enumerate(handle, 1):
            line = line.rstrip("\r\n")

            found = Section.match(line)
            if found:
                section = found.group(1)
                continue

            if line.lstrip().startswith(";") or "=" not in line:
                continue

            key, value = line.split("=", 1)
            if key.strip().lower() in OverrideKeys:
                yield section, key.strip(), value.strip(), lineNo


def check(root, quiet=False):
    """(carried, cleared, own, remappedSections)"""
    carried = []
    cleared = own = remappedSections = 0

    for path in iniFiles(root):
        rel = os.path.relpath(path, root)

        with open(path, encoding="utf-8", errors="replace") as handle:
            for line in handle:
                found = Section.match(line.rstrip("\r\n"))
                if found and RemapMark in found.group(1).lower():
                    remappedSections += 1

        for section, key, value, lineNo in overrideLines(path):
            if RemapMark not in section.lower():
                own += 1
                continue

            if value.lower() == "null":
                cleared += 1
                continue

            carried.append((rel, lineNo, section, key, value))

    if not quiet:
        name = os.path.basename(os.path.abspath(root))
        if remappedSections == 0:
            print("%-22s NOTHING WAS CHECKED -- no remapped section in any .ini. Is it fixed, and "
                  "is this the right folder?" % name)
        elif carried:
            print("%-22s CARRIED %d line(s) that undo the remap "
                  "(%d cleared, %d in the mod's own sections)"
                  % (name, len(carried), cleared, own))
            for rel, lineNo, section, key, value in carried:
                print("    %s:%d  [%s]  %s = %s" % (rel, lineNo, section, key, value))
        elif own == 0 and cleared == 0:
            # Not a pass. A mod whose own sections bind no blend remap had nothing for the fix to
            # carry across, so this question was never put to it -- which is a different answer
            # from "asked and clean", and reads identically unless it is said out loud.
            print("%-22s VACUOUS -- %d remapped section(s) but the mod binds no blend remap of its "
                  "own, so there was nothing to carry. Not evidence either way."
                  % (name, remappedSections))
        else:
            print("%-22s ok -- %d remapped section(s), %d override(s) cleared, %d left in the "
                  "mod's own sections" % (name, remappedSections, cleared, own))

    return carried, cleared, own, remappedSections


def prove(root):
    """Require the check to FAIL on a copy with one carried line put back."""
    with tempfile.TemporaryDirectory() as scratch:
        copy = os.path.join(scratch, os.path.basename(os.path.abspath(root)) or "mod")
        shutil.copytree(root, copy)

        broke = None
        for path in iniFiles(copy):
            lines = open(path, encoding="utf-8", errors="replace").read().split("\n")

            # The cleared line has to be one INSIDE a remapped section. A mod's own cleanup list
            # clears these too, and breaking one of those proves nothing: the check would rightly
            # file it under the mod's own lines and stay silent, which is what the first version of
            # this did -- it reported the check dead when the check was correct.
            section = ""
            for i, line in enumerate(lines):
                found = Section.match(line.rstrip("\r"))
                if found:
                    section = found.group(1)
                    continue

                if RemapMark not in section.lower() or "=" not in line:
                    continue

                key, value = line.split("=", 1)
                if key.strip().lower() in OverrideKeys and value.strip().lower() == "null":
                    indent = line[:len(line) - len(line.lstrip())]
                    lines[i] = ("%s%s = ref ResourceProvedCarriedComponent9"
                                % (indent, key.strip()))
                    broke = (os.path.relpath(path, copy), i + 1, section)
                    break

            if broke is not None:
                open(path, "w", encoding="utf-8", newline="").write("\n".join(lines))
                break

        if broke is None:
            print("--prove: no cleared `= null` override inside a remapped section to put back -- "
                  "nothing to break, so this folder cannot prove the check. Use a FIXED mod of a "
                  "character past 256 bones.")
            return False

        carried, _cleared, _own, _sections = check(copy, quiet=True)
        if carried:
            print("--prove: the check FAILS on the broken copy (%s:%d, [%s]), as it must" % broke)
            return True

        print("--prove: THE CHECK PASSED A COPY THAT CARRIES ONE -- it is not checking anything")
        return False


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n", 1)[0])
    parser.add_argument("folders", nargs="+", help="fixed mod folder(s)")
    parser.add_argument("--prove", action="store_true",
                        help="require the check to fail on a deliberately broken copy of the first "
                             "folder, then exit")
    args = parser.parse_args()

    sys.stdout.reconfigure(errors="replace")

    if args.prove:
        return 0 if prove(args.folders[0]) else 1

    # Three outcomes, counted separately on purpose. A summary that adds "carries a line" to
    # "could not be read" reports a folder that does not exist as a remap bug, which is the
    # conflation Overview habit 66 is about -- this script printed exactly that once.
    bad = blind = missing = 0
    for folder in args.folders:
        if not os.path.isdir(folder):
            print("%-22s NOT A FOLDER" % os.path.basename(os.path.abspath(folder)))
            missing += 1
            continue

        carried, cleared, own, sections = check(folder)
        if carried:
            bad += 1
        elif sections == 0 or (own == 0 and cleared == 0):
            blind += 1

    answered = len(args.folders) - blind - missing
    print()
    print("%d of %d folder(s) answered the question; %d carry a line that undoes the remap"
          % (answered, len(args.folders), bad))
    if blind:
        print("%d answered nothing (unfixed, or the mod binds no blend remap of its own)" % blind)
    if missing:
        print("%d could not be read" % missing)

    return 1 if (bad or missing or answered == 0) else 0


if __name__ == "__main__":
    sys.exit(main())
