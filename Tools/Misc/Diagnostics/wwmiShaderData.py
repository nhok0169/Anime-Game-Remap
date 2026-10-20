"""wwmiShaderData.py: turn a pixel shader captured from the game (3DMigoto's decompiled HLSL) and the constant
buffers its draw read (a frame dump's ``ps-cbN`` files) into the data a ``WWMIFixerConfig::Shader``
holds -- a generated C++ file for the compiled fix and a JSON file for the Python prototype.

    py -3 wwmiShaderData.py --hlsl <hash>-ps_replace.txt --cb ps-cb4=<dump ps-cb4 .buf>:136 \
        --name LynaeFilm --func lynaeFilmShader --header AGRemapCore/data/IniFixData/Lynae/LynaeFilmShader.h \
        --label "Lynae's jacket pass, 404f5464fdffc665" --cpp <out .cpp> --json <out .json>

``--cb reg=path:rows`` keeps the first ``rows`` 16-byte rows of the dump: the rows the shader declares (its
``cbuffer cbN { float4 cbN[rows]; }``). Rows past those are not the material's -- a dump's buffer carries whatever
the frame left there and differs dump to dump. The rows are written as their 32-bit words, so nothing is rounded.

Capturing the HLSL: 3DMigoto's hunting mode marks the selected shader into ``ShaderFixes`` (``marking_actions =
hlsl``). Capture it with every third-party shader mod (RabbitFX, ...) parked: a patched shader is not the game's,
and one decompiles to assembly only. Move the file OUT of ``ShaderFixes`` straight away, or 3DMigoto loads it as a
replacement. The assembly comment block 3DMigoto appends is dropped here, and so is its dated first line.

The DECOMPILER IS REPAIRED on the way: an integer bit operation on a constant-buffer value (``bfi``, ``ubfe`` ...
``cb2[23].w`` in the assembly) comes out as ``(uint)cb2[23].w`` -- a float-to-integer CONVERSION, which turns a
value stored as raw integer bits (1, printed as the float 1.4e-45) into 0 -- where the assembly reads the bits.
Every such cast becomes ``asuint(...)``. Without it Lynae's shaders wrote the wrong shading model into the
G-buffer (``o2.w`` 12 where her own draw writes 44), and the deferred lighting shaded every part of hers with
the wrong model: a glow on gold, whiter gauze, flat lime on some materials.

    py -3 wwmiShaderData.py --from-json <old .json> --func ... --header ... --label ... --cpp <out> --json <out>

rebuilds both files from an earlier JSON (its HLSL repaired, its buffers kept) when the capture is gone.
"""
import argparse, json, os, re, struct

CREDITS = """// ##### Credits

// ===== Anime Game Remap (AG Remap) =====
// Authors: Albert Gold#2696, NK#1321
//
// if you used it to remap your mods pls give credit for "Albert Gold#2696" and "Nhok0169"
// Special Thanks:
//   nguen#2011 (for support)
//   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
//   HazrateGolabi#1364 (for being awesome, and improving the code)

// ##### EndCredits
"""

PIECE = 12000        # MSVC refuses one string literal past ~16 KB, so the source is appended in pieces
DELIM = "hlsl"


def readHlsl(path, label):
    text = open(path, "rb").read().decode("utf-8").replace("\r\n", "\n")
    lines = text.split("\n")
    if lines and lines[0].startswith("// ---- Created with 3Dmigoto"):
        lines = lines[1:]
    for i, line in enumerate(lines):
        if line.startswith("/*~~~~"):
            lines = lines[:i]
            break
    body = "\n".join(lines).strip("\n")
    if ")" + DELIM + '"' in body:
        raise SystemExit("the shader holds the raw-string delimiter")
    return "// " + label + " -- decompiled by 3DMigoto\n" + body + "\n"


CB_CAST = re.compile(r"\((?:uint|int)\)(cb\d+\[\d+\]\.[xyzw]+)")


def repairCasts(hlsl):
    """An integer read of a constant-buffer value is its raw bits (``asuint``), not a conversion."""
    return CB_CAST.sub(r"asuint(\1)", hlsl)


# ---- RabbitFX, carried into a custom shader ----
#
# RabbitFX (8.3.2) patches a game pixel shader by regex ([ShaderRegexMain]) so a mod can hand it maps: its
# SetTextures binds a mod's Diffuse / Lightmap / Normalmap / Materialmap / Cutoutmap / Specialmap at ps-t60..t65
# and sets routing flags at IniParams 187 from what the draw has bound at ps-t0..t3, and the patched shader reads
# one of t60..t65 in place of the texture at each of up to ten sample instructions ("sites", the regex's groups
# t0..t8). Its Run binds a GlowMap / FXMap at ps-t50 / t51 with a hue / saturation / value / brightness at
# IniParams 217, which the patch blends over the shader's o3 (colour) and o0 (emission).
#
# A CustomShader is never patched (the regex runs on the game's shaders only), so a fix that draws a part with
# the source character's own shader loses all of it: her own card shows the mod's maps routed by RabbitFX, the
# remap shows the plain game routing. --rabbitfx writes the same selection into the HLSL: every site samples its
# own texture and the one RabbitFX would route there, and picks as the patch does -- by the flags and by which of
# t60..t65 are bound -- so a draw that has no RabbitFX bindings is unchanged. The menu tweak RabbitFX also
# carries is gated on its censor key (IniParams 172.x = $menu * $censor) and is not carried.
#
# The sites are the regex's groups, given as `group=register:occurrence` (the N-th sample of that register in
# the HLSL, from 0). Tools/Misc/Diagnostics/wwmiRabbitFXSites.py reads them off a shader by running RabbitFX's
# own pattern over its assembly.

RFX_GROUPS = ("t0", "t1", "t2", "t3", "t4", "t5", "t6", "t6d", "t7", "t8", "t8d")


def parseRabbitFXSites(spec):
    sites = {}
    for item in spec.split(","):
        group, site = item.strip().split("=")
        reg, occurrence = site.split(":")
        if group not in RFX_GROUPS:
            raise SystemExit("unknown RabbitFX site group " + group)
        sites[group] = (reg.strip(), int(occurrence))
    return sites


def _rfxSelections(sites):
    """The HLSL computing, per site, which register RabbitFX reads there: 0 for the shader's own, 60..65, or -1
    where RabbitFX's t3 site drops the instruction (the shader's own sample is taken there -- see applyRabbitFX)."""
    has3, has2 = "t3" in sites, "t2" in sites
    dots = "dot(rfxF, 1)" if has3 else ("dot(rfxF.xyz, 1)" if has2 else "dot(rfxF.xy, 1)")
    out = ["  // ---- RabbitFX's routing (see wwmiShaderData.py --rabbitfx) ----",
           "  float4 rfxF = IniParams.Load(int2(187, 0));",
           "  int4 rfxT = (int4)rfxF;",
           "  bool rfxMany = 1.0 < " + dots + ";",
           "  uint rfxW, rfxH;"]
    for k in range(60, 66):
        out.append("  t%d.GetDimensions(rfxW, rfxH); bool rfxB%d = (rfxW + rfxH) != 0;" % (k, k))
    pick = {
        "t0": "rfxT.x ? (rfxMany ? (rfxB62 ? 62 : 0) : (rfxB60 ? 60 : 0)) : (rfxB61 ? 61 : 0)",
        "t1": "rfxT.y ? (rfxB60 ? 60 : 0) : (rfxB61 ? 61 : 0)",
        "t2": "rfxT.z ? (rfxB60 ? 60 : 0) : (rfxT.w ? (" + ("rfxB63 ? 63 : 0" if has3 else "rfxB61 ? 61 : 0") + ") : 0)",
        "t3": "rfxT.w ? (rfxMany ? (rfxB60 ? 60 : 0) : (rfxB63 ? 63 : 0)) : (rfxMany ? (rfxT.x == 0 ? (rfxB62 ? 62 : 0) : -1) : 0)",
        "t4": "rfxB61 ? 61 : 0",
        "t5": "rfxB63 ? 63 : 0",
        "t6": "rfxB60 ? 64 : 0",      # RabbitFX tests the Diffuse's binding here, and reads the Cutoutmap
        "t6d": "rfxB60 ? 64 : 0",
        "t7": "rfxB65 ? 65 : 0",
        "t8": "rfxB64 ? 64 : 0",
        "t8d": "rfxB64 ? 64 : 0",
    }
    for group in RFX_GROUPS:
        if group in sites:
            out.append("  int rfxS_%s = %s;" % (group, pick[group]))
    return "\n".join(out) + "\n"


SAMPLE_CALL = re.compile(r"\b(t\d+)\.(Sample\w*)\(")


def _callEnd(text, open_):
    depth = 0
    for i in range(open_, len(text)):
        if text[i] == "(":
            depth += 1
        elif text[i] == ")":
            depth -= 1
            if depth == 0:
                return i + 1
    raise SystemExit("unbalanced call in the HLSL")


def _rfxGlow(texcoord):
    """RabbitFX's Run (GlowMap at t50, FXMap at t51), transcribed from its patch's assembly; applied where the
    patch applies it, over the shader's last o3 / o0."""
    return """  // ---- RabbitFX's glow (CommandList\\RabbitFX\\Run), see wwmiShaderData.py --rabbitfx ----
  {
    float4 rfxIni = IniParams.Load(int2(172, 0));
    if (rfxIni.y != 0) {
      uint rfxGW, rfxGH;
      t51.GetDimensions(rfxGW, rfxGH);
      if (rfxGW + rfxGH != 0) {
        float4 rfxMask = t51.Load(int3((int2)(float2(rfxGW, rfxGH) * frac(%(uv)s.xy)), 0), int2(1, 1));
        if (rfxMask.w == 0) rfxMask = t51.Sample(s0_s, %(uv)s.xy);
        if (rfxMask.w < 1.0) discard;
      }
      t50.GetDimensions(rfxGW, rfxGH);
      if (rfxGW + rfxGH != 0) {
        float4 rfxC = t50.Load(int3((int2)(float2(rfxGW, rfxGH) * frac(%(uv)s.xy)), 0), int2(1, 1));
        if (rfxC.w == 0) rfxC = t50.Sample(s0_s, %(uv)s.xy);
        float4 rfxShift = IniParams.Load(int2(217, 0));
        if (rfxC.w != 0) {
          float4 p = (rfxC.y < rfxC.z) ? float4(rfxC.z, rfxC.y, -1.0, 0.666667) : float4(rfxC.y, rfxC.z, 0.0, -0.333333);
          float4 q = (rfxC.x < p.x) ? float4(p.x, p.y, p.w, rfxC.x) : float4(rfxC.x, p.y, p.z, p.x);
          float chroma = q.x - min(q.y, q.w);
          float hue = q.z + (q.w - q.y) / (chroma * 6.0);
          float shifted = rfxShift.x + abs(hue);
          shifted = (shifted >= -shifted) ? frac(abs(shifted)) : -frac(abs(shifted));
          float3 rgb = saturate(abs(frac(shifted + float3(1.0, 0.666667, 0.333333)) * 6.0 - 3.0) - 1.0) - 1.0;
          float value = max(q.x + rfxShift.z, 0.0);
          float sat = max(chroma / q.x + rfxShift.y, 0.0);
          float3 colour = (sat * rgb + 1.0) * value;
          o3.xyz = colour;
          o0.xyz = colour * rfxC.w * rfxShift.w;
        }
      }
    }
  }
""" % {"uv": texcoord}


def applyRabbitFX(hlsl, sites, texcoord="v2"):
    """Carries RabbitFX's routing (and glow) into the shader -- see the note above."""
    # every sample call, by register, in source order
    calls = []
    for m in SAMPLE_CALL.finditer(hlsl):
        calls.append((m.start(), _callEnd(hlsl, m.end() - 1), m.group(1), m.group(2)))
    seen = {}
    located = {}
    for start, end, reg, method in calls:
        n = seen.get(reg, 0)
        seen[reg] = n + 1
        for group, (want, occurrence) in sites.items():
            if want == reg and occurrence == n:
                located[group] = (start, end, reg, method)
    missing = sorted(set(sites) - set(located))
    if missing:
        raise SystemExit("RabbitFX sites not found in the HLSL: " + ", ".join(missing))

    # rewrite from the back so the earlier offsets stay valid
    for group, (start, end, reg, method) in sorted(located.items(), key=lambda kv: -kv[1][0]):
        call = hlsl[start:end]
        rest = call[len(reg):]                       # ".Sample(...)"
        alts = {"t0": (60, 61, 62), "t1": (60, 61), "t2": (60, 61, 63), "t3": (60, 62, 63), "t4": (61,), "t5": (63,),
                "t6": (64,), "t6d": (64,), "t7": (65,), "t8": (64,), "t8d": (64,)}[group]
        expr = call
        for k in reversed(alts):
            expr = "(rfxS_%s == %d ? t%d%s : %s)" % (group, k, k, rest, expr)
        # (RabbitFX's t3 site drops the instruction in one branch, leaving whatever the register held -- an
        # undefined value, so that branch -- rfxS_t3 == -1 -- takes the shader's own sample here)
        hlsl = hlsl[:start] + expr + hlsl[end:]

    # declarations
    decls = []
    for k in (50, 51, 60, 61, 62, 63, 64, 65):
        if not re.search(r"\bt%d\s*:\s*register\(t%d\)" % (k, k), hlsl):
            decls.append("Texture2D<float4> t%d : register(t%d);" % (k, k))
    if not re.search(r"\bIniParams\s*:\s*register\(t120\)", hlsl):
        decls.append("Texture1D<float4> IniParams : register(t120);")
    if not re.search(r"\bs0_s\s*:\s*register\(s0\)", hlsl):
        decls.append("SamplerState s0_s : register(s0);")
    main = hlsl.find("void main(")
    if main < 0:
        raise SystemExit("no main() in the HLSL")
    hlsl = hlsl[:main] + "\n".join(decls) + "\n\n" + hlsl[main:]

    # the selections at the top of main, the glow before its last return
    body = hlsl.find("{", hlsl.find("void main("))
    hlsl = hlsl[:body + 1] + "\n" + _rfxSelections(sites) + hlsl[body + 1:]
    ret = hlsl.rfind("return;")
    if ret < 0:
        raise SystemExit("no return in main()")
    lineStart = hlsl.rfind("\n", 0, ret) + 1
    hlsl = hlsl[:lineStart] + _rfxGlow(texcoord) + hlsl[lineStart:]
    return hlsl


def readCb(spec):
    reg, rest = spec.split("=", 1)
    path, rows = rest.rsplit(":", 1)
    raw = open(path, "rb").read()
    rows = int(rows)
    if len(raw) < rows * 16:
        raise SystemExit(path + " holds " + str(len(raw) // 16) + " rows, not " + str(rows))
    return reg, list(struct.unpack("<" + str(rows * 4) + "I", raw[:rows * 16]))


def cppText(args, hlsl, cbs):
    out = [CREDITS,
           "// GENERATED by Tools/Misc/Diagnostics/wwmiShaderData.py -- do not edit by hand.",
           "// " + args.label + ": its decompiled pixel shader and the constant buffers its draw read.",
           "",
           '#include "' + args.header + '"',
           "",
           "#include <cstdint>",
           "#include <string>",
           "#include <vector>",
           "",
           "",
           "namespace AGRemapCore {",
           "    WWMIFixerConfig::Shader " + args.func + "() {",
           "        WWMIFixerConfig::Shader shader;",
           '        shader.name = "' + args.name + '";',
           ""]
    for i in range(0, len(hlsl), PIECE):
        out.append('        shader.ps += R"' + DELIM + "(" + hlsl[i:i + PIECE] + ")" + DELIM + '";')
    for reg, words in cbs:
        out.append("")
        out.append('        shader.constantBuffers["' + reg + '"] = std::vector<std::uint32_t>{')
        for i in range(0, len(words), 8):
            out.append("            " + ", ".join("0x%08xu" % w for w in words[i:i + 8]) + ",")
        out.append("        };")
    out += ["",
            "        return shader;",
            "    }",
            "}",
            ""]
    return "\n".join(out)


def main():
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("--hlsl")
    p.add_argument("--from-json", dest="fromJson")
    p.add_argument("--cb", action="append", default=[])
    p.add_argument("--name")
    p.add_argument("--func", required=True)
    p.add_argument("--header", required=True)
    p.add_argument("--label", required=True)
    p.add_argument("--cpp")
    p.add_argument("--json")
    p.add_argument("--rabbitfx", help="carry RabbitFX's routing and glow into the shader: group=register:occurrence,...")
    args = p.parse_args()

    if args.fromJson:
        old = json.load(open(args.fromJson, encoding="utf-8"))
        hlsl = repairCasts(old["ps"])
        cbs = list(old["constantBuffers"].items())
        args.name = args.name or old["name"]
        if old.get("rabbitfx") and args.rabbitfx:
            raise SystemExit("the JSON's shader already carries RabbitFX (" + old["rabbitfx"] + ")")
        args.rabbitfx = args.rabbitfx or None
        carried = old.get("rabbitfx")
    else:
        if not args.hlsl or not args.name:
            raise SystemExit("--hlsl and --name are needed without --from-json")
        hlsl = repairCasts(readHlsl(args.hlsl, args.label))
        cbs = [readCb(spec) for spec in args.cb]
        carried = None
    if args.rabbitfx:
        hlsl = applyRabbitFX(hlsl, parseRabbitFXSites(args.rabbitfx))
        carried = args.rabbitfx
    if args.cpp:
        open(args.cpp, "wb").write(cppText(args, hlsl, cbs).replace("\n", "\r\n").encode("utf-8"))
        print("wrote", args.cpp)
    if args.json:
        data = {"name": args.name, "ps": hlsl, "constantBuffers": {reg: words for reg, words in cbs}}
        if carried:
            data["rabbitfx"] = carried
        open(args.json, "w", encoding="utf-8", newline="\n").write(json.dumps(data, indent=1) + "\n")
        print("wrote", args.json)


if __name__ == "__main__":
    main()
