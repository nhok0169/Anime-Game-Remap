# Synthetic variants of a WWMI identity mod, to exercise more of the WWMI fixer than the identity does.
#   py -3 wwmiSynth.py <identity folder> <TextureUsage.json> <out folder> <variant: bound|toggle>
#  bound : every component section binds ps-tK = ResourceTextureM itself (the first hash the dump saw at that
#          slot that the mod ships), then runs RabbitFX's SetTextures
#  toggle: as bound, but ps-t0 is toggled by $swapvar between the texture and a renamed copy of it, and the
#          component's draw is split into two ranges, the second one drawn only while $swapvar == 0
import json, os, re, shutil, sys

src, usagePath, out, variant = sys.argv[1:5]
shutil.rmtree(out, ignore_errors=True)
shutil.copytree(src, out)
ini = os.path.join(out, "mod.ini")
text = open(ini, encoding="utf-8").read().replace("\r\n", "\n")
usage = json.load(open(usagePath, encoding="utf-8"))

resOfHash = {}
for m in re.finditer(r"\[(ResourceTexture\d+)\]\nfilename = Textures/Components-0 t=([0-9a-f]{8})\.dds", text):
    resOfHash[m.group(2)] = m.group(1)

extraSections = []
alt = 0
def bindings(component):
    global alt
    lines = []
    slots = usage.get(f"Component {component}", {})
    for reg in sorted(slots, key=lambda r: int(r[4:]) if r[4:].isdigit() else 99):
        for entry in slots[reg]:
            h = entry.split("-")[0].lower()
            if h in resOfHash:
                lines.append((reg, resOfHash[h], h))
                break
    return lines

def block(component, indent):
    global alt
    b = bindings(component)
    L = []
    for reg, res, h in b:
        if variant == "toggle" and reg == "ps-t0":
            altRes = f"ResourceTextureAlt{alt}"
            altFile = f"Textures/Alt{alt} t={h}.dds"
            shutil.copyfile(os.path.join(out, "Textures", f"Components-0 t={h}.dds"), os.path.join(out, altFile))
            extraSections.append(f"[{altRes}]\nfilename = {altFile}\n")
            alt += 1
            L += [f"{indent}if $swapvar == 0", f"{indent}    {reg} = {res}", f"{indent}else", f"{indent}    {reg} = {altRes}", f"{indent}endif"]
        else:
            L.append(f"{indent}{reg} = {res}")
    L.append(f"{indent}run = CommandList\\RabbitFX\\SetTextures")
    return L

def fixSection(m):
    component = int(m.group(1))
    body = m.group(0)
    anchor = "        run = CommandListTriggerResourceOverrides\n"
    assert body.count(anchor) == 1
    body = body.replace(anchor, anchor + "\n".join(block(component, "        ")) + "\n")
    if variant == "toggle":
        d = re.search(r"        drawindexed = (\d+), (\d+), 0\n", body)
        count, first = int(d.group(1)), int(d.group(2))
        half = (count // 6) * 3
        if half > 0:
            body = body.replace(d.group(0), f"        drawindexed = {half}, {first}, 0\n        if $swapvar == 0\n            drawindexed = {count - half}, {first + half}, 0\n        endif\n")
    return body

text = re.sub(r"\[TextureOverrideComponent(\d+)\]\n(?:.*\n)*?\n(?=\[|;)", fixSection, text)
if variant == "toggle":
    text = text.replace("[Constants]\n", "[Constants]\nglobal persist $swapvar = 0\n", 1)
    text += "\n[KeySwap]\ncondition = $object_detected\nkey = VK_RIGHT\ntype = cycle\n$swapvar = 0,1\n\n" + "\n".join(extraSections)
open(ini, "w", encoding="utf-8", newline="\r\n").write(text)
print(out, variant, "alt textures", alt)
