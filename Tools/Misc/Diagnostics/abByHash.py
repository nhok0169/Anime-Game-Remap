##### Credits

# ===== Anime Game Remap (AG Remap) =====
# Authors: Albert Gold#2696, NK#1321
#
# if you used it to remap your mods pls give credit for "Albert Gold#2696" and "Nhok0169"
# Special Thanks:
#   nguen#2011 (for support)
#   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
#   HazrateGolabi#1364 (for being awesome, and improving the code)

##### EndCredits

# A/B two fixed folders' remap blocks keyed by what 3DMigoto MATCHES -- (hash, match_first_index) --
# rather than by section name, since the old script and this library name draw sections differently
# (..HeadHuTaoRemapFix against ..HeadHuTaoRemapIB). Every value naming a Resource section is replaced
# by the md5 of the file it names (plus its type/stride/format); `if` lines are KEPT (a namespace
# mod's `if $\X\Master\swapvar == n` is the point), whitespace-normalised. Called command lists are
# inlined, so a body bound through `run =` compares equal to one bound directly.
#   py -3 abByHash.py <folder A> <folder B>
import hashlib, os, re, sys

def sections(text):
    out, cur = {}, None
    for line in text.splitlines():
        s = line.strip()
        if not s or s.startswith(";"):
            continue
        m = re.match(r"^\[(.+)\]$", s)
        if m:
            cur = m.group(1); out[cur] = []
            continue
        if cur is not None:
            out[cur].append(s)
    return out

def remapBlock(text):
    i = text.find(" Remap ---------------")
    return text[text.rfind("\n", 0, i) + 1:] if i >= 0 else ""

def kv(line):
    if "=" not in line:
        return line.lower(), None
    k, v = line.split("=", 1)
    return k.strip().lower(), v.strip()

def reduce(folder, iniPath):
    text = open(iniPath, encoding="utf-8", errors="replace").read()
    allSecs = {k.lower(): v for k, v in sections(text).items()}
    iniDir = os.path.dirname(iniPath)
    def resolve(val):
        sec = allSecs.get(val.lower())
        if sec is None or not val.lower().startswith("resource"):
            return val
        props = dict(kv(l) for l in sec if "=" in l)
        fn = props.get("filename")
        digest = "?"
        if fn:
            p = os.path.join(iniDir, fn.replace("\\", os.sep))
            digest = hashlib.md5(open(p, "rb").read()).hexdigest()[:10] if os.path.exists(p) else "MISSING"
        extra = ",".join(f"{k}={props[k]}" for k in ("type", "stride", "format") if k in props)
        return f"<{digest}{',' + extra if extra else ''}>"
    def body(name, depth=0):
        lines = []
        for l in allSecs.get(name.lower(), []):
            k, v = kv(l)
            if k == "run" and v and v.lower() in allSecs and depth < 8:
                lines.append(f"run -> {{")
                lines += ["  " + x for x in body(v, depth + 1)]
                lines.append("}")
                continue
            if v is None:
                lines.append(re.sub(r"\s+", " ", l.lower()))
            else:
                lines.append(f"{k} = {resolve(v)}")
        return lines
    out = {}
    for name, lines in sections(remapBlock(text)).items():
        if not name.lower().startswith("textureoverride"):
            continue
        props = [kv(l) for l in lines]
        h = next((v for k, v in props if k == "hash"), None)
        idx = next((v for k, v in props if k == "match_first_index"), "")
        key = (h, idx)
        out.setdefault(key, []).append([l for l in body(name) if not l.startswith("hash =") and not l.startswith("match_first_index =")])
    return out

def iniFiles(root):
    for d, _, fs in os.walk(root):
        for f in fs:
            if f.lower().endswith(".ini") and not f.lower().startswith("disabled"):
                yield os.path.relpath(os.path.join(d, f), root)

A, B = sys.argv[1], sys.argv[2]
paths = sorted(set(iniFiles(A)) | set(iniFiles(B)))
problems = 0
for rel in paths:
    pa, pb = os.path.join(A, rel), os.path.join(B, rel)
    if not os.path.exists(pa) or not os.path.exists(pb):
        print("ONLY ONE SIDE:", rel, "(A)" if os.path.exists(pa) else "(B)"); problems += 1; continue
    ra, rb = reduce(A, pa), reduce(B, pb)
    if not ra and not rb:
        continue
    for key in sorted(set(ra) | set(rb), key=str):
        if ra.get(key) == rb.get(key):
            continue
        problems += 1
        print(f"DIFF {rel} hash={key[0]} index={key[1]}")
        for side, r in (("A", ra), ("B", rb)):
            for n, b in enumerate(r.get(key, [["<absent>"]])):
                print(f"   {side}{n}: " + " | ".join(b))
    print(f"checked {rel}: {len(ra)} vs {len(rb)} matched keys")
print("problems:", problems)
sys.exit(1 if problems else 0)
