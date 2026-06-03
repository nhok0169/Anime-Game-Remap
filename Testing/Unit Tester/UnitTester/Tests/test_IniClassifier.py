import sys
from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class IniClassifierTest(BaseUnitTest):
    @classmethod
    def setUpClass(cls):
        super().setUpClass()

        cls._iniClassifierBuilder = FRB.IniClassifierBuilder()

        cls._classifier = FRB.IniClassifier(cls._iniClassifierBuilder)
        cls._slowClassifier = FRB.IniClassifier(cls._iniClassifierBuilder, ahoCorasickCls = FRB.AhoCorasickDFA)

    def setUp(self):
        super().setUp()
        self._classifier.reset()

    # ============ classify ==========================

    def test_diferentIniTxt_modTypeClassifiedOnly(self):
        tests = [
            ["", None],
            ["[TextureOverrideNingguang]", "Ningguang"],
            ["   [    TextureOverrideDilucFlamme   PositionRemap Fix     ]", "DilucFlamme"],
            ["  TextureOverrideKeqingOp    ", None],
            ["""
            ; keqing opulent
            TextureOverrideKeqingOpulentIb

            ; nilou
            ; [ TextureOverrideNilouPosition]
             
            # ayaka
            # [TextureOverrideAyakaBody]

            ; shenhe
            [TextureOverrideShenheHead]
            """, "Shenhe"],
        ]

        for test in tests:
            iniTxt = test[0]
            expected = test[1]

            result = self._classifier.classify(iniTxt, checkIsMod = False, checkIsFixed = False)

            if (expected is None):
                self.assertIsNone(result.modType)
                continue

            self.assertIsInstance(result.modType, FRB.ModType)
            self.assertEqual(result.modType.name, expected)

    def test_differentIniTxt_classificationStatsRetreived(self):
        PositionKey = FRB.IniKeywords.Position.value
        BlendKey = FRB.IniKeywords.Blend.value

        tests = [
            ["", None, False, False],
            ["[TextureOverrideNingguang]", "Ningguang", True, False],
            ["   [    TextureOverrideDilucFlamme   RemapPosition     ]", "DilucFlamme", True, True],
            ["  TextureOverrideKeqingOp    ", None, False, False],
            ["""
            ; keqing opulent
            TextureOverrideKeqingOpulentIb

            ; nilou
            ; [ TextureOverrideNilouPosition]
             
            # ayaka
            # [TextureOverrideAyakaBody]

            ; shenhe
            [TextureOverrideShenheHead]
            """, "Shenhe", True, False, False],
            ["[   TextureOveRridehutaoRemapBlend        ]", "HuTao", True, True],
            ["[    TextureOverrideBernkastel   nipah  Position    ]", None, True, False],
            ["[  MoeMoeKyun! RemapFixing is cool --> Nobody cares about the ending bracket ", None, False, False],
            ["""
             [ TextureOverrideNilouPosition]
             ...

             [TextureOverrideKeqingPositionAnother]
             ...

             [TextureOverrideFurinaBlend]
             ...

             [TextureOverrideAyakaRemapBlend]
             ...

             [TextureOverrideRaidenBlender]
             ...

             [TextureOverrideMavuikaPositionCool Sexy    RemapFix]
             """, "Nilou", True, True],
             ["[TextureOverrideMavuikaPositionCool Sexy    RemapFix]", None, True, True]
        ]

        for test in tests:
            iniTxt = test[0]
            expectedMod = test[1]
            expectedIsMod = test[2]
            exepectedIsFixed = test[3]

            result = self._classifier.classify(iniTxt, checkIsMod = True, checkIsFixed = True)

            if (expectedMod is None):
                self.assertIsNone(result.modType)
            else:
                self.assertIsInstance(result.modType, FRB.ModType)
                self.assertEqual(result.modType.name, expectedMod)

            self.assertEqual(result.isMod, expectedIsMod)
            self.assertEqual(result.isFixed, exepectedIsFixed)

    # ================================================

    def test_differentIniTxt_timeComparisonWithOldMultiRegex(self):
        import re
        from timeit import default_timer as timer

        search = """
; Merged Mod: .\Sayori_gamer_bunny_bikini_classic\Sayori Gamer Bunny Bikini Classic\Sayori.ini, .\Sayori_gamer_bunny_suit_classic\Sayori Gamer Bunny Suit Classic\Sayori.ini

; Constants ---------------------------

[Constants]
global persist $swapvar = 0
global $active
global $creditinfo = 0

[KeySwap]
condition = $active == 1
key = h
type = cycle
$swapvar = 0,1
$creditinfo = 0

[Present]
post $active = 0

; Shader ------------------------------

; Overrides ---------------------------

[TextureOverrideKeqiffffffffngPosition]
hash = 3aaf3e94
run = CommandListSayoriPosition
$active = 1

[TextureOverrideKeqfffingBlend]
hash = 0bf8e621
run = CommandListSayoriBlend

[TextureOverrideKegTexcoord]
hash = 723848fe
run = CommandListSayoriTexcoord

[TextureOverrideKeqinfffgVertexLimitRaise]
hash = ccc33b79

[TextureOverridingIB]
hash = cbf1894b
;hash = cbf1894b
run = CommandListSayoriIB

[TextureOverrideKeffffqingHead]
hash = cbf1894b
;hash = cbf1894b
match_first_index = 0
run = CommandListSayoriHead

[TextureOverrideeqingBody]
hash = cbf1894b
;hash = cbf1894b
match_first_index = 10824
run = CommandListSayoriBody

[TextureOverriddfdingDress]
hash = cbf1894b
;hash = cbf1894b
match_first_index = 48216
run = CommandListSayoriDress

[TextureOverrHeadDiffuse]
hash = d8c9c399
run = CommandListSayoriFaceHeadDiffuse

; CommandList -------------------------

[ComdddddddddmandLgPosition]
if $swapvar == 0
	vb0 = ResourceSayoriPosition.0
else if $swapvar == 1
	vb0 = ResourceSayoriPosition.1
endif

[CommandListgBlend]
if $swapvar == 0
	vb1 = ResourceSayoriBlend.0
	handling = skip
	draw = 47251,0
else if $swapvar == 1
	vb1 = ResourceSayoriBlend.1
	handling = skip
	draw = 34316,0
endif

[CommandListinord]
if $swapvar == 0
	vb1 = ResourceSayoriTexcoord.0
else if $swapvar == 1
	vb1 = ResourceSayoriTexcoord.1
endif

[CommandLdddddisingIB]
if $swapvar == 0
	handling = skip
	drawindexed = auto
else if $swapvar == 1
	handling = skip
	drawindexed = auto
endif

[Commanead]
if $swapvar == 0
	ib = ResourceSayoriHeadIB.0
	ps-t0 = ResourceSayoriHeadDiffuse.0
	ps-t1 = ResourceSayoriHeadLightMap.0
	ps-t2 = ResourceSayoriHeadMetalMap.0
	ps-t3 = ResourceSayoriHeadShadowRamp.0
else if $swapvar == 1
	ib = ResourceSayoriHeadIB.1
	ps-t0 = ResourceSayoriHeadDiffuse.1
	ps-t1 = ResourceSayoriHeadLightMap.1
	ps-t2 = ResourceSayoriHeadMetalMap.1
	ps-t3 = ResourceSayoriHeadShadowRamp.1
endif

[ComfdmandLingBody]
if $swapvar == 0
	ib = ResourceSayoriBodyIB.0
	ps-t0 = ResourceSayoriBodyDiffuse.0
	ps-t1 = ResourceSayoriBodyLightMap.0
	ps-t2 = ResourceSayoriBodyMetalMap.0
	ps-t3 = ResourceSayoriBodyShadowRamp.0
else if $swapvar == 1
	ib = ResourceSayoriBodyIB.1
	ps-t0 = ResourceSayoriBodyDiffuse.1
	ps-t1 = ResourceSayoriBodyLightMap.1
	ps-t2 = ResourceSayoriBodyMetalMap.1
	ps-t3 = ResourceSayoriBodyShadowRamp.1
endif

[CommandLisdftKedfdfdfdfdfdfqgDresdfs]
if $swapvar == 0
	ib = ResourceSayoriDressIB.0
	ps-t0 = ResourceSayoriBodyDiffuse.0
	ps-t1 = ResourceSayoriBodyLightMap.0
	ps-t2 = ResourceSayoriBodyMetalMap.0
	ps-t3 = ResourceSayoriBodyShadowRamp.0
else if $swapvar == 1
	ib = ResourceSayoriDressIB.1
	ps-t0 = ResourceSayoriBodyDiffuse.1
	ps-t1 = ResourceSayoriBodyLightMap.1
	ps-t2 = ResourceSayoriBodyMetalMap.1
	ps-t3 = ResourceSayoriBodyShadowRamp.1
endif

[ComdfdfdmandfdfddLisgFaceHeadfdfdfdfdDiffuse]
if $swapvar == 0
	ps-t0 = ResourceSayoriFaceHeadDiffuse.0
else if $swapvar == 1
	ps-t0 = ResourceSayoriFaceHeadDiffuse.1
endif

; Resources ---------------------------

[dfdfdfdfdfdfdfggggggggggg]
type = Buffer
stride = 40
filename = .\Sayori_gamer_bunny_bikini_classic\Sayori Gamer Bunny Bikini Classic\SayoriPosition.buf

[ResourceKeqdfdfdfingBlend.0]
type = Buffer
stride = 32
filename = .\Sayori_gamer_bunny_bikini_classic\Sayori Gamer Bunny Bikini Classic\SayoriBlend.buf

[ResourceKeqdfdfdfingTexcoord.0]
type = Buffer
stride = 20
filename = .\Sayori_gamer_bunny_bikini_classic\Sayori Gamer Bunny Bikini Classic\SayoriTexcoord.buf

[ResoudfdfdfrceKeqidfdfdfdfngHeadIB.0]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = .\Sayori_gamer_bunny_bikini_classic\Sayori Gamer Bunny Bikini Classic\SayoriHead.ib

[ResourceSayoriBodyIB.0]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = .\Sayori_gamer_bunny_bikini_classic\Sayori Gamer Bunny Bikini Classic\SayoriBody.ib

[ResourceKedfdfdfqingDressIB.0]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = .\Sayori_gamer_bunny_bikini_classic\Sayori Gamer Bunny Bikini Classic\SayoriDress.ib

[ResourcedfdfdKeqindfgHefdfdfdfadDiffuse.0]
filename = .\Sayori_gamer_bunny_bikini_classic\Sayori Gamer Bunny Bikini Classic\SayoriHeadDiffuse.dds

[ResourcefdfdKeqfdfdfingHdfdeadLightMap.0]
filename = .\Sayori_gamer_bunny_bikini_classic\Sayori Gamer Bunny Bikini Classic\SayoriHeadLightMap.dds

[ResourceKedfdfdfqingHeadMetalMap.0]
filename = .\Sayori_gamer_bunny_bikini_classic\Sayori Gamer Bunny Bikini Classic\SayoriHeadMetalMap.dds

[ResourceKeqdfdfingfdfHeadShadowRamp.0]
filename = .\Sayori_gamer_bunny_bikini_classic\Sayori Gamer Bunny Bikini Classic\SayoriHeadShadowRamp.jpg

[ResourceKeqindfdfdgBodyDiffuse.0]
filename = .\Sayori_gamer_bunny_bikini_classic\Sayori Gamer Bunny Bikini Classic\SayoriBodyDiffuse.dds

[ResourceSayoriBodyLdfdfdightMap.0]
filename = .\Sayori_gamer_bunny_bikini_classic\Sayori Gamer Bunny Bikini Classic\SayoriBodyLightMap.dds

[ResourceKedfdfqidfdfdfngBodyfMetalMap.0]
filename = .\keqidng_gamer_bunny_bikini_classic\Sayori Gamer Bunny Bikini Classic\SayoriBodyMetalMap.dds

[ResourceSayoriBodyShadowRamp.0]
filename = .\Sayori_gamer_bunny_bikini_classic\Sayori Gamer Bunny Bikini Classic\SayoriBodyShadowRamp.jpg

[ResourceSayoriDressdfdfdffDiffuse.0]
filename = .\Sayori_gamer_bunny_bikini_classic\Sayori Gamer Bunny Bikini Classic\SayoriDressDiffuse.dds

[ResourceSayoriDresdfsLightMap.0]
filenamde = .\Sayori_gamer_bunny_bikini_classic\Sayori Gamer Bunny Bikini Classic\SayoriDressLightMap.dds
fd
[ResourceSayoriDfdfressMetalMap.0]
filename = .\Sayori_gamer_bunny_bikini_classic\Sayori Gamer Bunny Bikini Classic\SayoriDressMetalMap.dds

[ResourcedfdfdSayoriDressdfShadowRamp.0]
filename = .\Sayori_gamer_bunny_bikini_classic\Sayori Gamer Bunny Bikini Classic\SayoriDressShadowRamp.jpg

[ResourcedfdfdeqindfdgFaceHeadDiffuse.0]
filename = .\Sayori_gamer_bunny_bikini_classic\Sayori Gamer Bunny Bikini Classic\SayoriFaceHeadDiffuse.dds

[TexturingBlend]
type = Buffer
stride = 40
filename = .\Sayori_gamer_bunny_suit_classic\Sayori Gamer Bunny Suit Classic\SayoriPosition.buf

[ResourceKdfdfdfdeqingBlend.1]
type = Buffer
stride = 32
filename = .\Sayori_gamer_bunny_suit_classic\Sayori Gamer Bunny Suit Classic\SayoriBlend.buf

[ResouingTexcoord.1]
type = Buffer
stride = 20
filename = .\Sayori_gamer_bunny_suit_classic\Sayori Gamer Bunny Suit Classic\SayoriTexcoord.buf

[ResourcdfdfdfeKeqdfdfdfdfingHeadIB.1]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = .\Sayori_gamer_bunny_suit_classic\Sayori Gamer Bunny Suit Classic\SayoriHead.ib

[ResourcedfdfKeqinsdsdfdfgBodyIB.1]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = .\Sayori_gamer_bunny_suit_classic\Sayori Gamer Bunny Suit Classic\SayoriBody.ib

[ResouerererrceKeererqingDreserersIB.1]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = .\Sayori_gamer_bunny_suit_classic\Sayori Gamer Bunny Suit Classic\SayoriDress.ib

[ResourceKeqierertytytyngHerereadDierffuse.1]
filename = .\Sayori_gamer_bunny_suit_classic\Sayori Gamer Bunny Suit Classic\SayoriHeadDiffuse.dds

[RedfsourcedfdKeqdfdfdfingHefgadLightMap.1]
filename = .\Sayori_gamer_bunny_suit_classic\Sayori Gamer Bunny Suit Classic\SayoriHeadLightMap.dds

[ResodfdfurcdfeKeqdfdfingdfdfHeadMetalMap.1]
filename = .\Sayori_gamer_bunny_suit_classic\Sayori Gamer Bunny Suit Classic\SayoriHeadMetalMap.dds

[RedfsoudvbvfgrdsceKsdedfdfdfgsqindfgHefgadShadowRamp.1]
filename = .\Sayori_gamer_bunny_suit_classic\Sayori Gamer Bunny Suit Classic\SayoriHeadShadowRamp.jpg

[ResourcedfdKdfdeqinfdfdfdfdfgBodyDiffuse.1]
filename = .\Sayori_gamer_bunny_suit_classic\Sayori Gamer Bunny Suit Classic\SayoriBodyDiffuse.dds

[ResodfursdfddsffceKeagqingBfsgfdodyLightMap.1]
filename = .\Sayori_gamer_bunny_suit_classic\Sayori Gamer Bunny Suit Classic\SayoriBodyLightMap.dds

[ResourcedfsdhKefgfgqidfngBoasddfsdyMdfgsdfetalMap.1]
filename = .\Sayori_gamer_bunny_suit_classic\Sayori Gamer Bunny Suit Classic\SayoriBodyMetalMap.dds

[ResngBodyShadowRamp.1]
filename = .\Sayori_gamer_bunny_suit_classic\Sayori Gamer Bunny Suit Classic\SayoriBodyShadowRamp.jpg

[ResoursdfcdfeKfdfdeqisdfsdngDredfdfdssDiffuse.1]
filename = .\Sayori_gamer_bunny_suit_classic\Sayori Gamer Bunny Suit Classic\SayoriDressDiffuse.dds

[ResourcsdfesdKefsdqifsdnfsdfsdgfsdfDressLightMap.1]
filename = .\Sayori_gamer_bunny_suit_classic\Sayori Gamer Bunny Suit Classic\SayoriDressLightMap.dds

[ResoursdfceKeqsdfdfifngDresdssfsdMetalMap.1]
filename = .\Sayori_gamer_bunny_suit_classic\Sayori Gamer Bunny Suit Classic\SayoriDressMetalMap.dds

[ResousdfrceKsdfeqisdngDfsdfressShadowRamp.1]
filename = .\Sayori_gamer_bunny_suit_classic\Sayori Gamer Bunny Suit Classic\SayoriDressShadowRamp.jpg

[ResfsdourfceKsdfsdeqinsdgFaceHeasdfdDiffuse.1]
filename = .\Sayori_gamer_bunny_suit_classic\Sayori Gamer Bunny Suit Classic\SayoriFaceHeadDiffuse.dds



; .ini generated by GIMI (Genshin-Impact-Model-Importer) mod merger script
; If you have any issues or find any bugs, please open a ticket at https://github.com/SilentNightSound/GI-Model-Importer/issues or contact SilentNightSound#7430 on discord

; 4.1 Character Fix 
[TextureOsdsdffsverriddfsdfe41FixVesdrtefsdxfLimitRaise0]
hash = ccc33b79
match_priority = 1


; --------------- Sayori Remap ---------------
; Sayori remapped by NK#1321 and Albert Gold#2696. If you used it to remap your Sayori mods pls give credit for "Nhok0169" and "Albert Gold#2696"
; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

; ***** SayoriOpulent *****
[TextureOverrideSayoriSayoriOpulentRemapBlend]
hash = 6f010b58
run = CommandListSayoriSayoriOpulentRemapBlend

[CommandLiqingKfgfgeqOpufgflentRemapBlend]
if $swapvar == 0
	vb1 = ResourceSayoriSayoriOpulentRemapBlend.0
	handling = skip
	draw = 47251,0
else if $swapvar == 1
	vb1 = ResourceSayoriSayoriOpulentRemapBlend.1
	handling = skip
	draw = 34316,0
endif


[TextureOvefggrrfideKeqfgifggfngPosifgtiKeqfginlentRemapFix]
hash = 0d7e3cc5
run = CommandListSayoriPositionSayoriOpulentRemapFix
$active = 1

[CommandsdfListKesdfdfdfingdfPofsdfsitsdsdfionKeqdfdingOpuslentRemapFix]
if $swapvar == 0
	vb0 = ResourceSayoriPosition.0
else if $swapvar == 1
	vb0 = ResourceSayoriPosition.1
endif

[TextureOverrsdfideKeqifsdngsdfsdTfsdfexcossdfdforsdfdKeqisngOpudflentRemapFix]
hash = 52f78cb7
run = CommandListSayoriTexcoordSayoriOpulentRemapFix

[CommandListSayoriTexcoordSayoriOpulentRemapFix]
if $swapvar == 0
	vb1 = ResourceSayoriTexcoord.0
else if $swapvar == 1
	vb1 = ResourceSayoriTexcoord.1
endif

[TextureOasdverriengVerasdtexLimitRaqindfdfgOpuasdlentRemapFix]
hash = 6629a84e

[TextureOverridedfKeIBdfdfdfdSayoriOdfdfdfpulentRemapFix]
hash = 7c6fc8c3
run = CommandListSayoriIBSayoriOpulentRemapFix

[CommandListSayoriIBSayoriOpulentRemapFix]
if $swapvar == 0
	handling = skip
	drawindexed = auto
else if $swapvar == 1
	handling = skip
	drawindexed = auto
endif

[TexdfdftureOveeSayoriBodsdySayoriOpulentRemadfdfdfpFix]
hash = 7c6fc8c3
match_first_index = 19623
run = CommandListSayoriBodySayoriOpulentRemapFix

[CommandListgBodyKeqindfdfgOfgfgfgpulentRemdfdfdapFix]
if $swapvar == 0
	ib = ResourceSayoriBodyIB.0
	ps-t0 = ResourceSayoriBodyDiffuse.0
	ps-t1 = ResourceSayoriBodyLightMap.0
	ps-t2 = ResourceSayoriBodyMetalMap.0
	ps-t3 = ResourceSayoriBodyShadowRamp.0
else if $swapvar == 1
	ib = ResourceSayoriBodyIB.1
	ps-t0 = ResourceSayoriBodyDiffuse.1
	ps-t1 = ResourceSayoriBodyLightMap.1
	ps-t2 = ResourceSayoriBodyMetalMap.1
	ps-t3 = ResourceSayoriBodyShadowRamp.1
endif

[TextureOverrideSayoriFaceHeadDiffuseSayoriOpulentRemapFix]
hash = c2b17f84
run = CommandListSayoriFaceHeadDiffuseSayoriOpulentRemapFix

[CommandListSayoriFaceHeadDiffuseSayoriOpulentRemapFix]
if $swapvar == 0
	ps-t0 = ResourceSayoriFaceHeadDiffuse.0
else if $swapvar == 1
	ps-t0 = ResourceSayoriFaceHeadDiffuse.1
endif

[TextureOverride41FixVertexLimitRaise0SayoriOpulentRemapFix]
hash = 6629a84e
match_priority = 1

[TextureOverrideSayoriHeadSayoriOpulentRemapFix]
hash = 7c6fc8c3
match_first_index = 0
run = CommandListSayoriHeadSayoriOpulentRemapFix

[CommandListSayoriHeadSayoriOpulentRemapFix]
if $swapvar == 0
	ib = ResourceSayoriDressIB.0
	ps-t0 = ResourceSayoriDressOpaqueDressDiffuseSayoriOpulentRemapTex0
	ps-t1 = ResourceSayoriBodyLightMap.0
	ps-t2 = ResourceSayoriBodyMetalMap.0
	ps-t3 = ResourceSayoriBodyShadowRamp.0
else if $swapvar == 1
	ib = ResourceSayoriDressIB.1
	ps-t0 = ResourceSayoriDressOpaqueDressDiffuseSayoriOpulentRemapTex1
	ps-t1 = ResourceSayoriBodyLightMap.1
	ps-t2 = ResourceSayoriBodyMetalMap.1
	ps-t3 = ResourceSayoriBodyShadowRamp.1
endif


[ResourceSayoriSayoriOpulentRemapBlend.0]
type = Buffer
stride = 32
filename = Sayori_gamer_bunny_bikini_classic\Sayori Gamer Bunny Bikini Classic\SayoriSayoriOpulentRemapBlend.buf

[ResourceSayoriSayoriOpulentRemapBlend.1]
type = Buffer
stride = 32
filename = Sayori_gamer_bunny_suit_classic\Sayori Gamer Bunny Suit Classic\SayoriSayoriOpulentRemapBlend.buf

[ResourceSayoriDressOpaqueDressDiffuseSayoriOpulentRemapTex0]
filename = Sayori_gamer_bunny_bikini_classic\Sayori Gamer Bunny Bikini Classic\SayoriBodyDiffuseSayoriOpulentRemapTex0.dds

[ResourceSayoriDressOpaqueDressDiffuseSayoriOpulentRemapTex1]
filename = Sayori_gamer_bunny_suit_classic\Sayori Gamer Bunny Suit Classic\SayoriBodyDiffuseSayoriOpulentRemapTex0.dds

; *************************

; --------------------------------------------
        """

        searchLines = FRB.TextTools.getTextLines(search)

        modRegexes = [
            re.compile(r"^\s*\[\s*textureoverride.*(amber)((?!cn).)*\]"),
            re.compile(r"^\s*\[\s*textureoverride.*(ambercn).*\]"),
            re.compile(r"^\s*\[\s*textureoverride.*(ayaka)((?!(springbloom)).)*\]"),
            re.compile(r"^\s*\[\s*textureoverride.*(ayakaspringbloom).*\]"),
            re.compile(r"^\s*\[\s*textureoverride.*(arlecchino).*\]"),
            re.compile(r"^\s*\[\s*textureoverride.*(barbara)((?!summertime).)*\]"),
            re.compile(r"^\s*\[\s*textureoverride.*(barbarasummertime).*\]"),
            re.compile(r"^\s*\[\s*textureoverride.*(cherryhutao).*\]"),
            re.compile(r"^\s*\[\s*textureoverride.*(hutaocherry).*\]"),
            re.compile(r"^\s*\[\s*textureoverride.*(diluc)((?!|flamme).)*\]"),
            re.compile(r"^\s*\[\s*textureoverride.*(dilucflamme).*\]"),
            re.compile(r"^\s*\[\s*textureoverride.*(fischl)((?!highness).)*\]"),
            re.compile(r"^\s*\[\s*textureoverride.*(fischlhighness).*\]"),
            re.compile(r"^\s*\[\s*textureoverride.*(ganyu)((?!(twilight)).)*\]"),
            re.compile(r"^\s*\[\s*textureoverride.*(ganyutwilight).*\]"),
            re.compile(r"^\s*\[\s*textureoverride((?!cherry).)*(hutao)((?!cherry).)*\]"),
            re.compile(r"^\s*\[\s*textureoverride.*(jean)((?!(cn|sea)).)*\]"),
            re.compile(r"^\s*\[\s*textureoverride.*(jeancn)((?!sea).)*\]"),
            re.compile(r"^\s*\[\s*textureoverride.*(jeansea)((?!cn).)*\]"),
            re.compile(r"^\s*\[\s*textureoverride.*(keqing)((?!(opulent)).)*\]"),
            re.compile(r"^\s*\[\s*textureoverride.*(keqingopulent).*\]"),
            re.compile(r"^\s*\[\s*textureoverride.*(kirara)((?!boots).)*\]"),
            re.compile(r"^\s*\[\s*textureoverride.*(kiraraboots).*\]"),
            re.compile(r"^\s*\[\s*textureoverride.*(klee)((?!blossomingstarlight).)*\]"),
            re.compile(r"^\s*\[\s*textureoverride.*(kleeblossomingstarlight).*\]"),
            re.compile(r"^\s*\[\s*textureoverride.*(mona)((?!(cn)).)*\]"),
            re.compile(r"^\s*\[\s*textureoverride.*(monacn).*\]"),
            re.compile(r"^\s*\[\s*textureoverride.*(nilou)((?!(breeze)).)*\]"),
            re.compile(r"^\s*\[\s*textureoverride.*(niloubreeze).*\]"),
            re.compile(r"^\s*\[\s*textureoverride.*(ningguang)((?!(orchid)).)*\]"),
            re.compile(r"^\s*\[\s*textureoverride.*(ningguangorchid).*\]"),
            re.compile(r"^\s\*\[\s\*textureoverride.\*(raiden).*\]"),
            re.compile(r"^\s\*\[\s\*textureoverride.\*(shogun).*\]"),
            re.compile(r"^\s*\[\s*textureoverride.*(rosaria)((?!(cn)).)*\]"),
            re.compile(r"^\s*\[\s*textureoverride.*(rosariacn).*\]"),
            re.compile(r"^\s*\[\s*textureoverride.*(shenhe)((?!frostflower).)*\]"),
            re.compile(r"^\s*\[\s*textureoverride.*(shenhefrostflower).*\]"),
            re.compile(r"^\s*\[\s*textureoverride.*(xingqiu)((?!bamboo).)*\]"),
            re.compile(r"^\s*\[\s*textureoverride.*(xingqiubamboo).*\]")
        ]

        isFixedPattern = re.compile(r"^\s*\[\s*textureoverride.*remap(blend|position|texcoord|fix|tex).*\]")
        isModPattern = re.compile(r"^\s*\[\s*textureoverride.*(blend|position|texcoord).*\]")
        
        def tempRegex(line):
            for reg in modRegexes:
                if (re.search(reg, line)):
                    return True
                
            return False
        
        start = timer()
        modFound = False
        isMod = False
        isFixed = False
        
        regResult = None
        regIsMod = False
        regIsFixed = False

        for line in searchLines:
            cleanedLine = line.replace(FRB.IniKeywords.HideOriginalComment.value, "").lower()
            if (not regResult):
                regResult = tempRegex(cleanedLine)
                if (not modFound and regResult):
                    modFound = True

            if (not regIsMod):
                regIsMod = re.search(isModPattern, cleanedLine)
                if (not isMod and regIsMod):
                    isMod = True
                    regIsMod = True

            if (not regIsFixed):
                regIsFixed = re.search(isFixedPattern, cleanedLine)
                if (not isFixed and regIsFixed):
                    isFixed = True
                    regIsFixed = True

            if (modFound and isMod and isFixed):
                break
        end = timer()
        regTime = end - start

        start = timer()
        fastDFAResult = self._classifier.classify(search)
        end = timer()
        fastDFATime = end - start

        start = timer()
        dfaResult = self._slowClassifier.classify(search)
        end = timer()
        dfaTime = end - start

        # print(f"REGEX RESULT: {bool(regResult)} AND {bool(regIsMod)} AND {bool(regIsFixed)}")
        # print(f"FAST DFA RESULT: {bool(fastDFAResult.modType is not None)} AND {fastDFAResult.isMod} AND {fastDFAResult.isFixed}")
        # print(f"DFA RESULT: {bool(dfaResult.modType is not None)} AND {dfaResult.isMod} AND {dfaResult.isFixed}\n")
        

        # print(f"REGEX TIME: {regTime}")
        # print(f"FAST DFA TIME: {fastDFATime}")
        # print(f"DFA TIME: {dfaTime}\n")

        # print(f"DFA IS WINNER: FAST --> {fastDFATime <= regTime} AND REGULAR --> {dfaTime <= regTime}")
        # print(f"FAST DFA EFFICIENCY: {regTime / fastDFATime}")
        # print(f"DFA EFFICIENCY: {regTime / dfaTime}\n")