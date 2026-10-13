"""Structural check: does a FIXED mod draw a downloaded (or the game's own) index buffer over the
mod's vertex buffers?

For every .ini under the given folders (DISABLED / backup files skipped), every remapped
TextureOverride section (name contains 'Remap' and it carries match_first_index, or it is in a
*RemapFix<N>.ini) is walked with its `run =` CommandLists inlined.  An abstract interpretation
tracks the set of possible `ib` bindings (if/else-if/else branches joined, a block with no
`else` also keeps its fall-through).  At every `drawindexed` the possible bindings are recorded.

Reported, only for a file where the MOD itself skips its index buffer (`handling = skip` on a
non-remap *IB section, directly or through its run= CommandList):
  DL    a draw that may use ib = Resource...RemapDL  (the class under audit)
  GAME  a draw that may use the game's index buffer (no ib bound on that path at all)
  NULL  a draw that may use ib = null (draws nothing; informational)

Usage: py -3 checkUndrawnDL.py <folder> [<folder> ...]   exit 1 if any DL/GAME found.
"""
import os, re, sys

SEC_RE = re.compile(r"^\[(.+)\]\s*$")
KV_RE = re.compile(r"^([^=]+?)\s*=\s*(.*)$")


def parse(path):
    secs = {}
    order = []
    cur = None
    with open(path, encoding="utf-8", errors="replace") as fh:
        for raw in fh:
            s = raw.strip()
            if not s or s.startswith(";"):
                continue
            m = SEC_RE.match(s)
            if m:
                cur = m.group(1).strip()
                if cur not in secs:
                    secs[cur] = []
                    order.append(cur)
                continue
            if cur is not None:
                secs[cur].append(s)
    return secs, order


def tokens(lines):
    """-> list of ('if', cond) / ('elif', cond) / ('else',) / ('endif',) / ('kv', key, val)."""
    out = []
    for s in lines:
        low = s.lower()
        if low.startswith("if ") or low == "if":
            out.append(("if", s[3:]))
        elif low.startswith("else if ") or low.startswith("elif "):
            out.append(("elif", s))
        elif low == "else":
            out.append(("else",))
        elif low == "endif":
            out.append(("endif",))
        else:
            m = KV_RE.match(s)
            if m:
                out.append(("kv", m.group(1).strip().lower(), m.group(2).strip()))
    return out


class Walker:
    def __init__(self, secs):
        self.secs = {k.lower(): v for k, v in secs.items()}
        self.draws = []          # (set of ib states)
        self.stack = []
        self.known = []          # conditions true on the current path

    def run_section(self, name, state):
        key = name.lower()
        if key not in self.secs or key in self.stack or len(self.stack) > 20:
            return state  # external command list (ORFix ...) binds nothing of the mod's
        self.stack.append(key)
        toks = tokens(self.secs[key])
        state, _ = self.block(toks, 0, state)
        self.stack.pop()
        return state

    def block(self, toks, i, state):
        """Walk until an else/elif/endif at this depth; return (state, index)."""
        while i < len(toks):
            t = toks[i]
            if t[0] in ("elif", "else", "endif"):
                return state, i
            if t[0] == "if":
                outs = []
                has_else = False
                conds = [t[1]]
                norm = lambda c: re.sub(r"\s+", "", c).lower()
                implied = norm(t[1]) in self.known
                self.known.append(norm(t[1]))
                st, i = self.block(toks, i + 1, set(state))
                self.known.pop()
                outs.append(st)
                while i < len(toks) and toks[i][0] in ("elif", "else"):
                    if toks[i][0] == "else":
                        has_else = True
                    else:
                        conds.append(re.sub(r"^(else\s+if|elif)\s+", "", toks[i][1], flags=re.I))
                    pushed = toks[i][0] != "else"
                    if pushed:
                        self.known.append(norm(conds[-1]))
                    st, i = self.block(toks, i + 1, set(state))
                    if pushed:
                        self.known.pop()
                    outs.append(st)
                i += 1  # endif
                # A chain of `$v == N` over ONE variable is taken as exhaustive: a merged master's
                # $swapvar only cycles the values its branches list, so its fall-through never runs.
                eq = [re.fullmatch(r"\s*(\$[\w\\]+)\s*==\s*\d+\s*", c) for c in conds]
                exhaustive = len(conds) >= 2 and all(eq) and len({m.group(1).lower() for m in eq}) == 1
                if conds[0].strip() == "1" or implied:     # the fix's own `if 1` wrapper always runs
                    exhaustive = True
                    outs = [outs[0]]
                if not has_else and not exhaustive:
                    outs.append(set(state))
                state = set().union(*outs)
                continue
            _, k, v = t
            if k == "ib":
                state = {v}
            elif k == "run":
                state = self.run_section(v, state)
            elif k in ("drawindexed", "draw", "drawindexedinstanced"):
                self.draws.append(frozenset(state))
            i += 1
        return state, i


def classify(v):
    if v == "<game>":
        return "GAME"
    if v.lower() == "null":
        return "NULL"
    if "remapdl" in v.lower():
        return "DL"
    return "MOD"


def mod_skips_ib(secs):
    low = {k.lower(): v for k, v in secs.items()}
    for name, lines in secs.items():
        n = name.lower()
        if "remap" in n or not n.startswith("textureoverride") or not n.endswith("ib"):
            continue
        todo = [n]
        seen = set()
        while todo:
            x = todo.pop()
            if x in seen or x not in low:
                continue
            seen.add(x)
            for t in tokens(low[x]):
                if t[0] == "kv" and t[1] == "handling" and t[2].lower() == "skip":
                    return True
                if t[0] == "kv" and t[1] == "run":
                    todo.append(t[2].lower())
    return False


def check_file(path):
    secs, order = parse(path)
    if not mod_skips_ib(secs):
        return []
    isCopy = re.search(r"remapfix\d+\.ini$", path.lower()) is not None
    found = []
    for name in order:
        n = name.lower()
        if not n.startswith("textureoverride") or "remap" not in n:
            continue
        lines = secs[name]
        toks = tokens(lines)
        if not any(t[0] == "kv" and t[1] == "match_first_index" for t in toks):
            continue
        w = Walker(secs)
        end = w.run_section(name, {"<game>"})
        if not w.draws:
            # No draw of its own: an old-layout IB section (hash only, `drawindexed = auto`) on the
            # same hash draws whatever this section left bound.
            myHash = next((t[2].lower() for t in toks if t[0] == "kv" and t[1] == "hash"), None)
            for other in order:
                o = other.lower()
                if other == name or not o.startswith("textureoverride") or "remap" not in o:
                    continue
                ot = tokens(secs[other])
                if any(x[0] == "kv" and x[1] == "match_first_index" for x in ot):
                    continue
                if next((x[2].lower() for x in ot if x[0] == "kv" and x[1] == "hash"), None) != myHash:
                    continue
                w.run_section(other, set(end))
        kinds = {}
        for st in w.draws:
            for v in st:
                c = classify(v)
                if c != "MOD":
                    kinds.setdefault(c, set()).add(v)
        for c, vals in kinds.items():
            found.append((c, name, sorted(vals)))
    return found


def iter_inis(root):
    for dp, dn, fn in os.walk(root):
        for f in fn:
            fl = f.lower()
            if fl.endswith(".ini") and not fl.startswith("disabled") and "backup" not in fl and "desktop" not in fl:
                yield os.path.join(dp, f)


def main(argv):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
    bad = 0
    nfiles = 0
    nskip = 0
    for root in argv:
        for p in iter_inis(root):
            nfiles += 1
            secs, _ = parse(p)
            if mod_skips_ib(secs):
                nskip += 1
            for c, name, vals in check_file(p):
                if c in ("DL", "GAME"):
                    bad += 1
                print(f"{c:4} {p} [{name}] ib={vals}")
    print(f"checked {nfiles} .ini files ({nskip} skip their ib); {bad} DL/GAME findings")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
