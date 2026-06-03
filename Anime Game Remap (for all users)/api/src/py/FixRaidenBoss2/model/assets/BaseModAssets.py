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
from typing import Generic, List, Union, Hashable, Dict
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