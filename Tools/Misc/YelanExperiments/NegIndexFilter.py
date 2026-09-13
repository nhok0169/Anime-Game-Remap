"""
Negative-index split of this Yelan mod onto YelanTranquil's three components.

Every component (Body / Bang / Eye) is fed the WHOLE mod -- the mod's own Position.buf,
Texcoord.buf and .ib files -- plus its own RemapBlend.buf, remapped with only that component's
rows of the Yelan -> YelanTranquil vertex group remap. A source bone belonging to another
component is written as the negative sentinel (-index-1), which makes the game's skinning
collapse the vertices that are not this component's. Nothing is filtered, no index buffer is
rewritten; the cost is a triangle straddling two components stretching between a kept vertex
and a collapsed one.

Run from anywhere:

    py -3 NegIndexFilter.py
    py -3 NegIndexFilter.py --sheet "V5.7 Yelan to YelanTranquil"     (the hand-made draft instead)
    py -3 NegIndexFilter.py --library                                  (the installed API's rows)

Writes NegIndex/<Component>/yelan<Component>RemapBlend.buf and DISABLEDYelanTranquilNegIndex.ini
beside this script; the .ini is self-contained (its own resource names) and sits next to the
mod's own .ini. Remove the DISABLED prefix to try it (and disable any other YelanTranquil override).
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
            "out": "NegIndex",
            "ini": "DISABLEDYelanTranquilNegIndex.ini"}

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
    args = makeArgParser(__doc__, "negative", Defaults).parse_args()
    runSplit("negative", args, Layouts, Defaults["fromName"])
