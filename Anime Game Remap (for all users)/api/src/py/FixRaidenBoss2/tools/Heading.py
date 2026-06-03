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


##### Script
class Heading():
    """
    Class for handling information about a heading for pretty printing

    Examples
    --------

    .. code-block:: python
        :linenos:
        :emphasize-lines: 1,3

        ======= Title: Fix Raiden Boss 2 =======
        ...
        ========================================

    Parameters
    ----------
    title: :class:`str`
        The title for the heading :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ""

    sideLen: :class:`int`
        The number of characters we want one side for the border of the opening heading to have :raw-html:`<br />` :raw-html:`<br />`

        **Default**: 0

    sideChar: :class:`str`
        The type of character we want the border for the heading to have  :raw-html:`<br />` :raw-html:`<br />`

        **Default**: "="

    Attributes
    ----------
    title: :class:`str`
        The title for the heading

    sideLen: :class:`int`
        The number of characters we want one side for the border of the opening heading to have

    sideChar: :class:`str`
        The type of character we want the border for the heading to have
    """

    def __init__(self, title: str = "", sideLen: int = 0, sideChar: str = "="):
        self.title = title
        self.sideLen = sideLen
        self.sideChar = sideChar

    def copy(self):
        """
        Makes a new copy of a heading

        Returns
        -------
        :class:`Heading`
            The new copy of the heading
        """
        return Heading(title = self.title, sideLen = self.sideLen, sideChar = self.sideChar)

    def open(self) -> str:
        """
        Makes the opening heading (see line 1 of the example at :class:`Heading`)

        Returns
        -------
        :class:`str`
            The opening heading created
        """

        side = self.sideLen * self.sideChar
        return f"{side} {self.title} {side}"

    def close(self) -> str:
        """
        Makes the closing heading (see line 3 of the example at :class:`Heading`)

        Returns
        -------
        :class:`str`
            The closing heading created
        """

        return self.sideChar * (2 * (self.sideLen + 1) + len(self.title))
##### EndScript