#
# ===== wwmiUpdateRows =====
#
# The `DrawPasses` and `TextureRegisters` rows of a WuWa game update that REHASHES SHADERS, read off frame dumps.
#
# Wuthering Waves 3.8 (2026-11-11) moved no model and rehashed every pass shader of every character, and gave most
# material shaders one more texture, which moved every register after it one on. Every `if vs == <tag>` gate a fix
# writes then matches nothing. The answer is data: new rows at the new version, beside the old ones (a fix made for
# the old version still reads them). This tool derives them, so the next update is a dump per character and one run.
#
# How (per character):
#
#   1. PAIR each of her draws in an OLD dump (taken before the update) with her draw in the NEW dump, by
#      (vb0, start index, index count) and, among several draws of one range, by the textures they bind. A draw is
#      hers by its vb0 (every `vb0` and `*Mesh` row `HashData` gives her, at any version).
#   2. MAP each shader of each `DrawPasses` row (at the newest version below the new one) to the shader the paired
#      draw of the same component uses now. The pixel shader comes from her draws of that component; a vertex shader
#      from the draws with that pixel shader (ALL of them, where one old shader became several), else any of her
#      draws with that vertex shader, else (pixel and vertex
#      shaders alike) any character's, only when every pairing agrees. Tags are the fixes' own values: one per new
#      vertex shader, the smallest where two old vertex shaders became one.
#      Then every OTHER vertex shader a new dump draws that component's pass (its new pixel shader) with is added: the
#      3.8 overworld draws the eyes with the menu's pixel shaders and other vertex shaders, which no 3.7 draw pairs.
#   3. MOVE a `TextureRegisters` role only where the draw SETS its texture at the old register and the new draw sets
#      (else binds) the same texture somewhere else. A register a draw only inherits is assumed unmoved.
#
# What it cannot do: a pass no dump draws stays UNRESOLVED (listed; it keeps its old row, which no longer fires), and
# a menu dump does not draw everything the overworld does. Use mod-free CHARACTER-MENU dumps of the same scene before
# and after the update (Game View's dump rules), at Ultra High texture quality.
#
#   py -3.11 wwmiUpdateRows.py --version 3.8 \
#       --pair "Lynae=UpdLynae-2026-11-12-051231:Lynae-2026-10-04-232242,LynaeTexDDS-2026-10-05-160356" \
#       --pair ... [--dumps <WWMI folder>] [--json out.json] [--check | --write]
#
#   --pair  <Character>=<new dump>:<old dump>[,<old dump>...]   (folder names with or without "FrameAnalysis-"); a
#           character may be given several times, one new dump per scene (a menu dump, an overworld log dump)
#   --check compare the derived rows with the KEPT BY HAND block of that version in both data files (exit 1 on a
#           difference); --write replace (or insert) that block. Pass every character of the block in one run: the
#           fallback map, the tag choice and the block are shared by all of them.
#
# The 3.8 run (it reproduces the committed block, `--check`):
#   Sanhua=UpdSanhua-2026-11-12-051858:Sanhua37Base-2026-10-05-012744
#   SanhuaExorcist=UpdSanhuaExorcist-2026-11-12-052120:Sanhua37Vanilla-2026-10-05-012508
#   Chisa=UpdChisa-2026-11-12-050627:ChisaRfxOrig-2026-11-11-120216,ChisaSheerOrig-2026-11-11-102630
#   ChisaParfait=UpdChisaParfait-2026-11-12-050852:ChisaNew1-2026-10-10-220321
#   Lynae=UpdLynae-2026-11-12-051231:Lynae-2026-10-04-232242,LynaeTexDDS-2026-10-05-160356
#   LynaePeppermint=UpdLynaePeppermint-2026-11-12-051503:LynaePeppermint-2026-10-04-232459,LynaePeppermintTexDDS-2026-10-05-160859
#   Changli=UpdChangli-2026-11-12-050045:Changli-2026-10-20-111723,ChangliGhost-2026-10-24-095036
#   ChangliNymph=UpdChangliNymph-2026-11-12-050318:ChangliNymph-2026-10-20-111534,ChangliGhostNymph-2026-10-24-095256
#   ChangliNymph=Upd38ChangliOW-2026-11-12-225922:ChangliNymphOverworldLog-2026-10-22-081210,ChangliNymphBlindfoldLog-2026-10-23-062520
#   ChangliNymph=Upd38ChangliOWLevel-2026-11-12-230651:ChangliNymphOverworldLog-2026-10-22-081210,ChangliNymphBlindfoldLog-2026-10-23-062520
#   ChangliNymph=Upd38ChangliMenuBlindfold-2026-11-12-230142:ChangliNymph-2026-10-20-111534
#   Changli=Upd38ChangliMenuBlindfoldBase-2026-11-12-230348:Changli-2026-10-20-111723
#   Sanhua=Upd38SanhuaOW-2026-11-13-013418:
#   Chisa=Upd38ChisaOW-2026-11-13-012757:
#   LynaePeppermint=Upd38LynaeOW-2026-11-13-012605:
# The last seven are the overworld log dumps (`dump --options "log"`, ~2 MB each) that found the face's overworld vertex
# shader (2c08d8bc, every character's face row by the shared-pass rule) and the overworld eye / face shaders. A dump
# with NO old dump (`<Char>=<new>:`) is paired with nothing: it only adds the vertex shaders it draws a pass with.
# Pairing a new OVERWORLD dump with an old MENU dump is wrong -- it maps the menu's vertex shaders onto the overworld's
# and gives them the menu's tags -- so give an overworld dump only an old overworld dump, or none.
#
import argparse
import json
import os
import re
import sys
from collections import Counter, defaultdict

REPO = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", ".."))
DATA = os.path.join(REPO, "Anime Game Remap (for all users)", "api", "src", "cpp", "core", "src", "data")
DUMPS = r"C:\Users\AlexX\Documents\Games\Mods\XXMI-Launcher-Portable-v1.8.5\WWMI"

ROW = re.compile(r'\{\{"([0-9.]+)", ModTypeIdTools::getName\(ModTypeId::(\w+)\), "([^"]*)", "([^"]*)"\}, "([^"]*)"\}')
HASH_ROW = re.compile(r'\{\{"([0-9.]+)", ModTypeIdTools::getName\(ModTypeId::(\w+)\), "([^"]*)"\}, "([^"]*)"\}')
INDEX_ROW = re.compile(r'\{\{"([0-9.]+)", ModTypeIdTools::getName\(ModTypeId::(\w+)\), "", "component(\d+)"\}, "(\d+)"\}')

DrawPattern = re.compile(r"^(?P<draw>\d{6}) (?P<call>[A-Za-z]+)\((?P<args>.*)\)(?P<tail>.*)$")
SlotPattern = re.compile(r"^\s+(?P<slot>\d+): (?:view=\S+ )?resource=\S+ hash=(?P<hash>[0-9a-f]+)")
DrawArgs = re.compile(r"IndexCount:(\d+), StartIndexLocation:(\d+)")
StartSlot = re.compile(r"StartSlot:(\d+), NumViews:(\d+)")


def ver(v):
    return tuple(int(x) for x in v.split("."))


# ---------- the dump ----------

def parseDump(folder):
    """Every draw: vb0, start, count, vs, ps, the ps-t textures standing, and the registers this draw itself SET."""
    state, fresh, vb, vs, ps, pending, out = {}, set(), {}, None, None, None, []
    for line in open(os.path.join(folder, "log.txt"), encoding = "utf-8", errors = "replace"):
        line = line.rstrip("\r\n")
        m = DrawPattern.match(line)
        if m:
            call, args, tail = m.group("call"), m.group("args"), m.group("tail")
            pending = None
            if call == "PSSetShaderResources":
                pending = "ps-t"
                s = StartSlot.search(args)
                if s:
                    a, n = int(s.group(1)), int(s.group(2))
                    for r in range(a, a + n):
                        state.pop(r, None)          # cleared unless a view follows
                        fresh.add(r)
            elif call == "IASetVertexBuffers":
                pending = "vb"
            elif call == "VSSetShader":
                h = re.search(r"hash=([0-9a-f]+)", tail)
                vs = h.group(1) if h else None
            elif call == "PSSetShader":
                h = re.search(r"hash=([0-9a-f]+)", tail)
                ps = h.group(1) if h else None
            elif call in ("DrawIndexed", "DrawIndexedInstanced"):
                d = DrawArgs.search(args)
                out.append({"draw": int(m.group("draw")), "vb0": vb.get(0), "start": int(d.group(2)) if d else None,
                            "count": int(d.group(1)) if d else None, "vs": vs, "ps": ps,
                            "tex": dict(state), "set": sorted(fresh)})
                fresh = set()
            continue
        if pending:
            s = SlotPattern.match(line)
            if s:
                (state if pending == "ps-t" else vb)[int(s.group("slot"))] = s.group("hash")
    return out


def overlap(a, b):
    return sum(1 for r, h in a["tex"].items() if b["tex"].get(r) == h) + 0.5 * len(set(a["tex"].values()) & set(b["tex"].values())) \
        + (5 if (a["set"] and b["set"] and len(a["set"]) == len(b["set"])) else 0)


def pairDraws(old, new, vb0s):
    """(range key, old draw, new draw) for each of her old draws that has a counterpart."""
    o, n = defaultdict(list), defaultdict(list)
    for src, dst in ((old, o), (new, n)):
        for d in src:
            if d["vb0"] in vb0s:
                dst[(d["vb0"], d["start"], d["count"])].append(d)
    res = []
    for key, olds in o.items():
        pool = n.get(key, [])
        cands = sorted(((overlap(a, b), i, j) for i, a in enumerate(olds) for j, b in enumerate(pool)), reverse = True)
        ua, ub = set(), set()
        for _, i, j in cands:
            if i in ua or j in ub:
                continue
            ua.add(i)
            ub.add(j)
            res.append((key, olds[i], pool[j]))
    return res


# ---------- the tables ----------

def readRows(file, before):
    """{(char, comp, type): value} at the newest version below `before`, and the version each came from."""
    best = {}
    for line in open(os.path.join(DATA, file), encoding = "utf-8"):
        m = ROW.search(line)
        if not m:
            continue
        v, char, comp, typ, val = m.groups()
        if ver(v) >= ver(before):
            continue
        k = (char, comp, typ)
        if k not in best or ver(v) >= ver(best[k][0]):
            best[k] = (v, val)
    return best


def vb0sOf(chars):
    out = defaultdict(dict)                 # char -> {hash: mesh name or None}
    for line in open(os.path.join(DATA, "HashData.cpp"), encoding = "utf-8"):
        m = HASH_ROW.search(line)
        if m and m.group(2) in chars and (m.group(3) == "vb0" or m.group(3).endswith("Mesh")):
            out[m.group(2)][m.group(4)] = None if m.group(3) == "vb0" else m.group(3)
    return out


def offsetsOf(chars, upTo):
    offs = defaultdict(dict)
    for line in open(os.path.join(DATA, "IndexData.cpp"), encoding = "utf-8"):
        m = INDEX_ROW.search(line)
        if m and m.group(2) in chars and ver(m.group(1)) <= ver(upTo):
            offs[m.group(2)].setdefault(m.group(1), {})[int(m.group(3))] = int(m.group(4))
    return {c: vs[max(vs, key = ver)] for c, vs in offs.items()}


# ---------- the derivation ----------

def derive(pairs, newVersion, dumps):
    chars = list(pairs)
    vb0s, offsets = vb0sOf(chars), offsetsOf(chars, newVersion)

    def componentOf(char, vb0, start):
        if vb0s[char].get(vb0):
            return vb0s[char][vb0]
        comp = None
        for k, off in sorted(offsets[char].items(), key = lambda kv: kv[1]):
            if start >= off:
                comp = k
        return "component" + str(comp)

    folder = lambda n: os.path.join(dumps, n if n.startswith("FrameAnalysis-") else "FrameAnalysis-" + n)
    draws = {c: defaultdict(Counter) for c in chars}          # (comp, old ps, old vs) -> Counter((new ps, new vs))
    regMoves = {c: defaultdict(Counter) for c in chars}       # (comp or *, old ps, old reg) -> Counter(new regs)
    drawnWith = {c: defaultdict(Counter) for c in chars}      # (comp, new ps) -> Counter(new vs), every NEW draw
    report = []
    for c, newDump, olds in ((c, n, o) for c, ps in pairs.items() for n, o in ps):
        new = parseDump(folder(newDump))
        for d in new:
            if d["vb0"] in vb0s[c] and d["vs"] and d["ps"]:
                drawnWith[c][(componentOf(c, d["vb0"], d["start"]), d["ps"])][d["vs"]] += 1
        for o in olds:
            got = pairDraws(parseDump(folder(o)), new, set(vb0s[c]))
            report.append(f"PAIRED {c} {o} -> {newDump}: {len(got)} draws")
            for key, a, b in got:
                comp = componentOf(c, key[0], key[1])
                draws[c][(comp, a["ps"], a["vs"])][(b["ps"], b["vs"])] += 1
                newAt, newSet = defaultdict(list), defaultdict(list)
                for r, h in b["tex"].items():
                    newAt[h].append(r)
                    if r in b["set"]:
                        newSet[h].append(r)
                for r in a["set"]:
                    h = a["tex"].get(r)
                    if h is None:
                        continue
                    to = tuple(sorted(newSet.get(h) or newAt.get(h, [])))
                    regMoves[c][(comp, a["ps"], r)][to] += 1
                    regMoves[c][("*", a["ps"], r)][to] += 1

    glob = defaultdict(Counter)                                # any character's: old hash -> Counter(new hash)
    globPass = defaultdict(Counter)                            # any character's: (old ps, old vs, new ps) -> Counter(new vs)
    for c in chars:
        for (_, ps, vs), cnt in draws[c].items():
            for (nps, nvs), n in cnt.items():
                glob[ps][nps] += n
                glob[vs][nvs] += n
                globPass[(ps, vs, nps)][nvs] += n

    def only(cnt):
        return next(iter(cnt)) if len(cnt) == 1 else None

    def mapPs(char, comp, ps):
        got = Counter()
        for (cmp, ops, _), cnt in draws[char].items():
            if cmp == comp and ops == ps:
                for (nps, _), n in cnt.items():
                    got[nps] += n
        if got:
            return only(got), dict(got)
        return only(glob.get(ps, {})), dict(glob.get(ps, {}))

    def mapVs(char, comp, ps, vs, newPs):
        """The new vertex shaders of an old one: EVERY one her draws of this component and pass use with the new
        pixel shader (one old shader may become two, e.g. a menu and an overworld variant -- a gate needs both),
        else every one ANY character's draws use with the same old and new pixel shaders (the shared eye / face
        shaders), else the one any of her draws, else any character's, uses."""
        got, anyOfHers = Counter(), Counter()
        for (cmp, ops, ovs), cnt in draws[char].items():
            for (nps, nvs), n in cnt.items():
                if ovs == vs:
                    anyOfHers[nvs] += n
                    if cmp == comp and ops == ps and nps == newPs:
                        got[nvs] += n
        if got:
            return sorted(got, key = lambda h: -got[h]), dict(got)
        shared = globPass.get((ps, vs, newPs))                 # a shader the characters SHARE (eyes, face): same pass anywhere
        if shared:
            return sorted(shared, key = lambda h: -shared[h]), dict(shared)
        for c in (anyOfHers, glob.get(vs, {})):
            if c:
                return ([only(c)] if only(c) else None), dict(c)
        return None, {}

    passRows = defaultdict(dict)                               # (char, comp, pass) -> {"vs", "ps", "tags"}
    for (char, comp, typ), (_, val) in readRows("DrawPassData.cpp", newVersion).items():
        if char not in pairs or ".ps-t" in typ:
            continue
        name, _, field = typ.partition(".")
        passRows[(char, comp, name)][field or "vs"] = val

    out, tagOfNew = [], defaultdict(set)
    for (char, comp, name) in sorted(passRows, key = lambda k: (chars.index(k[0]), k[1], k[2])):
        r = passRows[(char, comp, name)]
        if "vs" not in r or "ps" not in r:
            continue
        ps, seen = mapPs(char, comp, r["ps"])
        if ps is None:
            report.append(f"UNRESOLVED ps {char} {comp} {name} {r['ps']}: {seen}")
            continue
        vss, tags = [], r.get("tags", "").split(",")
        for i, vs in enumerate(r["vs"].split(",")):
            nvs, seen = mapVs(char, comp, r["ps"], vs, ps)
            if nvs is None:
                report.append(f"UNRESOLVED vs {char} {comp} {name} {vs} (ps {r['ps']}->{ps}): {seen}")
                continue
            if len(nvs) > 1:
                report.append(f"SPLIT vs {char} {comp} {name}: {vs} -> {nvs}")
            for nv in nvs:
                if nv in vss:
                    report.append(f"MERGED vs {char} {comp} {name}: two old vs -> {nv}")
                    continue
                vss.append(nv)
                if i < len(tags) and tags[i]:
                    tagOfNew[nv].add(tags[i])
        for nv in drawnWith[char].get((comp, ps), {}):         # every vertex shader a new dump draws this pass with
            if vss and nv not in vss:
                report.append(f"ALSO DRAWN WITH vs {char} {comp} {name}: {nv} (ps {ps})")
                vss.append(nv)
                if not tagOfNew[nv] and tags and tags[0]:
                    tagOfNew[nv].add(tags[0])
        if not vss:
            report.append(f"DROPPED pass {char} {comp} {name}: no vertex shader resolved")
            continue
        out.append({"char": char, "comp": comp, "pass": name, "vs": vss, "ps": ps})
    tagKey = lambda t: tuple(int(x) for x in t.split("."))
    for o in out:
        o["tags"] = [sorted(tagOfNew[nv], key = tagKey)[0] for nv in o["vs"]] if all(tagOfNew[nv] for nv in o["vs"]) else []
    for nv, ts in tagOfNew.items():
        if len(ts) > 1:
            report.append(f"TAG CONFLICT new vs {nv}: {sorted(ts, key = tagKey)}")

    regs = []
    for (char, comp, typ), (_, reg) in readRows("TextureRegisterData.cpp", newVersion).items():
        if char not in pairs or not reg.startswith("ps-t"):
            continue
        pk = (char, comp, typ.split("@")[1] if "@" in typ else "main")
        if pk not in passRows or "ps" not in passRows[pk]:
            continue
        ops, old = passRows[pk]["ps"], int(reg[4:])
        mv = regMoves[char].get((comp, ops, old)) or regMoves[char].get(("*", ops, old))
        if not mv:
            continue
        if len(mv) != 1:
            report.append(f"REG AMBIGUOUS {char} {comp} {typ} {reg}: {dict(mv)}")
            continue
        to = next(iter(mv))
        if not to:
            report.append(f"REG GONE {char} {comp} {typ} {reg}")
        elif old in to:
            continue
        elif len(to) != 1:
            report.append(f"REG MULTI {char} {comp} {typ} {reg} -> {to}")
        else:
            regs.append({"char": char, "comp": comp, "type": typ, "old": reg, "new": "ps-t" + str(to[0])})
    regs.sort(key = lambda r: (chars.index(r["char"]), r["comp"], r["type"]))
    return out, regs, report


# ---------- the KEPT BY HAND block ----------

def blocks(newVersion, passes, regs):
    row = lambda c, comp, typ, val: (f'        {{{{"{newVersion}", ModTypeIdTools::getName(ModTypeId::{c}), "{comp}", "{typ}"}}, '
                                     f'"{val}"}},')
    dp = [f"        // The game's {newVersion} update rehashed the pass shaders of these characters -- read off their mod-free",
          "        // character-menu frame dumps against the ones from before it, draw by draw (Tools/Misc/Diagnostics/",
          "        // wwmiUpdateRows.py). A pass the dumps do not draw has no row here, so it keeps its older one. Tags are the",
          "        // fixes' own values, one per vertex shader (the smallest where two older vertex shaders became one)"]
    last = None
    for r in passes:
        if r["char"] != last:
            dp.append("        // " + r["char"])
            last = r["char"]
        dp += [row(r["char"], r["comp"], r["pass"], ",".join(r["vs"])), row(r["char"], r["comp"], r["pass"] + ".ps", r["ps"])]
        if r["tags"]:
            dp.append(row(r["char"], r["comp"], r["pass"] + ".tags", ",".join(r["tags"])))
    tr = [f"        // The game's {newVersion} update moved these roles: the texture the draw sets there now sits at another",
          "        // register (Tools/Misc/Diagnostics/wwmiUpdateRows.py). A role not listed is where it was"]
    tr += [row(r["char"], r["comp"], r["type"], r["new"]) for r in regs]
    return {"DrawPassData.cpp": dp, "TextureRegisterData.cpp": tr}


def markers(newVersion):
    return (f"        // ===== KEPT BY HAND: Wuthering Waves {newVersion} (genWWMIAssetData.py keeps this block) =====\n",
            f"        // ===== END KEPT BY HAND: Wuthering Waves {newVersion} =====\n")


def rowsOfText(text):
    return sorted(m.group(0) for m in ROW.finditer(text))


def checkOrWrite(newVersion, out, write):
    begin, end = markers(newVersion)
    differs = False
    for file, lines in out.items():
        path = os.path.join(DATA, file)
        raw = open(path, "rb").read().decode("utf-8")
        crlf = "\r\n" in raw
        text = raw.replace("\r\n", "\n")
        block = begin + "\n".join(lines) + "\n" + end
        has = begin in text
        cur = text[text.index(begin):text.index(end, text.index(begin)) + len(end)] if has else ""
        mine, theirs = rowsOfText(block), rowsOfText(cur)
        if mine == theirs:
            print(f"{file}: SAME ({len(mine)} rows)")
        else:
            differs = True
            only = lambda a, b: [x for x in a if x not in b]
            print(f"{file}: DIFFERS ({len(theirs)} rows in the file, {len(mine)} derived)")
            for x in only(theirs, mine):
                print("   - " + x)
            for x in only(mine, theirs):
                print("   + " + x)
        if write:
            if has:
                text = text.replace(cur, block)
            else:
                anchor = "    };\n    return rows;\n}"
                assert text.count(anchor) == 1, (file, text.count(anchor))
                text = text.replace(anchor, block + anchor)
            open(path, "wb").write((text.replace("\n", "\r\n") if crlf else text).encode("utf-8"))
            print(f"   wrote {file}")
    return differs


def main():
    ap = argparse.ArgumentParser(description = __doc__)
    ap.add_argument("--version", required = True, help = "the game version the new dumps were taken at")
    ap.add_argument("--pair", action = "append", required = True, help = "<Character>=<new dump>:<old dump>[,...]")
    ap.add_argument("--dumps", default = DUMPS, help = "the folder holding the FrameAnalysis-* dumps")
    ap.add_argument("--json", help = "write the derived rows and the report here")
    g = ap.add_mutually_exclusive_group()
    g.add_argument("--check", action = "store_true", help = "compare with the version's KEPT BY HAND block; exit 1 if different")
    g.add_argument("--write", action = "store_true", help = "replace (or insert) the version's KEPT BY HAND block")
    a = ap.parse_args()

    pairs = {}
    for p in a.pair:
        char, rest = p.split("=", 1)
        new, olds = rest.split(":", 1)
        pairs.setdefault(char, []).append((new, [o for o in olds.split(",") if o]))
    passes, regs, report = derive(pairs, a.version, a.dumps)
    print(f"{len(passes)} passes, {len(regs)} moved registers")
    print("\n".join(report))
    if a.json:
        json.dump({"drawPasses": passes, "textureRegisters": regs, "report": report}, open(a.json, "w"), indent = 1)
    if a.check or a.write:
        differs = checkOrWrite(a.version, blocks(a.version, passes, regs), a.write)
        if a.check and differs:
            sys.exit(1)


if __name__ == "__main__":
    main()
