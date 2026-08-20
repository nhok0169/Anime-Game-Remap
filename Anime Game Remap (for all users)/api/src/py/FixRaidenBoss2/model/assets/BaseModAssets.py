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
from typing import Generic, List, Optional, Union, Hashable, Dict
##### EndExtImports

##### LocalImports
from ...constants.GenericTypes import T
from ...tools.DictTools import UnHashableNone
##### EndLocalImports


##### Script
class BaseModAssets(Generic[T]):
    """
    Base class for retrieving an asset from a mod
    """

    VersionKey = "version"
    NameKey = "name"
    ValueKey = "value"

    def _convertIndexVals(self, indexVals: Union[Hashable, List[Hashable], Dict[str, Hashable]], indexOrder: List[str]) -> Dict[str, Hashable]:
        indexKeysLen = len(indexOrder)

        if (isinstance(indexVals, list)):
            newIndexVals = {}
            indexValsLen = len(indexVals)

            for i in range(indexKeysLen):
                newIndexVals[indexOrder[i]] = indexVals[i] if (i < indexValsLen) else UnHashableNone

            indexVals = newIndexVals

        elif (not isinstance(indexVals, dict)):
            newIndexVals = {}
            for i in range(indexKeysLen):
                newIndexVals[indexOrder[i]] = indexVals if (i == 0) else UnHashableNone

            indexVals = newIndexVals
        
        else:
            newIndexVals = {}
            for i in range(indexKeysLen):
                index = indexOrder[i]
                newIndexVals[index] = indexVals[index] if (index in indexVals) else UnHashableNone

            indexVals = newIndexVals

        return indexVals

    @staticmethod
    def toWildcardList(indexVals: Optional[Union[Hashable, List[Hashable], Dict[str, Hashable]]], indexOrder: List[str]) -> List[Optional[Hashable]]:
        """
        Normalizes a flexible non-version-values argument into a plain positional list, with
        ``None`` filling any position 'indexVals' doesn't specify a value for :raw-html:`<br />` :raw-html:`<br />`

        Same normalization :meth:`_convertIndexVals` does (accepts a bare :class:`Hashable`, a
        :class:`list`, or a :class:`dict` keyed by index name), but returns a plain ``List[Optional[Hashable]]``
        with ``None`` as the "no value given" marker instead of a name-keyed dict using
        :class:`UnHashableNone` -- the shape the C++-backed :class:`ModMappedAssets`'s ``getKey``/
        ``hasFrom``/``replace`` expect for their ``fromNonVersionVals``/``nonVersionVals``
        arguments. ``None`` (or :class:`UnHashableNone`) for 'indexVals' itself means "no values
        given at all" (every position wildcarded)

        Parameters
        ----------
        indexVals: Optional[Union[`Hashable`_, List[`Hashable`_], Dict[:class:`str`, `Hashable`_]]]
            The raw, flexibly-shaped filter values to normalize

        indexOrder: List[:class:`str`]
            The names of the non-version indices, in position order

        Returns
        -------
        List[Optional[`Hashable`_]]
            The normalized, positional filter values
        """

        indexKeysLen = len(indexOrder)

        if (indexVals is None or isinstance(indexVals, UnHashableNone) or indexVals is UnHashableNone):
            return [None] * indexKeysLen

        if (isinstance(indexVals, list)):
            indexValsLen = len(indexVals)
            return [(indexVals[i] if (i < indexValsLen) else None) for i in range(indexKeysLen)]

        if (isinstance(indexVals, dict)):
            return [(indexVals[indexOrder[i]] if (indexOrder[i] in indexVals) else None) for i in range(indexKeysLen)]

        # a bare Hashable goes in the first position
        return [(indexVals if (i == 0) else None) for i in range(indexKeysLen)]

    def clearCache(self):
        """
        Clear any saved cached values in the data structure
        """

        pass

    def get(self, *args, **kwargs) -> T:
        """
        Retrieves the corresponding asset

        Returns
        -------
        T
            The found assets
        """

        pass
##### EndScript