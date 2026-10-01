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

    TextureOverride = "TextureOverride"
    """
    The starting prefix used for some `section`_ that overrides the resource of a modss
    """

    Then = "then"
    """
    The keyword after the logical predicate of an if statement
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

    RemapRef = f"{Remap}Ref"
    """
    The substring used to indicate that a resource `section`_ inside a fix's block names one of the MOD's own files,
    only referenced by the fix -- an undo removes the section and keeps the file (see the C++ ``IniKeywords::RemapRef``)
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

    NNFixPath = r"CommandList\global\ORFix\NNFix"
    """
    The sub command to call `ORFix` for mods without normal maps
    """

    TexFxFolder = r"CommandList\TexFx"
    """
    The folder to the sub command call to the `TexFx`_ module
    """

    TexFxShortTransparency0 = TexFxFolder + r"\T.0"
    """
    Short alias of transparency sub command in `TexFx`_ module mapping to ps-t0
    """

    TexFxShortTransparency1 = TexFxFolder + r"\T.1"
    """
    Short alias of transparency sub command in `TexFx`_ module mapping to ps-t1
    """

    TexFxShortTransparency0Natlan = TexFxFolder + r"\TN.0"
    """
    Short alias of transparency sub command in `TexFx`_ module mapping to ps-t0 for GI version 5.0 +
    """

    TexFxShortTransparency1Natlan = TexFxFolder + r"\TN.1"
    """
    Short alias of transparency sub command in `TexFx`_ module mapping to ps-t1 for GI version 5.0 +
    """

    HideOriginalComment = r";RemapFixHideOrig -->"
    """
    Comment used to hide the `sections`_ or the original character
    """

    Null = "null"
    """
    Null value for some `KVP`_
    """

    ResourceGroup = "RG"
    """
    The substring to indicate that a resource `section`_ that was copied to satisfy a group of resources
    """


class IniGraphModObjKeywords(Enum):
    """
    Keywords for the component/mod object ids of the graphs used in :class:`IniGraphGroup`
    """

    Download = "download"
    """
    Used to indicate the `sections`_ for the graph are some download resources
    """
##### EndScript