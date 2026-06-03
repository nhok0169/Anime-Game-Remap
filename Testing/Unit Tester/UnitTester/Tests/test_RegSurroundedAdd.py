import sys
from ordered_set import OrderedSet

from .baseIniFileTest import BaseIniFileTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class RegSurroundedAddTest(BaseIniFileTest):
    @classmethod
    def setUpClass(cls):
        super().setUpClass()

        cls._parser = None
        cls._fixer = None

    def createParser(self):
        self._parser = FRB.GIMIParser(self._iniFile, modObjs = OrderedSet([("", "blend"), ("head", ""), ("body", "")]))
        
    def createFixer(self):
        self._fixer = FRB.GIMIFixer(self._parser,
                                    modsToFix = ["rika"])                                                                       

    def create(self):
        self.createIniFile()
        self.createParser()
        self.createFixer()
        self._iniFile._iniParser = self._parser
        self._iniFile._iniFixer = self._fixer

    # ====================== edit =======================================

    def test_editDifferentInis_resGroupCollected(self):
        self.create()
        self._fixer.graphGroupEdits = [FRB.GraphGroupEdit([{("head", ""): [FRB.RegSurroundedAdd({"ps-t0", "ps-t1", "ps-t2"}, "drawindexed", ("addition", "yay"))]}])]
        
        tests = [
# ["""
# [TextureOverrideRaidenHead]
# ps-t0 = 1
# ps-t2 = 2
# """, ["""
# [TextureOverrideRaidenHead]
# ps-t0 = 1
# ps-t2 = 2


# ; --------------- Raiden Remap ---------------
# ; Raiden remapped by Albert Gold#2696 and NK#1321. If you used it to remap your Raiden mods pls give credit for "Albert Gold#2696" and "Nhok0169"
# ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

# [TextureOverrideRaidenHead]
# ps-t0 = 1
# ps-t2 = 2

# ; --------------------------------------------"""]],

# ["""
# [TextureOverrideRaidenHead]
# ps-t0 = 1
# ps-t2 = 2
# drawindexed = auto
# """, ["""
# [TextureOverrideRaidenHead]
# ps-t0 = 1
# ps-t2 = 2
# drawindexed = auto


# ; --------------- Raiden Remap ---------------
# ; Raiden remapped by Albert Gold#2696 and NK#1321. If you used it to remap your Raiden mods pls give credit for "Albert Gold#2696" and "Nhok0169"
# ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

# [TextureOverrideRaidenHead]
# ps-t0 = 1
# ps-t2 = 2
# addition = yay
# drawindexed = auto

# ; --------------------------------------------"""]],

# ["""
# [TextureOverrideRaidenHead]
# ps-t0 = 1
# ps-t2 = 2
# drawindexed = auto
# foo = bar
# drawindexed = 1,2,3
# ps-t0 = 3
# drawindexed = auto
# """, ["""
# [TextureOverrideRaidenHead]
# ps-t0 = 1
# ps-t2 = 2
# drawindexed = auto
# foo = bar
# drawindexed = 1,2,3
# ps-t0 = 3
# drawindexed = auto


# ; --------------- Raiden Remap ---------------
# ; Raiden remapped by Albert Gold#2696 and NK#1321. If you used it to remap your Raiden mods pls give credit for "Albert Gold#2696" and "Nhok0169"
# ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

# [TextureOverrideRaidenHead]
# ps-t0 = 1
# ps-t2 = 2
# addition = yay
# drawindexed = auto
# foo = bar
# drawindexed = 1,2,3
# ps-t0 = 3
# addition = yay
# drawindexed = auto

# ; --------------------------------------------"""]],

["""
[TextureOverrideRaidenHead]
ps-t0 = 1
ps-t2 = 2
if $x == 1
    drawindexed = auto
else if $x == 2
    ps-t1 = 3
    drawindexed = auto
else if $x == 3
    drawindexed = auto
    if $y == 1
        drawindexed = auto
    endif
    ps-t1 = 3
    drawindexed = auto
endif
""", ["""

"""]]
]
        
        for test in tests:
            print(f"============== TEST ====================")
            iniTxt = test[0]
            expectedIniTxt = test[1]

            self._iniFile.clear()
            self._iniFile._iniParser = self._parser
            self._iniFile._iniFixer = self._fixer

            self._iniFile.fileTxt = iniTxt

            self._iniFile.parse()
            resultFix = self._iniFile.fix()

            fixLen = len(resultFix)
            self.assertEqual(fixLen, len(expectedIniTxt))

            resultFix = list(resultFix.values())
            for i in range(fixLen):
                print(resultFix[i])
                currentResultFix = resultFix[i]
                currentExpectedFix = expectedIniTxt[i]

                self.assertEqual(currentResultFix, currentExpectedFix)