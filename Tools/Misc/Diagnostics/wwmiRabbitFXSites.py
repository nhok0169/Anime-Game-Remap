"""wwmiRabbitFXSites.py: which sample instructions of a pixel shader RabbitFX's [ShaderRegexMain] reroutes -- the
``--rabbitfx`` sites of wwmiShaderData.py.

    py -3 wwmiRabbitFXSites.py <RabbitFX.ini> <shader> [--asm <the game's disassembly>]

``<shader>`` is the decompiled HLSL (a ``-ps_replace.txt``) or one of the prototype's ``data/*Shader.json``. RabbitFX
matches the game's ASSEMBLY, so the regex runs over ``--asm`` when the game's own disassembly is at hand (3DMigoto's
hunting mark writes it as ``<hash>-ps.txt`` with ``marking_actions = asm``), and otherwise over what ``fxc`` makes of
the HLSL. On Lynae's lower body (a68c6144) the two agree on every site but the two cut-out reads of t8, which only
the game's own assembly has -- so prefer ``--asm``.

Each regex group's sample is mapped to its occurrence among the HLSL's samples of the same register (the decompiler
keeps their order), and printed as ``group=register:occurrence,...`` for ``wwmiShaderData.py --rabbitfx``.
Prints NO MATCH when RabbitFX does not patch the shader at all -- then there is nothing to carry.
"""
import argparse, json, os, re, subprocess, sys, tempfile

import regex

GROUPS = ("t0", "t1", "t2", "t3", "t4", "t5", "t6", "t6d", "t7", "t8", "t8d")
FXC_GLOB = r"C:\Program Files (x86)\Windows Kits\10\bin"
ASM_SAMPLE = re.compile(r"sample\w*\([^)]*\)\([^)]*\)\s+[^,]+,\s*[^,]+,\s*(t\d+)\.")
HLSL_SAMPLE = re.compile(r"\b(t\d+)\.Sample\w*\(")


def findFxc():
    best = None
    for root, _, files in os.walk(FXC_GLOB):
        if "fxc.exe" in files and root.endswith("x64"):
            best = os.path.join(root, "fxc.exe") if best is None or root > os.path.dirname(best) else best
    if best is None:
        sys.exit("no fxc.exe under " + FXC_GLOB + " -- pass --asm")
    return best


def compileAsm(hlsl):
    with tempfile.TemporaryDirectory() as tmp:
        src, out = os.path.join(tmp, "s.hlsl"), os.path.join(tmp, "s.asm")
        open(src, "w", encoding="utf-8").write(hlsl)
        run = subprocess.run([findFxc(), "/nologo", "/T", "ps_5_0", "/E", "main", "/Fc", out, src],
                             capture_output=True, text=True)
        if run.returncode != 0:
            sys.exit("fxc failed:\n" + run.stdout + run.stderr)
        return open(out, encoding="utf-8", errors="replace").read()


def normalise(asm):
    asm = asm.replace("\r\n", "\n")
    start = asm.find("ps_5_0")
    lines = (asm[start:] if start >= 0 else asm).split("\n")
    idx = [i for i, l in enumerate(lines) if l.startswith("dcl_constantbuffer")]
    block = sorted((lines[i] for i in idx), key=lambda l: int(re.search(r"CB(\d+)", l).group(1)))
    for i, l in zip(idx, block):
        lines[i] = l                                  # fxc lists them in reverse, the game ascending
    return "\n".join(l for l in lines if not l.lstrip().startswith("//"))


def main():
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("rabbitfx")
    p.add_argument("shader")
    p.add_argument("--asm")
    args = p.parse_args()

    ini = open(args.rabbitfx, encoding="utf-8", errors="replace").read()
    pattern = re.search(r"^\[ShaderRegexMain\.Pattern\]\s*\n(.*?)\n\s*\n", ini, re.M | re.S).group(1).strip()
    rx = regex.compile(pattern.replace(r"\h", r"[ \t]"), regex.M)

    if args.shader.endswith(".json"):
        hlsl = json.load(open(args.shader, encoding="utf-8"))["ps"]
    else:
        hlsl = open(args.shader, encoding="utf-8", errors="replace").read()
    asm = normalise(open(args.asm, encoding="utf-8", errors="replace").read() if args.asm else compileAsm(hlsl))

    hit = rx.search(asm)
    if hit is None:
        print("NO MATCH: RabbitFX does not patch this shader")
        return

    # the asm's samples by register, in order, against the HLSL's
    asmOrder = {}
    for m in ASM_SAMPLE.finditer(asm):
        asmOrder.setdefault(m.group(1), []).append(m.start())
    sites = []
    for group in GROUPS:
        if hit.group(group) is None:
            continue
        m = ASM_SAMPLE.search(hit.group(group))
        reg = m.group(1)
        occurrence = asmOrder[reg].index(hit.start(group) + m.start())
        hlslCount = sum(1 for h in HLSL_SAMPLE.finditer(hlsl) if h.group(1) == reg)
        if occurrence >= hlslCount:
            sys.exit("the HLSL has %d samples of %s, the assembly more -- the orders disagree" % (hlslCount, reg))
        sites.append("%s=%s:%d" % (group, reg, occurrence))
        print("  %-4s %s" % (group, hit.group(group).strip()), file=sys.stderr)
    print(",".join(sites))


if __name__ == "__main__":
    main()
