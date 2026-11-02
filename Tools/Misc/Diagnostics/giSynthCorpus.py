"""A GI sweep corpus built from the repo alone: every GI character's identity mod and five structural variants.

    py -3 giSynthCorpus.py <out folder> [--names Yelan,YelanTranquil,...] [--list <corpus.txt>]

There is no GI mod library on every machine (the 245-mod sweep lives on the maintainer's other computer), so
this builds one from `Data/Mod Downloads/GI/<Name>/<version>`: those folders ARE the game's own model as GIMI
buffers and textures, per object (a classic character: `<Name><Obj>.ib`, one Position / Blend / Texcoord) or per
component (a multi-component skin: `<Name><Comp><Obj>.ib`, a Position / Blend / Texcoord per component). The
hashes come from the library's own `HashData` (every version merged, the newest winning) and an object's
`match_first_index` is the running total of the index buffers before it, which is what `IndexData` holds for the
classic characters (checked on Yelan: 0 / 20913 / 51759 / 54042).

Per character, one folder each:

  <Name>Identity   the game's model as a mod: every object drawn with its own textures
  <Name>Merged     a genshin_merge_mods.py-shaped master of two variants behind `$swapvar`, every override only
                   `hash` / `match_first_index` / `run =`, the bindings in command lists; variant 1's index
                   buffers are the same triangles in REVERSE order, so a fix that pairs a branch with the wrong
                   buffer moves bytes
  <Name>MergedNull the same master with one object `ib = null` in variant 1 (a hidden object, not a missing one)
  <Name>Ranges     the first drawn object drawn as two halves under two independent toggles
  <Name>Ib16       every index buffer 16-bit (R16_UINT), when the vertex counts allow it
  <Name>Missing    the identity without its last object (a classic character) or its last component (a skin)

None of these is a real mod, and none replaces one: they cover the STRUCTURAL axes (branches, ranges, widths,
absences) on every character at once, which is what a change to the templates' shared code moves. `--list`
writes the folders, one per line, for `regressBuilds.py`.
"""
import argparse
import array
import os
import re
import shutil
import sys

Repo = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", ".."))
APISrc = os.path.join(Repo, "Anime Game Remap (for all users)", "api", "src", "py")
sys.path.insert(0, APISrc)
if hasattr(os, "add_dll_directory"):
    os.add_dll_directory(os.path.join(APISrc, "FixRaidenBoss2"))
import FixRaidenBoss2 as FRB  # noqa: E402

Downloads = os.path.join(Repo, "Data", "Mod Downloads", "GI")
Roles = [("Diffuse", "ps-t0"), ("LightMap", "ps-t1")]
SkinRoles = [("NormalMap", "ps-t0"), ("Diffuse", "ps-t1"), ("LightMap", "ps-t2")]


def verKey(v):
    return tuple(int(p) for p in re.findall(r"\d+", v))


def merged(table, name):
    """A name's rows of a version-keyed asset table, every version merged, the newest winning."""
    out = {}
    for ver in sorted(table, key = verKey):
        row = table[ver].get(name)
        if row is not None:
            out.update(row[""] if "" in row and isinstance(row[""], dict) else row)
    return out


def latestFolder(name):
    base = os.path.join(Downloads, name)
    # the newest folder holding the MESH: a later one may carry only textures (Nilou's 5_4)
    vers = [d for d in os.listdir(base) if os.path.isdir(os.path.join(base, d))
            and any(f.endswith("Position.buf") for f in os.listdir(os.path.join(base, d)))]
    return os.path.join(base, max(vers, key = verKey)) if vers else None


class Part:
    """One mesh: a classic character, or one component of a skin."""

    def __init__(self, prefix, files, hashes, folder, indices = None, notObjs = ()):
        self.prefix, self.hashes, self.folder = prefix, hashes, folder
        obj = re.compile(re.escape(prefix) + r"([A-Z][A-Za-z]*)\.ib$")
        self.objs = sorted(m.group(1) for m in (obj.match(f) for f in files)
                           if m and not any(m.group(1).startswith(c) for c in notObjs))
        if indices:
            self.objs.sort(key = lambda o: int(indices.get(o.lower(), "0")))
        self.first, at = {}, 0
        for o in self.objs:
            self.first[o] = at
            at += os.path.getsize(os.path.join(folder, prefix + o + ".ib")) // 4
        pos = os.path.join(folder, prefix + "Position.buf")
        self.vertices = os.path.getsize(pos) // 40 if os.path.isfile(pos) else 0
        self.texStride = (os.path.getsize(os.path.join(folder, prefix + "Texcoord.buf")) // self.vertices
                          if self.vertices else 20)

    def textures(self, o):
        roles = SkinRoles if any(os.path.isfile(os.path.join(self.folder, self.prefix + x + "NormalMap.dds")) for x in self.objs) else Roles
        return [(role, reg) for role, reg in roles if os.path.isfile(os.path.join(self.folder, self.prefix + o + role + ".dds"))]

    def ok(self):
        need = ["position_vb", "blend_vb", "texcoord_vb", "ib"]
        return self.vertices > 0 and self.objs and all(k in self.hashes for k in need)


def parts(name):
    folder = latestFolder(name)
    if folder is None:
        return None, []
    files = os.listdir(folder)
    mt = getattr(FRB.ModTypes, name).value
    H = mt.hashes.repo.toNestedDict()
    I = mt.indices.repo.toNestedDict()
    # a component is a mesh of its own (its own Position.buf); the unnamed one, beside named ones, is filed as <Name>Main
    comps = sorted({m.group(1) for m in (re.match(re.escape(name) + r"([A-Z][A-Za-z]*)Position\.buf$", f) for f in files) if m})
    if comps:
        result = [Part(name + c, files, merged(H, name + c), folder) for c in comps]
        if name + "Position.buf" in files:
            result.insert(0, Part(name, files, merged(H, name + "Main"), folder, notObjs = comps))
        return folder, result
    return folder, [Part(name, files, merged(H, name), folder, merged(I, name))]


def reverseIb(src, dst, bits16 = False):
    data = array.array("I")
    with open(src, "rb") as f:
        data.frombytes(f.read())
    tris = [data[i:i + 3] for i in range(0, len(data) - len(data) % 3, 3)]
    out = array.array("I")
    for t in reversed(tris):
        out.extend(t)
    with open(dst, "wb") as f:
        f.write(out.tobytes())


def narrowIb(src, dst):
    data = array.array("I")
    with open(src, "rb") as f:
        data.frombytes(f.read())
    out = array.array("H", data)
    with open(dst, "wb") as f:
        f.write(out.tobytes())


def resources(p, objs, ibDir = "", ibFormat = "DXGI_FORMAT_R32_UINT", suffix = ""):
    lines = []
    for kind, stride in (("Position", 40), ("Blend", 32), ("Texcoord", p.texStride)):
        lines += ["[Resource%s%s%s]" % (p.prefix, kind, suffix), "type = Buffer", "stride = %d" % stride,
                  "filename = %s%s.buf" % (p.prefix, kind), ""]
    for o in objs:
        lines += ["[Resource%s%sIB%s]" % (p.prefix, o, suffix), "type = Buffer", "format = " + ibFormat,
                  "filename = %s%s%s.ib" % (ibDir, p.prefix, o), ""]
    if not suffix:
        for o in objs:
            for role, _ in p.textures(o):
                lines += ["[Resource%s%s%s]" % (p.prefix, o, role), "filename = %s%s%s.dds" % (p.prefix, o, role), ""]
    return lines


def plainOverrides(p, objs, drawOf = None):
    h = p.hashes
    lines = ["[TextureOverride%sPosition]" % p.prefix, "hash = " + h["position_vb"], "vb0 = Resource%sPosition" % p.prefix, "",
             "[TextureOverride%sBlend]" % p.prefix, "hash = " + h["blend_vb"], "vb1 = Resource%sBlend" % p.prefix,
             "handling = skip", "draw = %d,0" % p.vertices, "",
             "[TextureOverride%sTexcoord]" % p.prefix, "hash = " + h["texcoord_vb"], "vb1 = Resource%sTexcoord" % p.prefix, ""]
    if "draw_vb" in h:
        lines += ["[TextureOverride%sVertexLimitRaise]" % p.prefix, "hash = " + h["draw_vb"], ""]
    lines += ["[TextureOverride%sIB]" % p.prefix, "hash = " + h["ib"], "handling = skip", ""]
    for o in objs:
        lines += ["[TextureOverride%s%s]" % (p.prefix, o), "hash = " + h["ib"], "match_first_index = %d" % p.first[o],
                  "ib = Resource%s%sIB" % (p.prefix, o)]
        lines += ["%s = Resource%s%s%s" % (reg, p.prefix, o, role) for role, reg in p.textures(o)]
        lines += (drawOf(o) if drawOf else ["drawindexed = auto"]) + [""]
    return lines


def identity(ps, dropLast = False):
    ps = ps[:-1] if dropLast and len(ps) > 1 else ps
    lines = []
    for i, p in enumerate(ps):
        objs = p.objs[:-1] if dropLast and len(ps) == 1 and len(p.objs) > 1 else p.objs
        lines += plainOverrides(p, objs) + resources(p, objs)
    return lines


def ranges(ps):
    lines = ["[Constants]", "global persist $halfA = 1", "global persist $halfB = 1", "",
             "[KeyHalfA]", "key = VK_LEFT", "type = cycle", "$halfA = 0,1", "",
             "[KeyHalfB]", "key = VK_RIGHT", "type = cycle", "$halfB = 0,1", ""]
    for i, p in enumerate(ps):
        target = p.objs[0]
        count = os.path.getsize(os.path.join(p.folder, p.prefix + target + ".ib")) // 4
        half = (count // 6) * 3

        def drawOf(o, target = target, count = count, half = half):
            if o != target or i != 0:
                return ["drawindexed = auto"]
            return ["if $halfA == 1", "    drawindexed = %d, 0, 0" % half, "endif",
                    "if $halfB == 1", "    drawindexed = %d, %d, 0" % (count - half, half), "endif"]
        lines += plainOverrides(p, p.objs, drawOf) + resources(p, p.objs)
    return lines


def master(ps, nullLast = False):
    lines = ["[Constants]", "global persist $swapvar = 0", "global $active", "", "[Present]", "post $active = 0", "",
             "[KeySwap]", "condition = $active == 1", "key = VK_DOWN", "type = cycle", "$swapvar = 0,1", ""]
    for p in ps:
        h = p.hashes
        nulled = p.objs[-1] if nullLast and p is ps[-1] else None
        lines += ["[TextureOverride%sPosition]" % p.prefix, "hash = " + h["position_vb"], "run = CommandList%sPosition" % p.prefix,
                  "$active = 1", "",
                  "[TextureOverride%sBlend]" % p.prefix, "hash = " + h["blend_vb"], "run = CommandList%sBlend" % p.prefix, "",
                  "[TextureOverride%sTexcoord]" % p.prefix, "hash = " + h["texcoord_vb"], "run = CommandList%sTexcoord" % p.prefix, ""]
        if "draw_vb" in h:
            lines += ["[TextureOverride%sVertexLimitRaise]" % p.prefix, "hash = " + h["draw_vb"], ""]
        lines += ["[TextureOverride%sIB]" % p.prefix, "hash = " + h["ib"], "run = CommandList%sIB" % p.prefix, ""]
        for o in p.objs:
            lines += ["[TextureOverride%s%s]" % (p.prefix, o), "hash = " + h["ib"], "match_first_index = %d" % p.first[o],
                      "run = CommandList%s%s" % (p.prefix, o), ""]

        def chain(body):
            out = []
            for v in (0, 1):
                out += [("if" if v == 0 else "else if") + " $swapvar == %d" % v] + ["    " + l for l in body(v)]
            return out + ["endif", ""]
        lines += ["[CommandList%sPosition]" % p.prefix] + chain(lambda v: ["vb0 = Resource%sPosition.%d" % (p.prefix, v)])
        lines += ["[CommandList%sBlend]" % p.prefix] + chain(lambda v: ["vb1 = Resource%sBlend.%d" % (p.prefix, v), "handling = skip",
                                                                     "draw = %d,0" % p.vertices])
        lines += ["[CommandList%sTexcoord]" % p.prefix] + chain(lambda v: ["vb1 = Resource%sTexcoord.%d" % (p.prefix, v)])
        lines += ["[CommandList%sIB]" % p.prefix] + chain(lambda v: ["handling = skip"])
        for o in p.objs:
            def body(v, o = o):
                if v == 1 and o == nulled:
                    return ["ib = null"]
                return (["ib = Resource%s%sIB.%d" % (p.prefix, o, v)]
                        + ["%s = Resource%s%s%s" % (reg, p.prefix, o, role) for role, reg in p.textures(o)]
                        + ["drawindexed = auto"])
            lines += ["[CommandList%s%s]" % (p.prefix, o)] + chain(body)
        for v in (0, 1):
            lines += resources(p, p.objs, ibDir = "" if v == 0 else "Variant1/", suffix = ".%d" % v)
        for o in p.objs:
            for role, _ in p.textures(o):
                lines += ["[Resource%s%s%s]" % (p.prefix, o, role), "filename = %s%s%s.dds" % (p.prefix, o, role), ""]
    return lines


def write(out, name, folder, lines, ps, ibOp = None, variant1 = False):
    if os.path.isdir(out):
        shutil.rmtree(out)
    os.makedirs(out)
    for f in os.listdir(folder):
        if not f.endswith(".ib") or ibOp is None:
            shutil.copy2(os.path.join(folder, f), os.path.join(out, f))
        else:
            ibOp(os.path.join(folder, f), os.path.join(out, f))
    if variant1:
        os.makedirs(os.path.join(out, "Variant1"))
        for p in ps:
            for o in p.objs:
                f = p.prefix + o + ".ib"
                reverseIb(os.path.join(folder, f), os.path.join(out, "Variant1", f))
    with open(os.path.join(out, name + ".ini"), "w", encoding = "utf-8", newline = "\r\n") as f:
        f.write("\n".join(lines))


def build(name, root):
    folder, ps = parts(name)
    if folder is None or not ps or not all(p.ok() for p in ps):
        return [], "skipped (no download folder, or hashes missing for " + ", ".join(p.prefix for p in ps if not p.ok()) + ")"
    made = []
    for suffix, lines, kw in (("Identity", identity(ps), {}), ("Merged", master(ps), {"variant1": True}),
                              ("MergedNull", master(ps, True), {"variant1": True}), ("Ranges", ranges(ps), {}),
                              ("Missing", identity(ps, True), {})):
        out = os.path.join(root, name + suffix)
        write(out, name, folder, lines, ps, **kw)
        made.append(out)
    if all(p.vertices < 65536 for p in ps):
        out = os.path.join(root, name + "Ib16")
        lines = []
        for p in ps:
            lines += plainOverrides(p, p.objs) + resources(p, p.objs, ibFormat = "DXGI_FORMAT_R16_UINT")
        write(out, name, folder, lines, ps, ibOp = narrowIb)
        made.append(out)
    return made, "%d folders, %s" % (len(made), " + ".join("%s(%s)" % (p.prefix, ",".join(p.objs)) for p in ps))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("out")
    ap.add_argument("--names", default = "")
    ap.add_argument("--list", default = "")
    a = ap.parse_args()
    names = [n for n in a.names.split(",") if n] or sorted(d for d in os.listdir(Downloads)
                                                           if os.path.isdir(os.path.join(Downloads, d)) and hasattr(FRB.ModTypes, d))
    os.makedirs(a.out, exist_ok = True)
    every = []
    for n in names:
        made, how = build(n, a.out)
        print("%-26s %s" % (n, how))
        every += made
    if a.list:
        with open(a.list, "w", encoding = "utf-8") as f:
            f.write("\n".join(every) + "\n")
    print("built", len(every), "folders")


if __name__ == "__main__":
    main()
