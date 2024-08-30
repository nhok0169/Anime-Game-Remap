import sys
import os

sys.path.insert(1, '../../Fix-Raiden-Boss 2.0 (for all user )/api')
import src.FixRaidenBoss2.FixRaidenBoss2 as FRB


showWackyRaidenIniTxtWithFix = r"""
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

[RaidenPuppetCommandResource]
type = Buffer
stride = 32
filename = ./Dont/Use\If/Statements\Or/SubCommands\In/Resource\Sections.buf

; ------ some lines originally generated from the fix ---------

[ResourceEiBlendsHerRemapBlenderInsteadOfHerSmoothie]
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
filename = ..\..\..\..\..\..\..\..\..\2-BunnyRaidenShogun\RaidenShogunRemapBlend.buf

[RaidenPuppetCommandResourceRemapBlend]
type = Buffer
stride = 32
filename = Dont\Use\If\Statements\Or\SubCommands\In\Resource\SectionsRemapBlend.buf

; --------------------------------------------------------------


; --------------- Raiden Boss Fix -----------------
; Raiden boss fixed by NK#1321 if you used it for fix your raiden pls give credit for "Nhok0169"
; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 and Albert Gold#2696 for support

[TextureOverrideRaidenShogunRemapBlend]
run = CommandListRaidenShogunRemapBlend
handling = skip
draw = 21916,0

[CommandListRaidenShogunRemapBlend]
if $swapmain == 0
    if $swapvar == 0 && $swapvarn == 0
    vb1 = ResourceRaidenShogunRemapBlend.0
    else
    vb1 = ResourceEiBlendsHerRemapBlenderInsteadOfHerSmoothie
    endif
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
type = Buffer
stride = 32
filename = ..\..\..\..\..\..\..\..\..\2-BunnyRaidenShogun\RaidenShogunRemapBlend.buf

[RaidenPuppetCommandResourceRemapBlend]
type = Buffer
stride = 32
filename = Dont\Use\If\Statements\Or\SubCommands\In\Resource\SectionsRemapBlend.buf


; -------------------------------------------------
"""

iniFile = FRB.IniFile(txt = showWackyRaidenIniTxtWithFix, modTypes = FRB.ModTypes.getAll())
fixCode = iniFile.removeFix(keepBackups = False)

iniPath = os.path.join(os.path.dirname(os.path.abspath(__file__)), "IniWithFixRemoved.ini")
with open(iniPath, "w", encoding = "utf-8") as f:
    f.write(fixCode)
