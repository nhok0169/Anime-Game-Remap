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
import os
import re
import pathlib
from typing import Tuple, Optional
##### EndExtImports

##### LocalImports
from ..constants.FileExt import FileExt
from ..constants.IniConsts import IniKeywords
from ..tools.TextTools import TextTools
##### EndLocalImports


##### Script
class IniNamingTools():
    """
    Utilities for some common naming conventions for .ini files
    """

    @classmethod
    def getResourceName(cls, name: str) -> str:
        """
        Makes the name of a `section`_ to be used for the resource `sections`_ of a .ini file

        Examples
        --------
        >>> IniNamingTools.getResourceName("CuteLittleEi")
        "ResourceCuteLittleEi"


        >>> IniNamingTools.getResourceName("ResourceCuteLittleEi")
        "ResourceCuteLittleEi"

        Parameters
        ----------
        name: :class:`str`
            The name of the `section`_

        Returns
        -------
        :class:`str`
            The name of the `section`_ as a resource in a .ini file
        """

        if (not name.startswith(IniKeywords.Resource.value)):
            name = f"{IniKeywords.Resource.value}{name}"
        return name
    
    @classmethod
    def removeResourceName(cls, name: str) -> str:
        """
        Removes the 'Resource' prefix from a section's name

        Examples
        --------
        >>> IniNamingTools.removeResourceName("ResourceCuteLittleEi")
        "CuteLittleEi"


        >>> IniNamingTools.removeResourceName("LittleMissGanyu")
        "LittleMissGanyu"

        Parameters
        ----------
        name: :class:`str`
            The name of the `section`_

        Returns
        -------
        :class:`str`
            The name of the `section`_ with the 'Resource' prefix removed
        """

        if (name.startswith(IniKeywords.Resource.value)):
            name = name[len(IniKeywords.Resource.value):]

        return name
    
    @classmethod
    def getRemapElementName(cls, name: str, elementName: str, modName: str = ""):
        """
        Changes a `section`_ name to have the keyword from 'elementName' to identify that the `section`_
        is created by this fix

        Examples
        --------
        >>> IniNamingTools.getRemapElementName("EiTriesToUseBlenderAndFails", "Blend", "Raiden")
        "EiTriesToUseRaidenRemapBlenderAndFails"


        >>> IniNamingTools.getRemapElementName("EiTextsTheTexture", "Tex", "Yae")
        "EiTextsTheYaeRemapTexture"
    

        >>> IniNamingTools.getRemapElementName("ResourceCuteLittleEi", "Position", "Raiden")
        "ResourceCuteLittleEiRaidenRemapPosition"


        >>> IniNamingTools.getRemapElementName("ResourceCuteLittleEiRemapDango", "Dango" "Raiden")
        "ResourceCuteLittleEiRemapRaidenRemapDango"

        Parameters
        ----------
        name: :class:`str`
            The name of the `section`_

        elementName: :class:`str`
            The name of the target element

        modName: :class:`str`
            The name of the mod to fix :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``""``

        Returns
        -------
        :class:`str`
            The name of the `section`_ with the keyword of 'elementName', prefixed by the word 'Remap' added
        """

        nameParts = name.rsplit(elementName, 1)
        namePartsLen = len(nameParts)

        remapName = f"{modName}{IniKeywords.Remap.value}{elementName}"
        if (namePartsLen > 1):
            name = remapName.join(nameParts)
        else:
            name += remapName

        return name
    
    @classmethod
    def getRemapBlendName(cls, name: str, modName: str = "") -> str:
        """
        Changes a `section`_ name to have the keyword 'RemapBlend' to identify that the `section`_
        is created by this fix

        .. tip::
            See :meth:`getRemapElementName` for some examples

        Parameters
        ----------
        name: :class:`str`
            The name of the `section`_

        modName: :class:`str`
            The name of the mod to fix :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``""``

        Returns
        -------
        :class:`str`
            The name of the `section`_ with the added 'RemapBlend' keyword
        """

        return cls.getRemapElementName(name, elementName = IniKeywords.Blend.value, modName = modName)
    
    @classmethod
    def getRemapPositionName(cls, name: str, modName: str = "") -> str:
        """
        Changes a `section`_ name to have the keyword 'RemapPosition' to identify that the `section`_
        is created by this fix

        .. tip::
            See :meth:`getRemapElementName` for some examples

        Parameters
        ----------
        name: :class:`str`
            The name of the `section`_

        modName: :class:`str`
            The name of the mod to fix :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``""``

        Returns
        -------
        :class:`str`
            The name of the `section`_ with the added 'RemapPosition' keyword
        """

        return cls.getRemapElementName(name, elementName = IniKeywords.Position.value, modName = modName)
    
    @classmethod
    def getRemapTexcoordName(cls, name: str, modName: str = "") -> str:
        """
        Changes a `section`_ name to have the keyword 'RemapTexcoord' to identify that the `section`_
        is created by this fix

        .. tip::
            See :meth:`getRemapElementName` for some examples

        Parameters
        ----------
        name: :class:`str`
            The name of the `section`_

        modName: :class:`str`
            The name of the mod to fix :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``""``

        Returns
        -------
        :class:`str`
            The name of the `section`_ with the added 'RemapTexcoord' keyword
        """

        return cls.getRemapElementName(name, elementName = IniKeywords.Texcoord.value, modName = modName)
    
    @classmethod
    def getRemapIbName(cls, name: str, modName: str = "") -> str:
        """
        Changes a `section`_ name to have the keyword 'RemapIb' to identify that the `section`_
        is created by this fix

        .. tip::
            See :meth:`getRemapElementName` for some examples

        Parameters
        ----------
        name: :class:`str`
            The name of the `section`_

        modName: :class:`str`
            The name of the mod to fix :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``""``

        Returns
        -------
        :class:`str`
            The name of the `section`_ with the added 'RemapIb' keyword
        """

        return cls.getRemapElementName(name, elementName = "IB", modName = modName)
    
    @classmethod
    def getModSuffixedName(cls, name: str, suffix: str = "", modName: str = ""):
        """
        Changes a `section`_ name to have the suffix of 'modName' followed by 'suffix'

        Parameters
        ----------
        name: :class:`str`
            The name of the `section`_

        suffix: :class:`str`
            The name of the suffix to put at the end of the `section`_ :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``""``

        modName: :class:`str`
            The name of the mod to fix :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``""``

        Returns
        -------
        :class:`str`
            The name of the `section`_ with the added suffix keyword
        """

        remapName = f"{modName}{suffix}"
        if (name.endswith(remapName)):
            return name
        elif(name.endswith(suffix)):
            return name[:len(suffix)] + remapName

        return name + remapName
    
    @classmethod
    def getRemapFixName(cls, name: str, modName: str = "") -> str:
        """
        Changes a `section`_ name to have the suffix `RemapFix` to identify that the `section`_
        is created by this fix

        Examples
        --------
        >>> IniNamingTools.getRemapFixName("EiIsDoneWithRemapFix", "Raiden")
        "EiIsDoneWithRaidenRemapFix"

        >>> IniNamingTools.getRemapFixName("EiIsHappy", "Raiden")
        "EiIsHappyRaidenRemapFix"

        Parameters
        ----------
        name: :class:`str`
            The name of the `section`_

        modName: :class:`str`
            The name of the mod to fix :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``""``

        Returns
        -------
        :class:`str`
            The name of the `section`_ with the added 'RemapFix' keyword
        """

        return cls.getModSuffixedName(name, suffix = IniKeywords.RemapFix.value, modName = modName)
    
    @classmethod
    def getRemapTexName(cls, name: str, modName: str = ""):
        """
        Changes a `section`_ name to have the keyword 'RemapTex' to identify that the `section`_
        is created by this fix

        Examples
        --------
        >>> IniNamingTools.getRemapTexName("EiIsDoneWithRemapTex", "Raiden")
        "EiIsDoneWithRaidenRemapTex"

        >>> IniNamingTools.getRemapTexName("EiIsHappy", "Raiden")
        "EiIsHappyRaidenRemapTex"

        Parameters
        ----------
        name: :class:`str`
            The name of the `section`_

        modName: :class:`str`
            The name of the mod to fix :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``""``

        Returns
        -------
        :class:`str`
            The name of the `section`_ with the added 'RemapFix' keyword
        """

        return cls.getModSuffixedName(name, suffix = IniKeywords.RemapTex.value, modName = modName)
    
    @classmethod
    def getRemapDLName(cls, name: str, modName: str = ""):
        """
        Changes a `section`_ name to have the suffix `RemapDL` to identify that the `section`_
        is created by this fix

        Examples
        --------
        >>> IniNamingTools.getRemapTexName("EiIsDoneWithRemapDL", "Raiden")
        "EiIsDoneWithRaidenRemapDL"

        >>> IniNamingTools.getRemapTexName("EiIsHappy", "Raiden")
        "EiIsHappyRaidenRemapDL"

        Parameters
        ----------
        name: :class:`str`
            The name of the `section`_

        modName: :class:`str`
            The name of the mod to fix :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``""``

        Returns
        -------
        :class:`str`
            The name of the `section`_ with the added 'RemapDL' keyword
        """

        return cls.getModSuffixedName(name, suffix = IniKeywords.RemapDL.value, modName = modName)

    @classmethod
    def getRemapFixResourceName(cls, name: str, modName: str = ""):
        """
        Changes a `section`_ name to be a new non-blend resource created by this fix

        .. note::
            See :meth:`getResourceName` and :meth:`getRemapFix` for more info

        Parameters
        ----------
        name: :class:`str`
            The name of the section

        modName: :class:`str`
            The name of the mod to fix :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``""``

        Returns
        -------
        :class:`str`
            The name of the section with the prefix 'Resource' and the suffix 'RemapFix' added
        """

        name = cls.getRemapFixName(name, modName = modName)
        name = cls.getResourceName(name)
        return name
    
    @classmethod
    def getRemapTexResourceName(cls, name: str, modName: str = ""):
        """
        Changes a `section`_ name to be a texture resource created by this fix

        .. note::
            See :meth:`getResourceName` and :meth:`getRemapTexName` for more info

        Parameters
        ----------
        name: :class:`str`
            The name of the section

        modName: :class:`str`
            The name of the mod to fix :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``""``

        Returns
        -------
        :class:`str`
            The name of the section with the prefix 'Resource' and the suffix 'RemapTex' added
        """

        name = cls.getRemapTexName(name, modName = modName)
        name = cls.getResourceName(name)
        return name
    
    @classmethod
    def getRemapDLResourceName(cls, name: str, modName: str = ""):
        """
        Changes a `section`_ name to be a texture resource created by this fix

        .. note::
            See :meth:`getResourceName` and :meth:`getRemapDLName` for more info

        Parameters
        ----------
        name: :class:`str`
            The name of the section

        modName: :class:`str`
            The name of the mod to fix :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``""``

        Returns
        -------
        :class:`str`
            The name of the section with the prefix 'Resource' and the suffix 'RemapDL' added
        """

        name = cls.getRemapDLName(name, modName = modName)
        name = cls.getResourceName(name)
        return name

    @classmethod
    def getRemapBlendResourceName(cls, name: str, modName: str = "") -> str:
        """
        Changes the name of a section to be a new blend resource that this fix will create

        .. note::
            See :meth:`getResourceName` and :meth:`getRemapBlendName` for more info

        Parameters
        ----------
        name: :class:`str`
            The name of the section

        modName: :class:`str`
            The name of the mod to fix :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``""``

        Returns
        -------
        :class:`str`
            The name of the section with the prefix 'Resource' and the keyword 'Remap' added
        """

        name = cls.getRemapBlendName(name, modName = modName)
        name = cls.getResourceName(name)
        return name
    
    @classmethod
    def getRemapPositionResourceName(cls, name: str, modName: str = "") -> str:
        """
        Changes the name of a section to be a new position resource that this fix will create

        .. note::
            See :meth:`getResourceName` and :meth:`getRemapPositionName` for more info

        Parameters
        ----------
        name: :class:`str`
            The name of the section

        modName: :class:`str`
            The name of the mod to fix :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``""``

        Returns
        -------
        :class:`str`
            The name of the section with the prefix 'Resource' and the keyword 'Remap' added
        """

        name = cls.getRemapPositionName(name, modName = modName)
        name = cls.getResourceName(name)
        return name
    
    @classmethod
    def getFixedFile(cls, file: str, modName: str = "", fileExt: Optional[str] = None) -> str:
        """
        Retrieves the file path for a a fixed element

        Parameters
        ----------
        file: :class:`str`
            The file path to the original file

        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``""``

        fileExt: Optional[:class:`str`]
            The file extension for the file path of the fixed element.
            If this argument is ``None``, then will use the file extension provided in the file :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        Returns
        -------
        :class:`str`
            The file path of the fixed file of the element
        """

        path = pathlib.Path(file)
        folder = path.parent
        baseName = path.stem
        
        if (fileExt is None):
            fileExt = path.suffix
        
        return os.path.join(f"{folder}", f"{cls.getRemapFixName(baseName, modName = modName)}{fileExt}")
    
    @classmethod
    def getFixedElementFile(cls, file: str, elementName: str, modName: str = "", fileExt: Optional[str] = None) -> str:
        """
        Retrieves the file path for a a fixed element

        Parameters
        ----------
        file: :class:`str`
            The file path to the original file

        elementName: :class:`str`
            The name of the element to fix

        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``""``

        fileExt: Optional[:class:`str`]
            The file extension for the file path of the fixed element.
            If this argument is ``None``, then will use the file extension provided in the file :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        Returns
        -------
        :class:`str`
            The file path of the fixed file of the element
        """

        path = pathlib.Path(file)
        folder = path.parent
        baseName = path.stem
        
        if (fileExt is None):
            fileExt = path.suffix

        file = f"{cls.getRemapElementName(baseName, elementName, modName = modName)}{fileExt}"
        if (folder == pathlib.Path(".")):
            return file
        
        return os.path.join(f"{folder}", file)

    @classmethod
    def getFixedBlendFile(cls, blendFile: str, modName: str = "") -> str:
        """
        Retrieves the file path for the fixed RemapBlend.buf file

        Parameters
        ----------
        blendFile: :class:`str`
            The file path to the original Blend.buf file

        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``""``

        Returns
        -------
        :class:`str`
            The file path of the fixed RemapBlend.buf file
        """

        return cls.getFixedElementFile(blendFile, IniKeywords.Blend.value, modName = modName, fileExt = FileExt.Buf.value)
    
    @classmethod
    def getFixedPositionFile(cls, positionFile: str, modName: str = "") -> str:
        """
        Retrieves the file path for the fixed RemapPosition.buf file

        Parameters
        ----------
        positionFile: :class:`str`
            The file path to the original Position.buf file

        modName: :class:`str`
            The name of the mod to fix to

        Returns
        -------
        :class:`str`
            The file path of the fixed RemapPosition.buf file
        """

        return cls.getFixedElementFile(positionFile, IniKeywords.Position.value, modName = modName, fileExt = FileExt.Buf.value)
    
    @classmethod
    def getFixedTexFile(cls, texFile: str, modName: str = "") -> str:
        """
        Retrieves the file path for the fixed RemapTex.dds file

        Parameters
        ----------
        texFile: :class:`str`
            The file path to the original .dds file

        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``""``

        Returns
        -------
        :class:`str`
            The file path of the fixed RemapTex.dds file
        """

        blendFolder = os.path.dirname(texFile)
        blendBaseName = os.path.basename(texFile)
        blendBaseName = blendBaseName.rsplit(".", 1)[0]

        return os.path.join(blendFolder, f"{cls.getRemapTexName(blendBaseName, modName = modName)}{FileExt.DDS.value}")
    
    @classmethod
    def getTextureOverrideRemapFix(cls, component: str, obj: str, modName: str = ""):
        """
        Retrieves the name to some generic ``TextureOverride`` `section`_ this software has made

        Parameters
        ----------
        component: :class:`str`
            The name of the component

        obj: :class:`str`
            The name of the object

        modName: :class:`str`
            The name of the mod  :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``""``

        Returns
        -------
        :class:`str`
            The name for the `section`_
        """

        return cls.getRemapFixName(f"{IniKeywords.TextureOverride.value}{TextTools.capitalize(modName)}{TextTools.capitalize(component)}{TextTools.capitalize(obj)}")
    
    @classmethod
    def getObjRemapFixName(cls, name: str, modName: str, objName: Tuple[str, str], newObjName: Tuple[str, str]) -> str:
        """
        Retrieves the new name of the `section`_ for a new mod object

        Parameters
        ----------
        name: :class:`str`
            The name of the `section`_

        modName: :class:`str`
            The name of the mod to be fixed

        objName: Tuple[:class:`str`, :class:`str`]
            The name of the component and object for the original mod object for the `section`_

        newObjName: Tuple[:class:`str`, :class:`str`]
            The name of the component and object for the new mod object for the `section`_

        Returns
        -------
        :class:`str`
            The new name for the `section`_
        """


        name = TextTools.reverse(name)
        objName = f"{TextTools.capitalize(objName[0])}{TextTools.capitalize(objName[1])}"
        newObjName = f"{TextTools.capitalize(newObjName[0])}{TextTools.capitalize(newObjName[1])}"
        modName = TextTools.capitalize(modName)
    
        nameParts = re.split(TextTools.reverse(objName), name, flags = re.IGNORECASE, maxsplit = 1)
        namePartsLen = len(nameParts)

        if (namePartsLen == 1):
            name = TextTools.reverse(name)
            return cls.getRemapFixName(name, modName = f"{modName}{newObjName}")

        name = TextTools.reverse(newObjName).join(nameParts)
        name = TextTools.reverse(name)
        return cls.getRemapFixName(name, modName = modName)
##### EndScript