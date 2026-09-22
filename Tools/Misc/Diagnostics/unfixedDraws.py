"""Does any remapped section DRAW over a set of bindings no fix library has re-slotted?

    python unfixedDraws.py <fixed folder> [<fixed folder> ...]

The other half of fixCallPaths.py, which counts a fix library called TWICE over one set of bindings
and is structurally blind to one called NOT AT ALL: a section that binds its ps-t registers, calls
ORFix, draws, binds again and draws again renders its second draws with the light map as the albedo
-- flat green -- and fixCallPaths.py reports 0. A real Citlali mod does exactly that (2026-09-22), and
RegDelimitedAddMode::PerPath's one call per path left 32 of its 42 remapped draws unfixed.

A linear walk per remapped TextureOverride, following `run =` into the file's own command lists: a ps-t assignment starts a new binding set, a `run =` ORFix / NNFix
fixes it, and a `drawindexed` reached while the current set is unfixed is reported. Conservative -- a
rebinding inside any branch counts -- so a report is worth reading, and 0 is a real 0. Exits non-zero
on any report. Needs nothing but the folders.
"""
import os, re, sys

FixCall = re.compile(r"run\s*=\s*CommandList\\global\\ORFix\\(ORFix|NNFix)\s*$", re.I)
RunCall = re.compile(r"run\s*=\s*(\S+)\s*$", re.I)
Binding = re.compile(r"ps-t\d+\s*=")


def sections(path):
    """[name] -> its lines, in order"""
    out, name = {}, None
    for line in open(path, encoding = "utf-8", errors = "replace"):
        s = line.strip()
        if s.startswith("[") and s.endswith("]"):
            name = s[1:-1]
            out[name] = []
        elif name is not None:
            out[name].append(s)
    return out


def effect(name, secs, memo, stack):
    """What calling a section leaves behind: None (touches no binding), "fixed" or "unfixed" --
    following `run =` into the file's own command lists (2026-09-22: a merged master binds and calls
    ORFix inside a command list, and a walk that stopped at the section boundary reported its draws)"""
    if name in memo:
        return memo[name]
    if name in stack or name not in secs:
        return None
    stack.add(name)
    state = None
    for s in secs[name]:
        if Binding.match(s):
            state = "unfixed"
        elif FixCall.match(s):
            state = "fixed"
        else:
            m = RunCall.match(s)
            if m:
                sub = effect(m.group(1), secs, memo, stack)
                if sub is not None:
                    state = sub
    stack.discard(name)
    memo[name] = state
    return state


bad = total = 0
for root in sys.argv[1:]:
    for d, _, fs in os.walk(root):
        for f in fs:
            if not f.lower().endswith(".ini") or f.upper().startswith("DISABLED"):
                continue
            path = os.path.join(d, f)
            secs, memo = sections(path), {}
            for section, lines in secs.items():
                if "RemapFix" not in section or not section.startswith("TextureOverride"):
                    continue
                state = None
                for s in lines:
                    if Binding.match(s):
                        state = "unfixed"
                    elif FixCall.match(s):
                        state = "fixed"
                    elif re.match(r"drawindexed\s*=", s):
                        total += 1
                        if state == "unfixed":
                            bad += 1
                            print(f"  unfixed draw: [{section}] {s}   ({os.path.relpath(path, root)})")
                    else:
                        m = RunCall.match(s)
                        if m:
                            sub = effect(m.group(1), secs, memo, set())
                            if sub is not None:
                                state = sub
print(f"{total} draws checked, {bad} drawn after a rebinding with no fix call since")
sys.exit(1 if bad else 0)
