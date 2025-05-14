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
from typing import List, Tuple
##### EndExtImports

##### LocalImports
from ..tools.ListTools import ListTools
##### EndLocalImports


##### Script
class TextTools():
    @classmethod
    def removeParts(cls, txt: str, partIndices: List[Tuple[int, int]]) -> str:
        """
        Remove multiple substrings from a text based off the indices of the substrings

        Parameters
        ----------
        txt: :class:`str`
            The target txt to have the substrings removed

        partIndices: List[Tuple[:class:`int`, :class:`int`]]
            The indices for the substrings to be removed :raw-html:`<br />` :raw-html:`<br />`

            The tuples contain the following data:

                #. The start index for the substring
                #. The ending index for the substring

        Returns 
        -------
        :class:`str`
            The new string with the substrings removed
        """

        chars = list(txt)
        chars = ListTools.removeParts(chars, partIndices, lambda: 0, lambda element: element == 0)
        result = "".join(chars)
        return result


    @classmethod
    def removeLines(cls, txtLines: List[str], partIndices: List[Tuple[int, int]]) -> List[str]:
        """
        Removes multiple sub-lists of lines from a list of text lines

        Parameters
        ----------
        txtLines: List[:class:`str`]
            The lines of text to have its lines removed

        partIndices: List[Tuple[:class:`int`, :class:`int`]]
            The indices for the list of lines to be removed :raw-html:`<br />` :raw-html:`<br />`

            The tuples contain the following data:

                #. The start index for the list of lines
                #. The ending index for the list of lines

        Returns 
        -------
        List[:class:`str`]
            The new lines of text with the removed lines
        """

        result = ListTools.removeParts(txtLines, partIndices, lambda: 0, lambda element: element == 0)
        return result
    
    @classmethod
    def getTextLines(cls, txt: str) -> List[str]:
        """
        Retrieves the lines of text, split by the newline character, similar to how python's `readlines`_ function works

        Parameters
        ----------
        txt: :class:`str`
            The target text to be split

        Returns
        -------
        List[:class:`str`]
            The lines of text that were split
        """

        txtLines = txt.split("\n")

        if (txt):
            txtLinesLen = len(txtLines)
            for i in range(txtLinesLen):
                if (i < txtLinesLen - 1):
                    txtLines[i] += "\n"
        else:
            txtLines = []

        return txtLines
    
    @classmethod
    def capitalize(cls, txt: str) -> str:
        """
        Capitalize the beginning letter of 'txt'

        Parameters
        ----------
        txt: :class:`str`
            The text to be capitalized

        Returns
        -------
        :class:`str`
            The new text with its first letter capitalized
        """

        if (not txt):
            return txt
        elif (len(txt) == 1):
            return txt.upper()
        
        return txt[0].upper() + txt[1:]
    
    @classmethod
    def capitalizeOnlyFirstChar(cls, txt: str) -> str:
        """
        Capitalize only the beginning letter of 'txt' while leaving the rest
        of 'txt' as lowercase

        Parameters
        ----------
        txt: :class:`str`
            The text to be capitalized

        Returns
        -------
        :class:`str`
            The new text with only the first letter capitalized
        """

        return cls.capitalize(txt.lower())
    
    @classmethod
    def reverse(cls, txt: str) -> str:
        """
        Reverses a string

        Parameters
        ----------
        txt: :class:`str`
            The text to be reversed

        Returns
        -------
        :class:`str`
            The reversed string
        """

        return txt[::-1]
##### EndScript