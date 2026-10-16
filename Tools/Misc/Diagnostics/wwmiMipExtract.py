"""wwmiMipExtract.py: get a game texture's FULL mip chain out of the game, which a 3DMigoto frame dump never has

A frame dump writes mip 0 of every texture, so a download folder built from dumps has single-mip textures. That is
harmless for a texture a shader samples near mip 0, and wrong for one it reads with ``SampleLevel`` at a computed
level -- a prefiltered matcap or foil, whose game mips are authored per level (Lynae's film foil: the alpha her
shader blends toward is 168 at mip 0 and ~80 at mips 3-5), and which a box filter does not reproduce.

Two steps:

  py -3 wwmiMipExtract.py mod <out folder> <name>:<ps hash>:<register>:<size> [...]
      writes a temporary mod: a ShaderOverride on each pixel shader renders every level of the texture it has at that
      register (``Load(int3(xy, level))``, into an R8G8B8A8_UNORM_SRGB target per level) and ``dump = dds`` s it.
      Copy the folder into ``WWMI/Mods``, take a frame dump with a draw of that shader on screen (GameView's ``dump``),
      and REMOVE the folder again.

  py -3 wwmiMipExtract.py assemble <frame dump folder> <name> <out.dds> [<old single-mip .dds>]
      one DDS (R8G8B8A8_UNORM_SRGB, every level) from the dumped levels; with the old download, says whether its mip 0
      is byte-equal to the extracted one (it should be: that is the losslessness check)

Three 3DMigoto traps this works around: a resource assigned by ``= ref`` needs its own ``[Resource...]`` section or the
line is ignored without a word; a render target given only a description is never created (each level is backed by a
blank ``.dds`` of its size, with ``bind_flags = render_target shader_resource``); ``dump = dds <resource>`` is what
writes DDS (``analyse_options`` alone gave JPG).
"""
import glob
import os
import struct
import sys

import numpy as np


def ddsHeader(size, mips):
    hdr = bytearray(128)
    struct.pack_into("<4sIIIIIII", hdr, 0, b"DDS ", 124, 0x1 | 0x2 | 0x4 | 0x1000 | 0x8 | (0x20000 if mips > 1 else 0),
                     size, size, size * 4, 0, mips)
    struct.pack_into("<II4s", hdr, 76, 32, 0x4, b"DX10")
    struct.pack_into("<I", hdr, 108, 0x1000 | ((0x400000 | 0x8) if mips > 1 else 0))
    return bytes(hdr) + struct.pack("<IIIII", 29, 3, 0, 1, 0)     # R8G8B8A8_UNORM_SRGB, Texture2D


def writeMod(out, specs):
    os.makedirs(out, exist_ok=True)
    with open(os.path.join(out, "mipvs.hlsl"), "w") as f:
        f.write("void main(uint id : SV_VertexID, out float4 pos : SV_Position0)\n{\n"
                "  float2 uv = float2((id << 1) & 2, id & 2);\n"
                "  pos = float4(uv * float2(2, -2) + float2(-1, 1), 0, 1);\n}\n")
    with open(os.path.join(out, "mipps.hlsl"), "w") as f:
        f.write("Texture1D<float4> IniParams : register(t120);\nTexture2D<float4> src : register(t100);\n\n"
                "void main(float4 pos : SV_Position0, out float4 o0 : SV_Target0)\n{\n"
                "  int level = (int)IniParams.Load(int2(0, 0)).x;\n"
                "  o0 = src.Load(int3((int2)pos.xy, level));\n}\n")

    byPs = {}
    for name, ps, reg, size in specs:
        byPs.setdefault(ps, []).append((name, reg, int(size)))

    lines = []
    for ps, items in byPs.items():
        lines += ["[ShaderOverrideMip%s]" % ps, "hash = %s" % ps, "local $savedx = x"]
        for name, reg, size in items:
            lines.append("Resource%sSrc = ref %s" % (name, reg))
            level, s = 0, size
            while s >= 1:
                lines += ["x = %d" % level, "run = CustomShader%sMip%d" % (name, level),
                          "dump = dds Resource%sMip%d" % (name, level)]
                level, s = level + 1, s // 2
        lines += ["x = $savedx", ""]

    for name, ps, reg, size in specs:
        lines += ["[Resource%sSrc]" % name, ""]
        level, s = 0, int(size)
        while s >= 1:
            blank = "blank%d.dds" % s
            if not os.path.exists(os.path.join(out, blank)):
                with open(os.path.join(out, blank), "wb") as f:
                    f.write(ddsHeader(s, 1) + bytes(s * s * 4))
            lines += ["[Resource%sMip%d]" % (name, level), "filename = %s" % blank, "format = R8G8B8A8_UNORM_SRGB",
                      "bind_flags = render_target shader_resource", "",
                      "[CustomShader%sMip%d]" % (name, level), "vs = mipvs.hlsl", "ps = mipps.hlsl",
                      "hs = null", "ds = null", "gs = null", "blend = disable", "cull = none", "depth_enable = false",
                      "stencil_enable = false", "scissor_enable = false", "topology = triangle_list",
                      "ps-t100 = Resource%sSrc" % name, "run = BuiltInCommandListUnbindAllRenderTargets",
                      "o0 = set_viewport Resource%sMip%d" % (name, level), "Draw = 3, 0", ""]
            level, s = level + 1, s // 2

    with open(os.path.join(out, "mips.ini"), "w", newline="\r\n") as f:
        f.write("\n".join(lines))
    print("wrote", os.path.join(out, "mips.ini"), "for", len(specs), "texture(s)")


def assemble(dumpFolder, name, out, old=None):
    levels = []
    while True:
        found = glob.glob(os.path.join(dumpFolder, "*resource%smip%d.dds" % (name.lower(), len(levels))))
        if not found:
            break
        data = open(found[0], "rb").read()
        height, width = struct.unpack_from("<II", data, 12)
        levels.append(np.frombuffer(data[148:148 + width * height * 4], np.uint8).reshape(height, width, 4))
    if not levels:
        sys.exit("no dumped levels of %s in %s" % (name, dumpFolder))

    if old is not None:
        from PIL import Image
        ref = np.asarray(Image.open(old).convert("RGBA")).astype(int)
        diff = np.abs(ref - levels[0].astype(int))
        print("mip 0 against", os.path.basename(old), ": max difference", diff.max())

    with open(out, "wb") as f:
        f.write(ddsHeader(levels[0].shape[0], len(levels)) + b"".join(level.tobytes() for level in levels))
    print(name, ":", len(levels), "levels, alpha means", [int(level[..., 3].mean()) for level in levels])


if __name__ == "__main__":
    if len(sys.argv) >= 4 and sys.argv[1] == "mod":
        writeMod(sys.argv[2], [spec.split(":") for spec in sys.argv[3:]])
    elif len(sys.argv) >= 5 and sys.argv[1] == "assemble":
        assemble(sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5] if len(sys.argv) > 5 else None)
    else:
        sys.exit(__doc__)
