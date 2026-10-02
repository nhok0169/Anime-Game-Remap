"""
Makes the members a C++ class inherits safe to render on the same page as the class they come from

The Doxyfile sets ``INLINE_INHERITED_MEMB = YES``, so the XML of every class also lists the members
it inherits -- which is what lets a reader of, say, ``IfPredTokenizer`` see the methods it gets from
``BaseTokenizer`` without leaving the class. Doxygen writes each inherited copy with the PARENT's id,
though, and the whole C++ reference is one page, so every copy is a second anchor of the same name
(886 "Duplicate explicit target name" warnings, and an attribute table link that jumps to the parent).
On a template class it also copies a member the child already overrides, which Sphinx reports as a
duplicate C++ declaration.

``prepare`` copies the XML to another folder and, in that copy only:

#. gives each inherited member an id under the class it was copied into
#. drops an inherited member when the class already declares one with the same signature

The committed ``core/xml`` stays exactly what Doxygen writes, so the way it is regenerated does not change.
"""

import os
import re
import shutil
from typing import Dict, List, Tuple

MemberPattern = re.compile(r'<memberdef\b[^>]*?\bid="([^"]+)".*?</memberdef>', re.DOTALL)
CompoundIdPattern = re.compile(r'<compounddef\b[^>]*?\bid="([^"]+)"')
EmptySectionPattern = re.compile(r'[ \t]*<sectiondef\b[^>]*>\s*</sectiondef>\n?')


def _signature(member: str) -> Tuple[str, str, str]:
    name = re.search(r"<name>(.*?)</name>", member, re.DOTALL)
    args = re.search(r"<argsstring>(.*?)</argsstring>", member, re.DOTALL)
    const = re.search(r'\bconst="(\w+)"', member)

    argsText = args.group(1) if args else ""
    argsText = re.sub(r"\s*(override|final|=\s*0|=\s*default|=\s*delete)\b\s*", "", argsText)
    argsText = re.sub(r"\s+", " ", argsText).strip()
    return (name.group(1).strip() if name else "", argsText, const.group(1) if const else "")


def _fixCompound(text: str) -> Tuple[str, int, int]:
    compound = CompoundIdPattern.search(text)
    if compound is None:
        return text, 0, 0

    ownPrefix = compound.group(1) + "_1"
    members: List[re.Match] = list(MemberPattern.finditer(text))
    ownSignatures = {_signature(m.group(0)) for m in members if m.group(1).startswith(ownPrefix)}
    usedIds = {m.group(1) for m in members if m.group(1).startswith(ownPrefix)}

    renamed = 0
    dropped = 0
    pieces: List[str] = []
    last = 0
    for m in members:
        memberId = m.group(1)
        pieces.append(text[last:m.start()])
        last = m.end()

        if memberId.startswith(ownPrefix):
            pieces.append(m.group(0))
            continue

        signature = _signature(m.group(0))
        if signature in ownSignatures:
            dropped += 1
            continue
        ownSignatures.add(signature)

        # an id is '<compound id>_1<anchor>', and the anchor itself has no underscore
        newId = ownPrefix + memberId.rsplit("_1", 1)[-1]
        while newId in usedIds:
            newId += "_inherited"
        usedIds.add(newId)

        member = m.group(0)
        pieces.append(member.replace(f'id="{memberId}"', f'id="{newId}"', 1))
        renamed += 1

    pieces.append(text[last:])
    text = "".join(pieces)
    if dropped:
        text = EmptySectionPattern.sub("", text)
    return text, renamed, dropped


def prepare(srcDir: str, dstDir: str) -> Dict[str, int]:
    """
    Copies the Doxygen XML in ``srcDir`` to ``dstDir`` with every class's inherited members given ids of
    their own, and returns how many members were renamed and dropped
    """

    if os.path.isdir(dstDir):
        shutil.rmtree(dstDir)
    shutil.copytree(srcDir, dstDir)

    stats = {"renamed": 0, "dropped": 0}
    for fileName in os.listdir(dstDir):
        if not fileName.startswith(("class", "struct")) or not fileName.endswith(".xml"):
            continue

        path = os.path.join(dstDir, fileName)
        with open(path, "r", encoding="utf-8", newline="") as f:
            text = f.read()

        newText, renamed, dropped = _fixCompound(text)
        if newText != text:
            with open(path, "w", encoding="utf-8", newline="") as f:
                f.write(newText)

        stats["renamed"] += renamed
        stats["dropped"] += dropped

    return stats
