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
from enum import Enum
##### EndExtImports

##### LocalImports
from ..tools.Heading import Heading
##### EndLocalImports


##### Script
class IniKeywords(Enum):
    """
    Common keywords used in the .ini file
    """

    Hash = "hash"
    """
    The unique id for a part in the mod
    """

    Vb0 = "vb0"
    """
    Vertex buffer #0
    """

    Vb1 = "vb1"
    """
    Vertex buffer #1
    """

    Ib = "ib"
    """
    Index buffer
    """

    Handling = "handling"
    """
    How to handle some resource
    """

    Draw = "draw"
    """
    Location to draw a resource
    """

    DrawIndexed = "drawindexed"
    """
    How to draw the triangular of the model
    """

    Resource = "Resource"
    """
    The starting prefix used for any `sections`_ that reference some file
    """

    Blend = "Blend"
    """
    The substring that usually occurs in the name of a `section`_ to indicate that the `section`_ will call some *.Blend.buf file
    """

    Position = "Position"
    """
    The substring that usually occurs in the name of a `section`_ to indicate that the `section`_ will call some *.Position.buf file
    """

    Texcoord = "Texcoord"
    """
    The substring that usually occurs in the name of a `section`_ to indicate that the `section`_ will call some *.Texcoord.buf file
    """

    Run = "run"
    """
    The subsection that will be called from a certain `section`_
    """

    MatchFirstIndex = "match_first_index"
    """
    The index location to map some resource
    """

    Remap = f"Remap"
    """
    The substring used to indicate a `section`_ is editted by this software
    """

    RemapBlend = f"{Remap}{Blend}"
    """
    The substring used to indicate that the `section`_ references some *.RemapBlend.buf file
    """

    RemapPosition = f"{Remap}{Position}"
    """
    The substring used to indicate that the `section`_ references some *.RemapPosition.buf file
    """

    RemapTexcoord = f"{Remap}{Texcoord}"
    """
    The substring used to indicate that the `section`_ is called by ``[TextureOverride.*Texcoord.*]`` section.
    """

    RemapFix = f"{Remap}Fix"
    """
    The substring used to indicate that the `section`_ was created by this program 
    """

    RemapTex = f"{Remap}Tex"
    """
    The substring used to indicate that the `section`_ contains some editted/created texture *.Remap.dds file
    """

    RemapDL = f"{Remap}DL"
    """
    The substring used to indicate that the `section`_ contains some downloaded file from the internet
    """

    RemapIb = f"{Remap}IB"
    """
    The substring used to indicate that the `section`_ is called by ``[TextureOverride.*Ib.*]`` section.
    """

    Filename = f"filename"
    """
    The filename for some resource
    """

    HashNotFound = "HashNotFound"
    """
    The hash for a mod has not been found
    """

    IndexNotFound = "IndexNotFound"
    """
    The index for a mod has not been found
    """

    ORFixPath = r"CommandList\global\ORFix\ORFix"
    """
    The sub command call to `ORFix`_
    """

    TexFxFolder = r"CommandList\TexFx"
    """
    The folder to the sub command call to the `TexFx`_ module
    """

    HideOriginalComment = r";RemapFixHideOrig -->"
    """
    Comment used to hide the `sections`_ or the original character
    """


class IniBoilerPlate(Enum):
    """
    Boilerplate constants used for fixing a .ini file    

    Attributes
    ----------
    ShortModTypeNameReplaceStr: :class:`str`
        Placeholder for the shortened name of the mod to fix

    ModTypeNameReplaceStr: :class:`str`
        Placeholder for the name of the mod to fix

    Credit: :class:`str`
        The credit text used in the .ini file

    OldHeading: :class:`Heading`
        The heading used for .ini files fixed by an older version of this software

    DefaultHeading: :class:`Heading`
        The current heading used when fixing .ini files
    """

    ShortModTypeNameReplaceStr = "{{shortModTypeName}}"
    ModTypeNameReplaceStr = "{{modTypeName}}"
    Credit = f'\n; {ModTypeNameReplaceStr}remapped by Albert Gold#2696 and NK#1321. If you used it to remap your {ShortModTypeNameReplaceStr}mods pls give credit for "Albert Gold#2696" and "Nhok0169"\n; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support'

    OldHeading = Heading(".*Boss Fix", 15, "-")
    DefaultHeading = Heading(".*Remap", 15, "-")


class IniComments(Enum):
    GIMIObjMergerPreamble = """; This is really bad!! Don't do this!
; ************************************
;
; jk, but joking aside...
;
; The goal is to display n mod objects from the mod to be remapped to the mod onto a single mod object of the remapped mod.
;   Therefore we will have n sets of resources all mapping onto a single index (and same hash).
;
; Ideally, we would want all the sections to be within a single .ini file. The naive approach would be to create n sets of sections
;   (not a single section, cuz you need to include the case of sections depending on other sections, which form a section caller/callee graph) 
;    where the sections names are all unique. However, this approach will trigger a warning on GIMI (or any GIMI like importer) of multiple sections
;   mapping to the same hash and only 1 of the mod objects will be displayed
;
; The next attempt would be to take advantage of GIMI's overlapping mod bug/feature from loading multiple mods of the same character
;   Apart from the original .ini file, there would be n-1 newly generated .ini files (total of n .ini files). Each .ini file would uniquely
;   display a single set of sections from the n sets of sections. The overlapping property from the bug/feature would allow for all the objects to be displayed.
;
; For now, we were lazy and just simply copied the original .ini file onto the generated .ini files, which results in the original mod to have overlapping copies.
;  But since the mod used in all the .ini files are exactly the same, the user would not see the overlap (they may have some performance issues depending on the size of n. But
;   usually remaps only merge 2 mod objects into a single mod object, which should not cause much of an issue)
;   We could optimize the amount of space taken up by the newly generated .ini files, by only putting the necessary sections, but that is for another day..."""
##### EndScript