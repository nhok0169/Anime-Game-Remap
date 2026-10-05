##### Credits

# ===== Anime Game Remap (AG Remap) =====
# Authors: Albert Gold#2696, NK#1321
#
# if you used it to remap your mods pls give credit for "Albert Gold#2696" and "Nhok0169"
# Special Thanks:
#   nguen#2011 (for support)
#   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
#   HazrateGolabi#1364 (for being awesome, and improving the code)

##### EndCredits

##### ExtImports
import sys
##### EndExtImports

##### LocalImports
from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB
##### EndLocalImports


# A mod's sections as IniFile parses them: GIMI's newer texture API brought into the traditional one
# (GIMIApiNormalizer, run as the sections are read), and 3DMigoto's conditionals in any case.
class GIMIApiNormalizerTest(BaseUnitTest):
    def _sections(self, txt: str):
        return FRB.IniFile(txt = txt).getIfTemplates()

    def _shape(self, section):
        result = []
        for part in section.parts:
            if (isinstance(part, FRB.IfContentPart)):
                if (part.entries()):
                    result.append(part.entries())
            else:
                result.append(part.src.strip())
        return result

    # ================================================
    # ============== the texture API ==================

    def test_setTextures_keysBecomeRegisters_refDropped_callBecomesORFix(self):
        sections = self._sections("[TextureOverrideA]\n"
                                  "hash = abc\n"
                                  "Resource\\GIMI\\NormalMap = ref ResourceN\n"
                                  "Resource\\GIMI\\Diffuse = ref ResourceD\n"
                                  "Resource\\GIMI\\LightMap = ref ResourceL\n"
                                  "run = CommandList\\GIMI\\SetTextures\n"
                                  "drawindexed = auto\n")

        self.compareList(self._shape(sections["TextureOverrideA"]), [[
            ("hash", "abc"), ("ps-t0", "ResourceN"), ("ps-t1", "ResourceD"), ("ps-t2", "ResourceL"),
            ("run", "CommandList\\global\\ORFix\\ORFix"), ("drawindexed", "auto")]])

    def test_setTextures_ignoresCase(self):
        sections = self._sections("[TextureOverrideA]\n"
                                  "resource\\gimi\\normalmap = ref ResourceN\n"
                                  "resource\\gimi\\diffuse = REF ResourceD\n"
                                  "Resource\\GIMI\\Lightmap = ResourceL\n"
                                  "run = commandlist\\gimi\\settextures\n")

        self.compareList(self._shape(sections["TextureOverrideA"]), [[
            ("ps-t0", "ResourceN"), ("ps-t1", "ResourceD"), ("ps-t2", "ResourceL"), ("run", "CommandList\\global\\ORFix\\ORFix")]])

    def test_setTextures_noNormalMap_plainLayoutUnderNNFix(self):
        # SetTextures leaves the normal map null: NNFix's reading, diffuse / light map at ps-t0 / ps-t1
        sections = self._sections("[TextureOverrideA]\n"
                                  "Resource\\GIMI\\Diffuse = ref ResourceD\n"
                                  "Resource\\GIMI\\LightMap = ref ResourceL\n"
                                  "run = CommandList\\GIMI\\SetTextures\n")

        self.compareList(self._shape(sections["TextureOverrideA"]), [[
            ("ps-t0", "ResourceD"), ("ps-t1", "ResourceL"), ("run", "CommandList\\global\\ORFix\\NNFix")]])

    def test_setTextures_insideToggles_everyPartOfTheSection(self):
        # the normal map is set only inside the toggle, and still decides the layout for the whole section
        sections = self._sections("[TextureOverrideA]\n"
                                  "Resource\\GIMI\\Diffuse = ref ResourceD\n"
                                  "run = CommandList\\GIMI\\SetTextures\n"
                                  "if $x == 0\n"
                                  "Resource\\GIMI\\NormalMap = ref ResourceN2\n"
                                  "Resource\\GIMI\\Diffuse = ref ResourceD2\n"
                                  "run = CommandList\\GIMI\\SetTextures\n"
                                  "drawindexed = 3, 0, 0\n"
                                  "endif\n")

        self.compareList(self._shape(sections["TextureOverrideA"]), [
            [("ps-t1", "ResourceD"), ("run", "CommandList\\global\\ORFix\\ORFix")],
            "if $x == 0",
            [("ps-t0", "ResourceN2"), ("ps-t1", "ResourceD2"), ("run", "CommandList\\global\\ORFix\\ORFix"), ("drawindexed", "3, 0, 0")],
            "endif"])

    def test_noSetTextures_leftAlone(self):
        # set but never read: converting these would bind files the mod never bound
        sections = self._sections("[TextureOverrideA]\n"
                                  "Resource\\GIMI\\NormalMap = ResourceL\n"
                                  "Resource\\GIMI\\NormalMap = ResourceN\n"
                                  "drawindexed = auto\n")

        self.compareList(self._shape(sections["TextureOverrideA"]), [[
            ("Resource\\GIMI\\NormalMap", "ResourceL"), ("Resource\\GIMI\\NormalMap", "ResourceN"), ("drawindexed", "auto")]])

    def test_otherSectionsUntouched(self):
        sections = self._sections("[TextureOverrideA]\n"
                                  "Resource\\GIMI\\Diffuse = ref ResourceD\n"
                                  "run = CommandList\\GIMI\\SetTextures\n"
                                  "[TextureOverrideB]\n"
                                  "ps-t0 = ResourceX\n"
                                  "run = CommandList\\global\\ORFix\\ORFix\n")

        self.compareList(self._shape(sections["TextureOverrideB"]), [[
            ("ps-t0", "ResourceX"), ("run", "CommandList\\global\\ORFix\\ORFix")]])

    def test_natlanKeys_leftAsTheyAre(self):
        sections = self._sections("[TextureOverrideA]\n"
                                  "Resource\\GIMI\\NormalMap = ref ResourceN\n"
                                  "Resource\\GIMI\\Diffuse = ref ResourceD\n"
                                  "Resource\\GIMI\\NatlanGlow = ref ResourceG\n"
                                  "run = CommandList\\GIMI\\SetTextures\n")

        self.compareList(self._shape(sections["TextureOverrideA"]), [[
            ("ps-t0", "ResourceN"), ("ps-t1", "ResourceD"), ("Resource\\GIMI\\NatlanGlow", "ref ResourceG"),
            ("run", "CommandList\\global\\ORFix\\ORFix")]])

    # ================================================
    # ============ conditionals, any case =============

    def test_conditionals_anyCase(self):
        sections = self._sections("[TextureOverrideA]\n"
                                  "IF $x == 0\n"
                                  "drawindexed = 1, 0, 0\n"
                                  "ELSE if $x == 1\n"
                                  "drawindexed = 2, 1, 0\n"
                                  "Else\n"
                                  "drawindexed = 3, 3, 0\n"
                                  "ENDIF\n")

        parts = sections["TextureOverrideA"].parts
        types = [part.type for part in parts if not isinstance(part, FRB.IfContentPart)]
        self.compareList(types, [FRB.IfPredPartType.If, FRB.IfPredPartType.Elif, FRB.IfPredPartType.Else, FRB.IfPredPartType.EndIf])
        draws = [part.entries() for part in parts if isinstance(part, FRB.IfContentPart) and part.entries()]
        self.compareList(draws, [[("drawindexed", "1, 0, 0")], [("drawindexed", "2, 1, 0")], [("drawindexed", "3, 3, 0")]])
