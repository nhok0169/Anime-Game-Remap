"""A fix call with nothing bound before it, in a remapped section or command list.

    python bareFixCalls.py <fixed folder> [<fixed folder> ...]

ORFix / NNFix re-slot whatever is bound when they run, and they are involutions: a call over the
PREVIOUS draw's already-fixed bindings undoes them. unfixedDraws.py looks for a draw after an unfixed
binding and fixCallPaths.py for two calls on one path; neither sees a call with no binding of its own
before it. The merge template's carried members produced exactly that for a YelanTranquil mod whose
Eye sections bind nothing (2026-09-22), and both of the others passed it.

A linear walk per section, not following `run =`: a merged master that binds inside the command list
it calls just before the fix call is reported too (four such, all older than the check). Read each
report; a new one is worth a look.
"""
import os, re, sys
Fix = re.compile(r"run\s*=\s*CommandList\\global\\ORFix\\(ORFix|NNFix)\s*$", re.I)
bad = total = 0
for root in sys.argv[1:]:
    for d, _, fs in os.walk(root):
        for f in fs:
            if not f.lower().endswith(".ini") or f.upper().startswith("DISABLED"):
                continue
            section = None; bound = False
            for line in open(os.path.join(d, f), encoding="utf-8", errors="replace"):
                s = line.strip()
                if s.startswith("[") and s.endswith("]"):
                    section = s[1:-1] if "RemapFix" in s else None
                    bound = False
                    continue
                if section is None:
                    continue
                if re.match(r"ps-t[0-2]\s*=", s):
                    bound = True
                elif Fix.match(s):
                    total += 1
                    if not bound:
                        bad += 1
                        print(f"  bare fix call: [{section}]  ({os.path.relpath(os.path.join(d, f), root)})")
print(f"{total} fix calls checked, {bad} with nothing bound before them in their own section")
