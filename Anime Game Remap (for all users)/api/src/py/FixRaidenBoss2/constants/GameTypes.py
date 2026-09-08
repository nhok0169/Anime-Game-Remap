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
from typing import List, Optional
##### EndExtImports

##### CppLocalImports
from ..core import GameTypeId, GameTypeIdTools
##### EndCppLocalImports

##### LocalImports
from ..tools.Heading import Heading
##### EndLocalImports


##### Script
class GameTypes():
    """
    The supported games whose mods can be fixed :raw-html:`<br />` :raw-html:`<br />`

    The :class:`GameTypes` counterpart of :class:`ModTypes`, and used the same way -- by
    :class:`CommandBuilder` to list the games in the CLI's ``--help`` epilog, and by ``main.py`` to
    hand what the user typed to :class:`RemapServiceCLI` :raw-html:`<br />` :raw-html:`<br />`

    .. note::
        Unlike :class:`ModTypes` this is **not** an ``Enum`` and owns no data of its own. Every
        name and alias lives in :class:`GameTypeIdTools` on the C++ side, which is also what
        actually resolves one -- there is no registry to populate here, since the games *are* the
        :class:`GameTypeId` members. This class is only the presentation layer over it

    :raw-html:`<br />`

    .. tip::
        The names/aliases for the game types are not case sensitive
    """

    @classmethod
    def getAll(cls) -> List[GameTypeId]:
        """
        Retrieves all the game types available, in a stable order

        Returns
        -------
        List[:class:`GameTypeId`]
            All the available game types
        """

        return GameTypeIdTools.getAll()

    @classmethod
    def search(cls, txt: str) -> Optional[GameTypeId]:
        """
        Searches for a game type by name or alias, ignoring case and surrounding whitespace

        Parameters
        ----------
        txt: :class:`str`
            The text to search for a game's name/alias within

        Returns
        -------
        Optional[:class:`GameTypeId`]
            The found game type, if 'txt' names one
        """

        return GameTypeIdTools.findByName(txt)

    @classmethod
    def getHelpStr(cls, showFullGames: bool = True) -> str:
        """
        Retrieves the help text listing the supported games, for the CLI's ``--help`` epilog

        Parameters
        ----------
        showFullGames: :class:`bool`
            Whether each game is listed with its aliases, rather than by name alone :raw-html:`<br />`
            :raw-html:`<br />`

            Defaults to ``True``, where :meth:`ModTypes.getHelpStr`'s equivalent defaults to
            ``False``: there are only a handful of games, and their aliases are the entire point of
            being able to name one, so there is nothing to condense and nowhere else to send the
            user to read them

        Returns
        -------
        :class:`str`
            The help text for all the supported games
        """

        result = ""
        helpHeading = Heading("supported types of games", 15)
        result += f"{helpHeading.open()}\n\nThe names/aliases for the game types are not case sensitive\n\n"

        gameTypeHelpTxt = []
        for gameType in cls.getAll():
            if (showFullGames):
                currentHelpStr = GameTypeIdTools.getHelpStr(gameType)
            else:
                currentHelpStr = f"- {GameTypeIdTools.getName(gameType)}"

            gameTypeHelpTxt.append(currentHelpStr)

        gameTypeHelpTxt = "\n".join(gameTypeHelpTxt)

        result += f"{gameTypeHelpTxt}\n\n{helpHeading.close()}"
        return result
##### EndScript
