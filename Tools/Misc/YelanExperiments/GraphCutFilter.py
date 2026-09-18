"""
Graph-cut split of this Yelan mod onto YelanTranquil's three components.

Per component (Body / Bang / Eye), with R = the source vertex groups that component's rows of the
Yelan -> YelanTranquil remap cover:
  1. keep the triangles whose vertices' vertex groups are all in R      (--mode strict, default)
  2. V = the vertices those triangles reference
  3. filter Position.buf / Texcoord.buf / Blend.buf to the lines in V -- an .ib index IS the line
     number in every .buf, so one vertex set filters all three
  4. renumber the kept triangles into V and write one .ib per mod object
  5. remap the kept blend lines into the component's bones (no negative sentinel can occur)

--mode majority is the suggested variant: a vertex belongs to the component holding most of its
weight and a triangle to the component most of its vertices belong to, so every triangle is drawn
exactly once and the seams close instead of leaving a one-triangle gap; weights on foreign bones
are dropped and the rest renormalised, and a vertex dragged into a component it has no bone in is
skinned to the bone its triangle neighbours use there.

Run from anywhere:

    py -3 GraphCutFilter.py
    py -3 GraphCutFilter.py --mode relaxed      (a triangle is kept when 2 of its 3 vertices are the component's)
    py -3 GraphCutFilter.py --mode majority
    py -3 GraphCutFilter.py --sheet "V5.7 Yelan to YelanTranquil"     (the hand-made draft instead)

Writes GraphCut/<Component>/yelan<Component>{Position,Texcoord,RemapBlend}.buf + .ib files and
DISABLEDYelanTranquilGraphCut.ini beside this script (self-contained; sits next to the mod's own
.ini). Remove the DISABLED prefix to try it (and disable any other YelanTranquil override).
"""

import os
import sys

# the tool this rides on: Tools/VGRemapFinder in the Anime Game Remap repo
ToolSrc = r"E:\Computer\Games\Genshin\Repos\Repos\Fix-Raiden-Boss\Tools\VGRemapFinder\src"
if (ToolSrc not in sys.path):
    sys.path.insert(0, ToolSrc)

from VGRemapFinder.ComponentSplit import IniLayout, makeArgParser, runSplit   # noqa: E402

Here = os.path.dirname(os.path.abspath(__file__))

Defaults = {"mod": Here,
            "prefix": "yelan",
            "hashJson": r"E:\Computer\Downloads\YelanTranquil\YelanTranquil\hash.json",
            "draft": r"E:\Computer\Games\Genshin\Repos\Repos\Fix-Raiden-Boss\Data\RemapDrafts\YelanRemapDraft.xlsx",
            "sheet": "V5.7 Yelan to Tranquil (tool)",      # same rows as the library's Yelan -> YelanTranquil table
            "fromName": "Yelan",
            "toName": "YelanTranquil",
            "out": "GraphCut",
            "ini": "DISABLEDYelanTranquilGraphCut.ini"}

# What each YelanTranquil component's draws take, from the experiments in 'yelan copy.ini'.
#   Body : ps-t0 normal map, ps-t1 diffuse, ps-t2 light map, ORFix   (slot C of the Body has no normal map)
#   Bang : ps-t1 diffuse, ps-t2 light map, ORFix
#   Eye  : ps-t0 diffuse, ps-t1 light map, NNFix
# The mod objects (Head, Body, ...) are drawn in the component's slots in rank order: Head in the
#   first slot (match_first_index 0), Body in the second, ...; an empty mod object is never drawn.
ORFix = "CommandList\\global\\ORFix\\ORFix"
NNFix = "CommandList\\global\\ORFix\\NNFix"
Layouts = {"Body": IniLayout({"A": [("ps-t0", "NormalMap"), ("ps-t1", "Diffuse"), ("ps-t2", "LightMap")],
                              "B": [("ps-t0", "NormalMap"), ("ps-t1", "Diffuse"), ("ps-t2", "LightMap")],
                              "C": [("ps-t0", "Diffuse"), ("ps-t1", "LightMap")]}, ORFix),
           "Bang": IniLayout([("ps-t1", "Diffuse"), ("ps-t2", "LightMap")], ORFix),
           "Eye": IniLayout([("ps-t0", "Diffuse"), ("ps-t1", "LightMap")], NNFix)}


if (__name__ == "__main__"):
    args = makeArgParser(__doc__, "graphcut", Defaults).parse_args()
    if (args.mode != "strict" and args.out == Defaults["out"]):
        args.out = "GraphCut" + args.mode.capitalize()
        args.ini = f"DISABLEDYelanTranquilGraphCut{args.mode.capitalize()}.ini"
    runSplit("graphcut", args, Layouts, Defaults["fromName"], mode = args.mode)
