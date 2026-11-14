# TWO TWEAKS OF A FIX, BY EDITING ITS LIST (Tools/Misc/Prototypes/editList.py): Lynae -> LynaePeppermint with
#   1. component 0 drawn through slot 3 -- one entry of the slot remap changed
#   2. her body all blue -- one step added: the diffuse each body part's texture list binds, replaced by a blue texture
#
#   py -3 editListTweaks.py <mod folder>
import os
import sys

os.environ["AGREMAP_LIST_PAIR"] = "Lynae,LynaePeppermint"
import editList as L    # noqa: E402

FRB = L.FRB

# 1. component 0 onto slot 3
L.SLOT_REMAP.remap = {**L.SLOT_REMAP.remap, (0,) + L.SRC(0): [(0,) + L.DST(3) + (L.RENAME,)]}

# 2. the upper and lower body's diffuse, blue -- after the texture lists are written, before they are remapped
blue = FRB.ResRegCollect({(0,) + L.LISTG(c): [reg for reg, role in L.REGISTERS[c] if role.endswith("Diffuse")] for c in (3, 4)},
                         {"blue": FRB.TexReplace((0, "", "blueBody"),
                                                 FRB.TexEditor(filters = [FRB.CppColourReplaceFilter(FRB.CppColour(0, 0, 255, 255), replaceAlpha = False)]),
                                                 resSubType = "BlueBody")})
L.LIST_OF_EDITS.insert(L.LIST_OF_EDITS.index(L.SLOT_REMAP), blue)

for path, error in L.fix(sys.argv[1]).stats.ini.skipped.items():
    print("SKIPPED", path, error)
