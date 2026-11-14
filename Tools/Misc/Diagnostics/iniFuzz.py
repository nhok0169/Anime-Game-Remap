"""iniFuzz.py <out folder> <count> [--seed N]: random mods, each <out>/fuzzNNNNN/mod/mod.ini from a grammar plus the files it
names (fuzzNNNNN is the mod's own root: what its paths name outside mod/ lands there).

The grammar mixes what real .ini files hold -- TextureOverride / CommandList / Resource / Constants / Present sections,
nested if / else if / else, run = chains (cycles included), ps-tN / vbN / ib bindings, drawindexed in every form, filename
lines to files present, missing, outside the folder or empty -- with what they should not: unbalanced endif, repeated
sections, repeated keys, junk values, unicode and spaces in names, bare words, empty sections. Hashes are drawn mostly
from one character's tables (default Sanhua) so the classifier hands the file to that character's parser."""
import argparse
import os
import random

SANHUA = {
    "vb0": ["33e4890f"], "cb4": ["f02baf77"], "shapekey_offsets": ["a80e0e2b"], "shapekey_scale": ["bf231a78"],
    "tex": ["48616ac9", "ae6e9014", "1c0c8b91", "345368c9", "abda232b", "ebeeda8c", "5efe7892", "0521977a", "881c236d",
            "aa70ef15", "68ca7071", "98b9635b", "2584190a", "cef6494f", "1dcc0f1d", "4b6d52b9", "03d9850b", "2c0c2728",
            "11b9cadd", "16695017", "1035197c", "b0828323"],
}
VARS = ["$swapvar", "$mod_enabled", "$object_detected", "$state_id", "$x", "$\\WWMIv1\\vg_offset", "$ünï", "$clothes"]
KEYS_JUNK = ["foo", "bar baz", "=", "==", "ps-t", "vb", "run", "hash", "drawindexed", "handling", "local $y", "post $z = 0",
             "$\\WWMIv1\\custom_mesh_scale", "CheckTextureOverride", "x" * 300, "ключ", "\t", "if", "endif"]


class Gen:
    def __init__(self, rng, folder):
        self.rng = rng
        self.folder = folder
        self.names = {"TextureOverride": [], "CommandList": [], "Resource": []}
        self.files = []

    def name(self, kind):
        r = self.rng
        known = self.names[kind]
        if known and r.random() < 0.55:
            return r.choice(known)
        base = r.choice(["Component", "Body", "Hair", "Texture", "Blend", "Position", "Index", "Shared", "Mark", "Restore",
                         "Témp", "With Space", "Remap", "RemapFix", "SanhuaExorcistRemapFix", ""])
        new = kind + base + str(r.randint(0, 12))
        known.append(new)
        return new

    def value(self, key):
        r = self.rng
        pick = r.random()
        if key == "hash":
            if pick < 0.5:
                return r.choice(SANHUA["vb0"])
            if pick < 0.8:
                return r.choice(SANHUA["tex"] + SANHUA["cb4"] + SANHUA["shapekey_offsets"] + SANHUA["shapekey_scale"])
            return r.choice(["deadbeef", "", "33E4890F", "zzzz", "33e4890f 12", "0"])
        if key in ("match_first_index", "match_index_count", "$\\WWMIv1\\vg_offset", "$\\WWMIv1\\vg_count"):
            return r.choice([str(r.randint(0, 200000)), "-1", "", "abc", "1e9", str(2 ** 40), "0x10"])
        if key == "drawindexed":
            return r.choice(["auto", "%d, %d, 0" % (r.randint(0, 99999), r.randint(0, 300000)), "1,2", "", "a, b, c",
                             "%d, %d, %d, 9" % (r.randint(0, 9), r.randint(0, 9), r.randint(0, 9)), "-5, -5, 0"])
        if key == "run":
            return r.choice([self.name("CommandList"), "CustomShader\\WWMIv1\\SkeletonMerger", "CommandList\\RabbitFX\\SetTextures",
                             "", "CommandListNowhere", self.name("CommandList") + " "])
        if key.startswith("ps-t") or key.startswith("vb") or key == "ib":
            return r.choice([self.name("Resource"), "ref " + self.name("Resource"), "null", "ref vb0", "",
                             "Resource\\RabbitFX\\Diffuse", "ResourceMissing", "copy " + self.name("Resource")])
        if key == "filename":
            return self.file()
        if key == "handling":
            return r.choice(["skip", "", "draw", "skip skip"])
        return r.choice(["0", "1", "", "abc", "ref ps-t0", "$x + 1", "null", "Ω"])

    def file(self):
        r = self.rng
        kind = r.random()
        sub = r.choice(["", "Meshes\\", "Textures\\", "..\\", "a b\\", "Ünï\\"])
        name = r.choice(["Blend.buf", "Position.buf", "Index.ib", "Texcoord.buf", "BlendRemapVertexVG.buf", "Diffuse.dds",
                         "Mask.dds", "t=33e4890f.dds", "x.txt", "Component0_Diffuse.dds", ""])
        rel = sub + name
        if kind < 0.6 and name and not sub.startswith(".."):
            path = os.path.join(self.folder, rel.replace("\\", os.sep))
            os.makedirs(os.path.dirname(path) or self.folder, exist_ok = True)
            size = r.choice([0, 1, 7, 8, 16, 24, 40, 64, 128, 4096, r.randint(0, 20000)])
            with open(path, "wb") as f:
                f.write(bytes(r.getrandbits(8) for _ in range(size)))
        # an absolute path stays inside the fuzz root (a fix writes beside the files it names)
        absolute = os.path.abspath(os.path.join(self.folder, "..", "absolute", name or "x")).replace("/", "\\")
        return r.choice([rel, rel, ".\\" + rel, rel + " ", absolute, "..\\" + name])

    def kvp(self, sectionKind):
        r = self.rng
        if sectionKind == "Resource":
            key = r.choice(["filename", "filename", "type", "format", "stride", "array", "data"])
        elif sectionKind == "Constants":
            key = r.choice(["global $x", "global persist $swapvar", "global $mesh_vertex_count", "global $\\WWMIv1\\vg_offset",
                            "$mod_enabled", "global $state_id"])
        elif sectionKind == "Present":
            key = r.choice(["post $object_detected", "run", "$x", "post $\\WWMIv1\\x"])
        else:
            key = r.choice(["hash", "match_first_index", "match_index_count", "run", "run", "ib", "vb0", "vb1", "vb2", "vb3", "vb4",
                            "vb6", "ps-t0", "ps-t1", "ps-t2", "ps-t3", "ps-t5", "drawindexed", "drawindexed", "handling",
                            "$\\WWMIv1\\vg_offset", "$\\WWMIv1\\vg_count", "$object_detected", "this",
                            "ResourceBlendBufferOverride", "CheckTextureOverride", "local $state_id_0", "cs-cb8", "vs-cb4"])
        if r.random() < 0.04:
            return r.choice(KEYS_JUNK)                  # a bare word or a malformed line
        if key.startswith("local ") and r.random() < 0.5:
            return key
        return key + " = " + self.value(key)

    def body(self, sectionKind, depth = 0):
        r = self.rng
        lines = []
        for _ in range(r.randint(0, 9)):
            roll = r.random()
            if roll < 0.18 and depth < 4:
                lines.append("if " + self.condition())
                lines += ["\t" + line for line in self.body(sectionKind, depth + 1)]
                for _ in range(r.choice([0, 0, 1, 2])):
                    lines.append(r.choice(["else if ", "elif "]) + self.condition())
                    lines += ["\t" + line for line in self.body(sectionKind, depth + 1)]
                if r.random() < 0.4:
                    lines.append("else")
                    lines += ["\t" + line for line in self.body(sectionKind, depth + 1)]
                if r.random() < 0.95:                # sometimes the endif is missing
                    lines.append("endif")
            elif roll < 0.21:
                lines.append(r.choice(["endif", "else", "; a comment", "", "[", "]"]))   # unbalanced or stray
            else:
                lines.append(self.kvp(sectionKind))
        return lines

    def condition(self):
        r = self.rng
        lhs = r.choice(VARS + ["vs", "ps", "ResourceMergedSkeleton", "vs-cb4", "o3"])
        op = r.choice(["==", "!=", "===", "!==", ">", "&&", ""])
        rhs = r.choice(["0", "1", "null", "3381.7777", "$x", "abc", ""])
        cond = (lhs + " " + op + " " + rhs).strip()
        return r.choice([cond, cond, "(" + cond + ") && " + r.choice(VARS) + " == 1", "!" + lhs, ""])

    def ini(self):
        r = self.rng
        out = []
        if r.random() < 0.7:
            out += ["[Constants]"] + self.body("Constants")
        if r.random() < 0.5:
            out += ["[Present]"] + self.body("Present")
        for _ in range(r.randint(0, 14)):
            kind = r.choice(["TextureOverride", "TextureOverride", "CommandList", "Resource", "Resource", "ShaderOverride",
                             "Key", "TextureOverrideComponent"])
            if kind == "TextureOverrideComponent":
                header = "TextureOverrideComponent" + str(r.randint(0, 7))
                body = (["hash = " + SANHUA["vb0"][0], "match_first_index = " + str(r.randint(0, 99999)),
                         "match_index_count = " + str(r.randint(0, 99999))] + self.body("TextureOverride"))
            elif kind in self.names:
                header = self.name(kind)
                body = self.body(kind)
            else:
                header = kind + str(r.randint(0, 5))
                body = self.body("TextureOverride")
            out += ["", "[" + header + "]"] + body
        if r.random() < 0.05:
            out = []                                          # an empty file
        r.shuffle(out) if r.random() < 0.03 else None         # a file with its lines in any order
        return "\n".join(out) + ("\n" if r.random() < 0.9 else "")


def main():
    args = argparse.ArgumentParser()
    args.add_argument("out")
    args.add_argument("count", type = int)
    args.add_argument("--seed", type = int, default = 0)
    a = args.parse_args()
    for i in range(a.count):
        seed = a.seed + i
        # each mod in a root of its own (fuzzNNNNN/mod): a path up (`..\`) or an absolute one stays inside that root, so a
        # run -- which follows the folders an .ini names -- never reaches another fuzzed mod
        folder = os.path.join(a.out, "fuzz%05d" % seed, "mod")
        os.makedirs(folder, exist_ok = True)
        gen = Gen(random.Random(seed), folder)
        text = gen.ini()
        with open(os.path.join(folder, "mod.ini"), "w", encoding = "utf-8", newline = "\r\n" if seed % 2 else "\n") as f:
            f.write(text)
    print("wrote", a.count, "folders to", a.out)


if __name__ == "__main__":
    main()
