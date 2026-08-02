import sys
from ordered_set import OrderedSet

from .baseIniFileTest import BaseIniFileTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class ResRegCollectTest(BaseIniFileTest):
    @classmethod
    def setUpClass(cls):
        super().setUpClass()

        cls._parser = None
        cls._fixer = None

    def createParser(self):
        self._parser = FRB.GIMIParser(self._iniFile, modObjs = OrderedSet([("", "blend"), ("head", ""), ("body", "")]), trackKeys = False)
        
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

    def test_differentInis_blendResourcesCollected(self):
        self.create()
        self._fixer.graphGroupEdits = [FRB.ResRegCollect({(0, "", "blend"): "vb1"}, {"blendType": FRB.RemapBlendReplace((0, "", "remapBlend"))})]

        tests = [
                    [self._defaultIniTxt, 4, [
                    """
                    [Constants]
                    global persist $swapvar = 0
                    global persist $swapvarn = 0
                    global persist $swapmain = 0
                    global persist $swapoffice = 0
                    global persist $swapglasses = 0

                    [KeyVar]
                    condition = $active == 1
                    key = VK_DOWN
                    type = cycle
                    $swapvar = 0,1,2

                    [KeyIntoTheHole]
                    condition = $active == 1
                    key = VK_RIGHT
                    type = cycle
                    $swapvarn = 0,1

                    ; The top part is not really important, so I not going to finish
                    ;   typing all the key swaps... 😋
                    ;
                    ; The bottom part is what the fix actually cares about

                    [TextureOverrideRaidenShogunBlend]
                    run = CommandListRaidenShogunBlend
                    handling = skip
                    draw = 21916,0

                    [CommandListRaidenShogunBlend]
                    if $swapmain == 0
                        if $swapvar == 0 && $swapvarn == 0
                            vb1 = ResourceRaidenShogunBlend.0
                        else
                            vb1 = ResourceEiBlendsHerBlenderInsteadOfHerSmoothie
                        endif
                    else if $swapmain == 1
                        run = SubSubTextureOverride
                    endif

                    [SubSubTextureOverride]
                    if $swapoffice == 0 && $swapglasses == 0
                        vb1 = GIMINeedsResourcesToAllStartWithResource
                    endif

                    [ResourceRaidenShogunBlend.0]
                    type = Buffer
                    stride = 32
                    filename = ..\..\..\../../../../../../2-BunnyRaidenShogun\RaidenShogunBlend.buf

                    [ResourceEiBlendsHerBlenderInsteadOfHerSmoothie]
                    type = Buffer
                    stride = 32
                    if $swapmain == 1
                        filename = M:\AnotherDrive\CuteLittleEi.buf
                    else
                        run = RaidenPuppetCommandResource
                    endif

                    [GIMINeedsResourcesToAllStartWithResource]
                    type = Buffer
                    stride = 32
                    filename = ./../AAA/BBBB\CCCCCC\DDDDDRemapBlend.buf

                    [TextureOverrideRaidenShogunRemapBlend]
                    run = CommandListRaidenShogunRemapBlend
                    handling = skip
                    draw = 21916,0
                    [RaidenPuppetCommandResource]
                    type = Buffer
                    stride = 32
                    filename = ./Dont/Use\If/Statements\Or/SubCommands\In/Resource\Sections.buf

                    ; ------ some lines originally generated from the fix ---------

                    [ResourceEiBlendsHerRemapBlenderInsteadOfHerSmoothie]
                    ; she drank the smoothie

                    type = Buffer
                    stride = 32

                    if $swapmain == 1
                        filename = M:\AnotherDrive\CuteLittleEiRemapBlend.buf

                    else
                        run = RaidenPuppetCommandResourceRemapBlend
                    endif

                    [ResourceRaidenShogunRemapBlend.0]
                    type = Buffer
                    stride = 32

                    ; where does this go?
                    filename = ..\..\..\..\..\..\..\..\..\\2-BunnyRaidenShogun\RaidenShogunRemapBlend.buf

                    [RaidenPuppetCommandResourceRemapBlend]
                    type = Buffer
                    stride = 32

                    # for some reason, GIMI does not work as what you expect for this case
                    filename = Dont\\Use\If\Statements\Or\SubCommands\In\Resource\SectionsRemapBlend.buf

                    ; --------------------------------------------------------------


                    ; --------------- Raiden Boss Fix -----------------
                    ; Raiden boss fixed by NK#1321 if you used it for fix your raiden pls give credit for "Nhok0169"
                    ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 and Albert Gold#2696 for support

                    [TextureOverrideRaidenShogunRemapBlend]
                    run = CommandListRaidenShogunRemapBlend
                    handling = skip
                    draw = 21916,0

                    [CommandListRaidenShogunRemapBlend]

                    # main swap
                    if $swapmain == 0

                        # some other subvariable swap
                        if $swapvar == 0 && $swapvarn == 0
                            vb1 = ResourceRaidenShogunRemapBlend.0

                        ; ruin the smoothie
                        else
                            vb1 = ResourceEiBlendsHerRemapBlenderInsteadOfHerSmoothie
                        endif

                    ; some boring swap
                    else if $swapmain == 1
                        run = SubSubTextureOverrideRemapBlend
                    endif

                    [SubSubTextureOverrideRemapBlend]
                    if $swapoffice == 0 && $swapglasses == 0
                        vb1 = ResourceGIMINeedsResourcesToAllStartWithResourceRemapBlend
                    endif


                    [GIMINeedsResourcesToAllStartWithResourceRemapBlend]
                    type = Buffer
                    stride = 32
                    filename = ..\AAA\BBBB\CCCCCC\DDDDDRemapRemapBlend.buf

                    [ResourceEiBlendsHerRemapBlenderInsteadOfHerSmoothie]
                    type = Buffer
                    stride = 32
                    if $swapmain == 1
                        filename = M:\AnotherDrive\CuteLittleEiRemapBlend.buf
                    else
                        run = RaidenPuppetCommandResourceRemapBlend
                    endif

                    [ResourceRaidenShogunRemapBlend.0]
                    type = Binaries
                    stride = 31
                    filename = ..\..\..\..\..\..\..\..\..\\2-BunnyRaidenShogun\RaidenShogunRemapBlend.buf

                    [RaidenPuppetCommandResourceRemapBlend]
                    type = Buffer
                    stride = 32
                    filename = Dont\\Use\If\Statements\Or\SubCommands\In\Resource\SectionsRemapBlend.buf


                    ; -------------------------------------------------

; --------------- Raiden Remap ---------------
; Raiden remapped by Albert Gold#2696 and NK#1321. If you used it to remap your Raiden mods pls give credit for "Albert Gold#2696" and "Nhok0169"
; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

[TextureOverrideRaidenShogunBlend]
run = CommandListRaidenShogunBlend
handling = skip
draw = 21916,0

[CommandListRaidenShogunBlend]
if $swapmain == 0
\tif $swapvar == 0 && $swapvarn == 0
\t\tvb1 = ResourceRaidenShogunRikaRemapBlend.0
\telse
\t\tvb1 = ResourceEiBlendsHerRikaRemapBlenderInsteadOfHerSmoothie
\tendif
else if $swapmain == 1
\trun = SubSubTextureOverride
endif

[SubSubTextureOverride]
if $swapoffice == 0 && $swapglasses == 0
\tvb1 = ResourceGIMINeedsResourcesToAllStartWithResourceRikaRemapBlend
endif

[ResourceRaidenShogunRikaRemapBlend.0]
type = Buffer
stride = 32
filename = ..\..\..\..\..\..\..\..\..\\2-BunnyRaidenShogun/RaidenShogunRikaRemapBlend.buf

[ResourceEiBlendsHerRikaRemapBlenderInsteadOfHerSmoothie]
type = Buffer
stride = 32
if $swapmain == 1
\tfilename = M:\AnotherDrive/CuteLittleEiRikaRemapBlend.buf
else
\trun = ResourceRaidenPuppetCommandResourceRikaRemapBlend
endif

[ResourceRaidenPuppetCommandResourceRikaRemapBlend]
type = Buffer
stride = 32
filename = Dont\\Use\If\Statements\Or\SubCommands\In\Resource/SectionsRikaRemapBlend.buf

[ResourceGIMINeedsResourcesToAllStartWithResourceRikaRemapBlend]
type = Buffer
stride = 32
filename = ..\AAA\BBBB\CCCCCC/DDDDDRemapRikaRemapBlend.buf

; --------------------------------------------"""
                    ]],
                    ["""
                     [boo]
                     tao = 1""", 0, [
                     """
                     [boo]
                     tao = 1

; --------------- GI Remap ---------------
; Mod remapped by Albert Gold#2696 and NK#1321. If you used it to remap your mods pls give credit for "Albert Gold#2696" and "Nhok0169"
; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support



; ----------------------------------------"""
                     ]]
                 ]

        for test in tests:
            iniTxt = test[0]
            expectedResourceCount = test[1]
            expectedIniTxt = test[2]

            self._iniFile.clear()
            self._iniFile._iniParser = self._parser
            self._iniFile._iniFixer = self._fixer

            self._iniFile.fileTxt = iniTxt

            self._iniFile.parse()
            resultFix = self._iniFile.fix()

            resultResources = self._iniFile.resources

            self.assertEqual(len(resultResources), expectedResourceCount)

            fixLen = len(resultFix)
            self.assertEqual(fixLen, len(expectedIniTxt))

            resultFix = list(resultFix.values())
            for i in range(fixLen):
                currentResultFix = resultFix[i]
                currentExpectedFix = expectedIniTxt[i]

                self.assertEqual(currentResultFix, currentExpectedFix)

    def test_differentInis_blendSubTypeResourcesCollected(self):
        self.create()
        self._fixer.graphGroupEdits = [FRB.ResRegCollect({(0, "", "blend"): "vb1"}, {"blendType1": FRB.RemapBlendReplace((0, "Life", "remapBlend"), resSubType = "JenovaLife"),
                                                                                     "blendType2": FRB.RemapBlendReplace((0, "Death", "remapBlend"), resSubType = "JenovaDeath"),
                                                                                     "blendType3": FRB.ResIdentity((0, "Birth", "remapBlend")),
                                                                                     "blendType4": FRB.ResIdentity((0, "Synthesis", "remapBlend"), createResModel = False)},
                                                         remaps = {(0, "", "blend"): {"blendType1": (0, "Life", "blend", lambda name: name + "Life"),
                                                                                      "blendType2": (0, "Death", "blend", lambda name: name + "Death"),
                                                                                      "blendType3": (0, "", "blend")}})]

        tests = [
                    [self._defaultIniTxt, 12, [
                    """
                    [Constants]
                    global persist $swapvar = 0
                    global persist $swapvarn = 0
                    global persist $swapmain = 0
                    global persist $swapoffice = 0
                    global persist $swapglasses = 0

                    [KeyVar]
                    condition = $active == 1
                    key = VK_DOWN
                    type = cycle
                    $swapvar = 0,1,2

                    [KeyIntoTheHole]
                    condition = $active == 1
                    key = VK_RIGHT
                    type = cycle
                    $swapvarn = 0,1

                    ; The top part is not really important, so I not going to finish
                    ;   typing all the key swaps... 😋
                    ;
                    ; The bottom part is what the fix actually cares about

                    [TextureOverrideRaidenShogunBlend]
                    run = CommandListRaidenShogunBlend
                    handling = skip
                    draw = 21916,0

                    [CommandListRaidenShogunBlend]
                    if $swapmain == 0
                        if $swapvar == 0 && $swapvarn == 0
                            vb1 = ResourceRaidenShogunBlend.0
                        else
                            vb1 = ResourceEiBlendsHerBlenderInsteadOfHerSmoothie
                        endif
                    else if $swapmain == 1
                        run = SubSubTextureOverride
                    endif

                    [SubSubTextureOverride]
                    if $swapoffice == 0 && $swapglasses == 0
                        vb1 = GIMINeedsResourcesToAllStartWithResource
                    endif

                    [ResourceRaidenShogunBlend.0]
                    type = Buffer
                    stride = 32
                    filename = ..\..\..\../../../../../../2-BunnyRaidenShogun\RaidenShogunBlend.buf

                    [ResourceEiBlendsHerBlenderInsteadOfHerSmoothie]
                    type = Buffer
                    stride = 32
                    if $swapmain == 1
                        filename = M:\AnotherDrive\CuteLittleEi.buf
                    else
                        run = RaidenPuppetCommandResource
                    endif

                    [GIMINeedsResourcesToAllStartWithResource]
                    type = Buffer
                    stride = 32
                    filename = ./../AAA/BBBB\CCCCCC\DDDDDRemapBlend.buf

                    [TextureOverrideRaidenShogunRemapBlend]
                    run = CommandListRaidenShogunRemapBlend
                    handling = skip
                    draw = 21916,0
                    [RaidenPuppetCommandResource]
                    type = Buffer
                    stride = 32
                    filename = ./Dont/Use\If/Statements\Or/SubCommands\In/Resource\Sections.buf

                    ; ------ some lines originally generated from the fix ---------

                    [ResourceEiBlendsHerRemapBlenderInsteadOfHerSmoothie]
                    ; she drank the smoothie

                    type = Buffer
                    stride = 32

                    if $swapmain == 1
                        filename = M:\AnotherDrive\CuteLittleEiRemapBlend.buf

                    else
                        run = RaidenPuppetCommandResourceRemapBlend
                    endif

                    [ResourceRaidenShogunRemapBlend.0]
                    type = Buffer
                    stride = 32

                    ; where does this go?
                    filename = ..\..\..\..\..\..\..\..\..\\2-BunnyRaidenShogun\RaidenShogunRemapBlend.buf

                    [RaidenPuppetCommandResourceRemapBlend]
                    type = Buffer
                    stride = 32

                    # for some reason, GIMI does not work as what you expect for this case
                    filename = Dont\\Use\If\Statements\Or\SubCommands\In\Resource\SectionsRemapBlend.buf

                    ; --------------------------------------------------------------


                    ; --------------- Raiden Boss Fix -----------------
                    ; Raiden boss fixed by NK#1321 if you used it for fix your raiden pls give credit for "Nhok0169"
                    ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 and Albert Gold#2696 for support

                    [TextureOverrideRaidenShogunRemapBlend]
                    run = CommandListRaidenShogunRemapBlend
                    handling = skip
                    draw = 21916,0

                    [CommandListRaidenShogunRemapBlend]

                    # main swap
                    if $swapmain == 0

                        # some other subvariable swap
                        if $swapvar == 0 && $swapvarn == 0
                            vb1 = ResourceRaidenShogunRemapBlend.0

                        ; ruin the smoothie
                        else
                            vb1 = ResourceEiBlendsHerRemapBlenderInsteadOfHerSmoothie
                        endif

                    ; some boring swap
                    else if $swapmain == 1
                        run = SubSubTextureOverrideRemapBlend
                    endif

                    [SubSubTextureOverrideRemapBlend]
                    if $swapoffice == 0 && $swapglasses == 0
                        vb1 = ResourceGIMINeedsResourcesToAllStartWithResourceRemapBlend
                    endif


                    [GIMINeedsResourcesToAllStartWithResourceRemapBlend]
                    type = Buffer
                    stride = 32
                    filename = ..\AAA\BBBB\CCCCCC\DDDDDRemapRemapBlend.buf

                    [ResourceEiBlendsHerRemapBlenderInsteadOfHerSmoothie]
                    type = Buffer
                    stride = 32
                    if $swapmain == 1
                        filename = M:\AnotherDrive\CuteLittleEiRemapBlend.buf
                    else
                        run = RaidenPuppetCommandResourceRemapBlend
                    endif

                    [ResourceRaidenShogunRemapBlend.0]
                    type = Binaries
                    stride = 31
                    filename = ..\..\..\..\..\..\..\..\..\\2-BunnyRaidenShogun\RaidenShogunRemapBlend.buf

                    [RaidenPuppetCommandResourceRemapBlend]
                    type = Buffer
                    stride = 32
                    filename = Dont\\Use\If\Statements\Or\SubCommands\In\Resource\SectionsRemapBlend.buf


                    ; -------------------------------------------------

; --------------- Raiden Remap ---------------
; Raiden remapped by Albert Gold#2696 and NK#1321. If you used it to remap your Raiden mods pls give credit for "Albert Gold#2696" and "Nhok0169"
; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

[TextureOverrideRaidenShogunBlendLife]
run = CommandListRaidenShogunBlendLife
handling = skip
draw = 21916,0

[CommandListRaidenShogunBlendLife]
if $swapmain == 0
\tif $swapvar == 0 && $swapvarn == 0
\t\tvb1 = ResourceRaidenShogunRikaJenovaLifeRemapBlend.0
\telse
\t\tvb1 = ResourceEiBlendsHerRikaJenovaLifeRemapBlenderInsteadOfHerSmoothie
\tendif
else if $swapmain == 1
\trun = SubSubTextureOverrideLife
endif

[SubSubTextureOverrideLife]
if $swapoffice == 0 && $swapglasses == 0
\tvb1 = ResourceGIMINeedsResourcesToAllStartWithResourceRikaJenovaLifeRemapBlend
endif

[TextureOverrideRaidenShogunBlendDeath]
run = CommandListRaidenShogunBlendDeath
handling = skip
draw = 21916,0

[CommandListRaidenShogunBlendDeath]
if $swapmain == 0
\tif $swapvar == 0 && $swapvarn == 0
\t\tvb1 = ResourceRaidenShogunRikaJenovaDeathRemapBlend.0
\telse
\t\tvb1 = ResourceEiBlendsHerRikaJenovaDeathRemapBlenderInsteadOfHerSmoothie
\tendif
else if $swapmain == 1
\trun = SubSubTextureOverrideDeath
endif

[SubSubTextureOverrideDeath]
if $swapoffice == 0 && $swapglasses == 0
\tvb1 = ResourceGIMINeedsResourcesToAllStartWithResourceRikaJenovaDeathRemapBlend
endif

[TextureOverrideRaidenShogunBlend]
run = CommandListRaidenShogunBlend
handling = skip
draw = 21916,0

[CommandListRaidenShogunBlend]
if $swapmain == 0
\tif $swapvar == 0 && $swapvarn == 0
\t\tvb1 = ResourceRaidenShogunBlend.0
\telse
\t\tvb1 = ResourceEiBlendsHerBlenderInsteadOfHerSmoothie
\tendif
else if $swapmain == 1
\trun = SubSubTextureOverride
endif

[SubSubTextureOverride]
if $swapoffice == 0 && $swapglasses == 0
\tvb1 = GIMINeedsResourcesToAllStartWithResource
endif

[ResourceRaidenShogunRikaJenovaLifeRemapBlend.0]
type = Buffer
stride = 32
filename = ..\..\..\..\..\..\..\..\..\\2-BunnyRaidenShogun/RaidenShogunRikaJenovaLifeRemapBlend.buf

[ResourceEiBlendsHerRikaJenovaLifeRemapBlenderInsteadOfHerSmoothie]
type = Buffer
stride = 32
if $swapmain == 1
\tfilename = M:\AnotherDrive/CuteLittleEiRikaJenovaLifeRemapBlend.buf
else
\trun = ResourceRaidenPuppetCommandResourceRikaJenovaLifeRemapBlend
endif

[ResourceRaidenPuppetCommandResourceRikaJenovaLifeRemapBlend]
type = Buffer
stride = 32
filename = Dont\\Use\If\Statements\Or\SubCommands\In\Resource/SectionsRikaJenovaLifeRemapBlend.buf

[ResourceGIMINeedsResourcesToAllStartWithResourceRikaJenovaLifeRemapBlend]
type = Buffer
stride = 32
filename = ..\AAA\BBBB\CCCCCC/DDDDDRemapRikaJenovaLifeRemapBlend.buf

[ResourceRaidenShogunRikaJenovaDeathRemapBlend.0]
type = Buffer
stride = 32
filename = ..\..\..\..\..\..\..\..\..\\2-BunnyRaidenShogun/RaidenShogunRikaJenovaDeathRemapBlend.buf

[ResourceEiBlendsHerRikaJenovaDeathRemapBlenderInsteadOfHerSmoothie]
type = Buffer
stride = 32
if $swapmain == 1
\tfilename = M:\AnotherDrive/CuteLittleEiRikaJenovaDeathRemapBlend.buf
else
\trun = ResourceRaidenPuppetCommandResourceRikaJenovaDeathRemapBlend
endif

[ResourceRaidenPuppetCommandResourceRikaJenovaDeathRemapBlend]
type = Buffer
stride = 32
filename = Dont\\Use\If\Statements\Or\SubCommands\In\Resource/SectionsRikaJenovaDeathRemapBlend.buf

[ResourceGIMINeedsResourcesToAllStartWithResourceRikaJenovaDeathRemapBlend]
type = Buffer
stride = 32
filename = ..\AAA\BBBB\CCCCCC/DDDDDRemapRikaJenovaDeathRemapBlend.buf

[ResourceRaidenShogunBlend.0]
type = Buffer
stride = 32
filename = ..\..\..\../../../../../../2-BunnyRaidenShogun\RaidenShogunBlend.buf

[ResourceEiBlendsHerBlenderInsteadOfHerSmoothie]
type = Buffer
stride = 32
if $swapmain == 1
\tfilename = M:\AnotherDrive\CuteLittleEi.buf
else
\trun = RaidenPuppetCommandResource
endif

[RaidenPuppetCommandResource]
type = Buffer
stride = 32
filename = ./Dont/Use\If/Statements\Or/SubCommands\In/Resource\Sections.buf

[GIMINeedsResourcesToAllStartWithResource]
type = Buffer
stride = 32
filename = ./../AAA/BBBB\CCCCCC\DDDDDRemapBlend.buf

[ResourceRaidenShogunBlend.0]
type = Buffer
stride = 32
filename = ..\..\..\../../../../../../2-BunnyRaidenShogun\RaidenShogunBlend.buf

[ResourceEiBlendsHerBlenderInsteadOfHerSmoothie]
type = Buffer
stride = 32
if $swapmain == 1
\tfilename = M:\AnotherDrive\CuteLittleEi.buf
else
\trun = RaidenPuppetCommandResource
endif

[RaidenPuppetCommandResource]
type = Buffer
stride = 32
filename = ./Dont/Use\If/Statements\Or/SubCommands\In/Resource\Sections.buf

[GIMINeedsResourcesToAllStartWithResource]
type = Buffer
stride = 32
filename = ./../AAA/BBBB\CCCCCC\DDDDDRemapBlend.buf

; --------------------------------------------"""
                    ]],
                    ["""
                     [boo]
                     tao = 1""", 0, [
                     """
                     [boo]
                     tao = 1

; --------------- GI Remap ---------------
; Mod remapped by Albert Gold#2696 and NK#1321. If you used it to remap your mods pls give credit for "Albert Gold#2696" and "Nhok0169"
; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support



; ----------------------------------------"""
                     ]]
                 ]

        for test in tests:
            iniTxt = test[0]
            expectedResourceCount = test[1]
            expectedIniTxt = test[2]

            self._iniFile.clear()
            self._iniFile._iniParser = self._parser
            self._iniFile._iniFixer = self._fixer

            self._iniFile.fileTxt = iniTxt

            self._iniFile.parse()
            resultFix = self._iniFile.fix()

            resultResources = self._iniFile.resources
            self.assertEqual(len(resultResources), expectedResourceCount)

            fixLen = len(resultFix)
            self.assertEqual(fixLen, len(expectedIniTxt))

            resultFix = list(resultFix.values())

            for i in range(fixLen):
                currentResultFix = resultFix[i]
                currentExpectedFix = expectedIniTxt[i]

                self.assertEqual(currentResultFix, currentExpectedFix)

    def test_differentInis_filteredBlendResourcesCollected(self):
        self.create()
        self._fixer.graphGroupEdits = [FRB.ResRegCollect({(0, "", "blend"): "vb1"}, {"blendType1": FRB.RemapBlendReplace((0, "", "remapBlend"))}, 
                                                         predicates = {(0, "", "blend"): lambda reg, val, iterData: "poopoo" in iterData.colouring},
                                                         trackKeys = True, keysToTrack = {(0, "", "blend"): "hash"})]

        tests = [
                    ["""
[TextureOverrideRaidenBlend1]
hash = poopoo
vb1 = ResourceRaidenBingo1

[TextureOverrideRaidenBlend2]
hash = apple
vb1 = ResourceRaidenBlend2
if $x == 1
    hash = baby
    vb1 = ResourceRaidenBlend3
    if $y == 2
        vb1 = ResourceRaidenBingo2
        hash = poopoo
        vb1 = ResourceRaidenBingo3
    else
        hash = cat
        vb1 = ResourceRaidenBlend4
    endif                
else
    hash = poopoo
    vb1 = ResourceRaidenBingo2
endif
                     
[TextureOverrideRaidenBlend3]
hash = doggy
                     
[ResourceRaidenBingo1]
filename = file1.buf
                     
[ResourceRaidenBingo2]
filename = file2.buf

[ResourceRaidenBingo3]
filename = file3.buf
""", 12, []]
                 ]

    def test_differentInis_textureCreateCollected(self):
        self.create()
        self._fixer.graphGroupEdits = [FRB.ResRegCollect({(0, "head", ""): "ps-t0", (0, "body", ""): "ps-t1"}, {"texType": FRB.TexCreate((0, "", "remapNormalTex"), "NormalMap", FRB.TexCreator(512, 512))})]

        tests = [
                    ["""
[TextureOverrideRaidenHead]
hash = 1bc3490d
;hash = 231723d2
match_first_index = 0
ib = ResourceBarbaraHeadIB
ps-t0 = ResourceBarbaraHeadDiffuse
ps-t1 = ResourceBarbaraHeadLightMap
ps-t2 = ResourceBarbaraHeadMetalMap
ps-t3 = ResourceBarbaraHeadShadowRamp

[SubSubBody]
ib = null
ps-t1 = ResourceBarbaraBodyLightMap3
run = NonExistentFunc

[SubBody]
if $x == 2
    ps-t1 = ResourceBarbaraBodyLightMap2
else if $x == 3
    run = SubSubBody
else
    ib = null
endif

[TextureOverrideRaidenBody]
hash = 1bc3490d
;hash = 231723d2
match_first_index = 12015

if $v == 1
    ib = ResourceBarbaraBodyIB
    ps-t0 = ResourceBarbaraBodyDiffuse
    ps-t1 = ResourceBarbaraBodyLightMap
    ps-t2 = ResourceBarbaraBodyMetalMap
    ps-t3 = ResourceBarbaraBodyShadowRamp
else
    run = SubBody
endif

[TextureOverrideRaidenDress]
hash = 1bc3490d
;hash = 231723d2
match_first_index = 46248
ib = ResourceBarbaraDressIB
ps-t0 = ResourceBarbaraDressDiffuse
ps-t1 = ResourceBarbaraDressLightMap
ps-t2 = ResourceBarbaraDressMetalMap
ps-t3 = ResourceBarbaraDressShadowRamp

[ResourceBarbaraHeadDiffuse]
filename = BarbaraHeadDiffuse.dds
filename = BarbaraHeadDiffuse2.dds
run = SubResource

[SubResource]
filename = BarbaraHeadDiffuse3.dds

[ResourceBarbaraBodyLightMap]
filename = BarbaraBodyLightMap.dds

[ResourceBarbaraBodyLightMap2]
filename = BarbaraBodyLightMap2.dds

[ResourceBarbaraBodyLightMap3]
filename = BarbaraBodyLightMap3.dds""", 4, [
"""
[TextureOverrideRaidenHead]
hash = 1bc3490d
;hash = 231723d2
match_first_index = 0
ib = ResourceBarbaraHeadIB
ps-t0 = ResourceBarbaraHeadDiffuse
ps-t1 = ResourceBarbaraHeadLightMap
ps-t2 = ResourceBarbaraHeadMetalMap
ps-t3 = ResourceBarbaraHeadShadowRamp

[SubSubBody]
ib = null
ps-t1 = ResourceBarbaraBodyLightMap3
run = NonExistentFunc

[SubBody]
if $x == 2
    ps-t1 = ResourceBarbaraBodyLightMap2
else if $x == 3
    run = SubSubBody
else
    ib = null
endif

[TextureOverrideRaidenBody]
hash = 1bc3490d
;hash = 231723d2
match_first_index = 12015

if $v == 1
    ib = ResourceBarbaraBodyIB
    ps-t0 = ResourceBarbaraBodyDiffuse
    ps-t1 = ResourceBarbaraBodyLightMap
    ps-t2 = ResourceBarbaraBodyMetalMap
    ps-t3 = ResourceBarbaraBodyShadowRamp
else
    run = SubBody
endif

[TextureOverrideRaidenDress]
hash = 1bc3490d
;hash = 231723d2
match_first_index = 46248
ib = ResourceBarbaraDressIB
ps-t0 = ResourceBarbaraDressDiffuse
ps-t1 = ResourceBarbaraDressLightMap
ps-t2 = ResourceBarbaraDressMetalMap
ps-t3 = ResourceBarbaraDressShadowRamp

[ResourceBarbaraHeadDiffuse]
filename = BarbaraHeadDiffuse.dds
filename = BarbaraHeadDiffuse2.dds
run = SubResource

[SubResource]
filename = BarbaraHeadDiffuse3.dds

[ResourceBarbaraBodyLightMap]
filename = BarbaraBodyLightMap.dds

[ResourceBarbaraBodyLightMap2]
filename = BarbaraBodyLightMap2.dds

[ResourceBarbaraBodyLightMap3]
filename = BarbaraBodyLightMap3.dds

; --------------- Raiden Remap ---------------
; Raiden remapped by Albert Gold#2696 and NK#1321. If you used it to remap your Raiden mods pls give credit for "Albert Gold#2696" and "Nhok0169"
; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

[TextureOverrideRaidenHead]
hash = 1bc3490d
match_first_index = 0
ib = ResourceBarbaraHeadIB
ps-t0 = ResourceRikaNormalMapRemapTex
ps-t1 = ResourceBarbaraHeadLightMap
ps-t2 = ResourceBarbaraHeadMetalMap
ps-t3 = ResourceBarbaraHeadShadowRamp

[TextureOverrideRaidenBody]
hash = 1bc3490d
match_first_index = 12015
if $v == 1
\tib = ResourceBarbaraBodyIB
\tps-t0 = ResourceBarbaraBodyDiffuse
\tps-t1 = ResourceRikaNormalMap1RemapTex
\tps-t2 = ResourceBarbaraBodyMetalMap
\tps-t3 = ResourceBarbaraBodyShadowRamp
else
\trun = SubBody
endif

[SubBody]
if $x == 2
\tps-t1 = ResourceRikaNormalMap2RemapTex
else if $x == 3
\trun = SubSubBody
else
\tib = null
endif

[SubSubBody]
ib = null
ps-t1 = ResourceRikaNormalMap3RemapTex
run = NonExistentFunc

[ResourceRikaNormalMapRemapTex]
filename = RikaNormalMapRemapTex.dds

[ResourceRikaNormalMap1RemapTex]
filename = RikaNormalMap1RemapTex.dds

[ResourceRikaNormalMap2RemapTex]
filename = RikaNormalMap2RemapTex.dds

[ResourceRikaNormalMap3RemapTex]
filename = RikaNormalMap3RemapTex.dds

; --------------------------------------------"""
]]
                 ]

        for test in tests:
            iniTxt = test[0]
            expectedResourceCount = test[1]
            expectedIniTxt = test[2]

            self._iniFile.clear()
            self._iniFile._iniParser = self._parser
            self._iniFile._iniFixer = self._fixer

            self._iniFile.fileTxt = iniTxt

            self._iniFile.parse()
            resultFix = self._iniFile.fix()
            resultResources = self._iniFile.resources

            self.assertEqual(len(resultResources), expectedResourceCount)

            fixLen = len(resultFix)
            self.assertEqual(fixLen, len(expectedIniTxt))

            resultFix = list(resultFix.values())
            for i in range(fixLen):
                currentResultFix = resultFix[i]
                currentExpectedFix = expectedIniTxt[i]

                self.assertEqual(currentResultFix, currentExpectedFix)

    # ===================================================================