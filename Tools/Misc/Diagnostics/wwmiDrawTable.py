#
# ===== wwmiDrawTable =====
#
# The per-draw SLOT TABLE of a character in a WWMI frame dump: for every draw call whose vb0 is the
# character's vertex buffer (Metadata.json's vb0_hash), which component it is (by its start index
# against Metadata's index_offset), the shaders, and what texture is bound in every ps-t slot --
# read off the 3DMigoto log.txt, never reasoned about. This is the WuWa form of the rule in
# Creating Remaps' "A TextureOverride binds registers only for the draw its hash matches": the way to
# know which slot a component's draw reads its diffuse / light map / normal map from is to parse the
# frame, and two skins of one character will disagree about it.
#
#   python wwmiDrawTable.py <FrameAnalysis folder> <vb0 hash> [--metadata <WWMI-Assets/.../Metadata.json>] [--json out.json]
#
# The log lists, per draw call number, the IASetVertexBuffers / VSSetConstantBuffers /
# PSSetShaderResources calls and the DrawIndexed that consumes them; a slot set once stays bound
# for later draws until it is set again, so the table carries the bindings forward the way the
# device does. A texture's `hash` is what the mod's [TextureOverrideTexture] matches on;
# `orig_hash` (when 3DMigoto shows one) is the pre-override hash of a resource a mod replaced.
#

import argparse
import json
import os
import re
import sys
from collections import OrderedDict

DrawPattern = re.compile(r"^(?P<draw>\d{6}) (?P<call>[A-Za-z]+)\((?P<args>.*)\)(?P<tail>.*)$")
SlotPattern = re.compile(r"^\s+(?P<slot>\d+): (?:view=\S+ )?resource=\S+ hash=(?P<hash>[0-9a-f]+)(?: orig_hash=(?P<orig>[0-9a-f]+))?")
StartSlotPattern = re.compile(r"StartSlot:(\d+)")
DrawIndexedPattern = re.compile(r"IndexCount:(\d+), StartIndexLocation:(\d+), BaseVertexLocation:(-?\d+)")


def parseLog(path):
    """Every DrawIndexed of the frame with the state bound at that moment"""
    state = {"vb": {}, "vs-cb": {}, "ps-cb": {}, "ps-t": {}, "vs-t": {}, "vs": None, "ps": None, "ib": None}
    pending = None
    draws = []
    with open(path, "r", encoding = "utf-8", errors = "replace") as f:
        for line in f:
            line = line.rstrip("\r\n")
            match = DrawPattern.match(line)
            if (match is not None):
                call, args, tail = match.group("call"), match.group("args"), match.group("tail")
                pending = None
                if (call == "IASetVertexBuffers"):
                    pending = "vb"
                elif (call == "VSSetConstantBuffers"):
                    pending = "vs-cb"
                elif (call == "PSSetConstantBuffers"):
                    pending = "ps-cb"
                elif (call == "PSSetShaderResources"):
                    pending = "ps-t"
                elif (call == "VSSetShaderResources"):
                    pending = "vs-t"
                elif (call == "IASetIndexBuffer"):
                    pending = "ib"
                elif (call == "VSSetShader"):
                    h = re.search(r"hash=([0-9a-f]+)", tail)
                    state["vs"] = h.group(1) if (h) else None
                elif (call == "PSSetShader"):
                    h = re.search(r"hash=([0-9a-f]+)", tail)
                    state["ps"] = h.group(1) if (h) else None
                elif (call == "DrawIndexed" or call == "DrawIndexedInstanced"):
                    d = DrawIndexedPattern.search(args)
                    draws.append({"draw": int(match.group("draw")), "call": call,
                                  "indexCount": int(d.group(1)) if d else None, "startIndex": int(d.group(2)) if d else None, "baseVertex": int(d.group(3)) if d else None,
                                  "vs": state["vs"], "ps": state["ps"], "ib": state["ib"],
                                  "vb": dict(state["vb"]), "vs-cb": dict(state["vs-cb"]), "ps-cb": dict(state["ps-cb"]), "ps-t": dict(state["ps-t"]), "vs-t": dict(state["vs-t"])})
                continue

            if (pending is None):
                continue
            slot = SlotPattern.match(line)
            if (slot is None):
                if (pending == "ib"):
                    h = re.search(r"hash=([0-9a-f]+)", line)
                    if (h):
                        state["ib"] = h.group(1)
                continue
            value = slot.group("hash") + (f"({slot.group('orig')})" if (slot.group("orig")) else "")
            if (pending == "ib"):
                state["ib"] = value
            else:
                state[pending][int(slot.group("slot"))] = value
    return draws


def componentOf(startIndex, components):
    for i, c in enumerate(components):
        if (int(c["index_offset"]) <= startIndex < int(c["index_offset"]) + int(c["index_count"])):
            return i
    return None


def main():
    parser = argparse.ArgumentParser(description = "the per-draw texture slot table of one character in a WWMI frame dump")
    parser.add_argument("frame", help = "the FrameAnalysis folder (its log.txt is read)")
    parser.add_argument("vb0", help = "the character's vertex buffer hash (Metadata.json's vb0_hash)")
    parser.add_argument("--metadata", default = None, help = "the character's Metadata.json, to name each draw's component by its index range")
    parser.add_argument("--json", default = None, help = "write the table as JSON here too")
    args = parser.parse_args()

    components = []
    if (args.metadata):
        with open(args.metadata, "r", encoding = "utf-8") as f:
            components = json.load(f).get("components") or []

    draws = [d for d in parseLog(os.path.join(args.frame, "log.txt")) if (d["vb"].get(0, "").startswith(args.vb0.lower()))]
    if (not draws):
        raise SystemExit(f"no DrawIndexed with vb0={args.vb0} in {args.frame}")

    rows = []
    for d in draws:
        component = componentOf(d["startIndex"], components) if (components and d["startIndex"] is not None) else None
        rows.append({"draw": d["draw"], "component": component, "startIndex": d["startIndex"], "indexCount": d["indexCount"], "vs": d["vs"], "ps": d["ps"], "ib": d["ib"],
                     "cb3": d["vs-cb"].get(3), "cb4": d["vs-cb"].get(4), "vb": {str(k): v for k, v in sorted(d["vb"].items())},
                     "ps-t": {f"ps-t{k}": v for k, v in sorted(d["ps-t"].items())}, "vs-t": {f"vs-t{k}": v for k, v in sorted(d["vs-t"].items())}})

    slots = sorted({int(k[4:]) for r in rows for k in r["ps-t"]})
    header = f"{'draw':>6} {'comp':>4} {'startIdx':>8} {'count':>6} {'vs':>16} {'ps':>16} {'cb4':>8} " + " ".join(f"{'ps-t' + str(s):>18}" for s in slots)
    print(header)
    for r in rows:
        print(f"{r['draw']:>6} {str(r['component']) if r['component'] is not None else '-':>4} {r['startIndex']:>8} {r['indexCount']:>6} {r['vs'] or '-':>16} {r['ps'] or '-':>16} {r['cb4'] or '-':>8} "
              + " ".join(f"{r['ps-t'].get('ps-t' + str(s), '-'):>18}" for s in slots))

    # the per-component summary: which (ps shader, slot) -> texture, over every pass that drew it
    print()
    print("per component, per pixel shader: the textures each slot binds")
    byComponent = OrderedDict()
    for r in rows:
        byComponent.setdefault(r["component"], OrderedDict()).setdefault(r["ps"], OrderedDict())
        for slot, value in r["ps-t"].items():
            byComponent[r["component"]][r["ps"]].setdefault(slot, set()).add(value)
    for component, shaders in byComponent.items():
        print(f"  component {component}:")
        for ps, slotMap in shaders.items():
            print(f"    ps={ps}: " + ", ".join(f"{slot}={'/'.join(sorted(values))}" for slot, values in slotMap.items()))

    if (args.json):
        with open(args.json, "w", encoding = "utf-8") as f:
            json.dump(rows, f, indent = 1)
        print(f"wrote {args.json}")


if (__name__ == "__main__"):
    main()
