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
from typing import List, Dict, Tuple, Optional
##### EndExtImports

##### CppLocalImports
from ...core import CppIfContentPart, IOrderedMultiMap
##### EndCppLocalImports

##### LocalImports
from .IfTemplatePart import IfTemplatePart
##### EndLocalImports


##### Script
class IfContentPart(CppIfContentPart, IfTemplatePart):
    """
    This class inherits from :class:`CppIfContentPart` and :class:`IfTemplatePart`

    Class for defining the content part of an :class:`IfTemplate`

    .. note::
        see :class:`IfTemplate` for more details

    :raw-html:`<br />`

    .. container:: operations

        **Supported Operations:**

        .. describe:: key in x

            Determines if 'key' exists in the content part

        .. describe:: len(x)

            Retrieves the number of KVPs in the content part

        .. describe:: x[index]

            Retrieves the ``(key, value)`` pair at the given true positional index, if ``index`` is
            an :class:`int`

        .. describe:: x[key]

            Retrieves every value currently stored under ``key``, in true positional order
            (equivalent to :meth:`getVals` with ``ordered=True``), if ``key`` is a :class:`str` --
            raises :class:`KeyError` if ``key`` doesn't exist

        .. describe:: iter(x)

            Iterates every KVP in true positional order, yielding ``(key, value, occurrenceIndex,
            orderIndex)`` tuples

    Parameters
    ----------
    src: Optional[Dict[Any, List[Tuple[:class:`int`, Any]]]]
        Initial data to populate ``content`` with, as key -> list of ``(index, value)`` pairs, one
        entry per occurrence of that key :raw-html:`<br />` :raw-html:`<br />`

        ``index`` orders every occurrence relative to every other occurrence *across all keys*
        (gathered, stable-sorted by ``index`` ascending, then appended in that order); it is **not**
        a strict absolute position, so gaps and duplicate/out-of-order values are fine. If
        ``content`` already holds data (a pre-populated instance was passed in rather than left to
        default), ``src``'s entries are appended after it, not merged/interleaved with it.
        :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    depth: :class:`int`
        The depth this part is within the owning `IfTemplate` :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``0``

    content: Optional[:class:`IOrderedMultiMap`]
        The backing ordered-multimap implementation to use, taken by ownership -- see :class:`CppIfContentPart`
        top-level warning about what that means for 'content' afterward :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``, meaning a fresh, empty :class:`CppOrderedMultiMap` is used

    id: Optional[:class:`int`]
        The id for the part. If this parameter is ``None``, will generate a new id for the part. :raw-html:`<br />` **OR** :raw-html:`<br />`

        **Default**: ``None``
    """

    def __init__(self, src: Dict[str, List[Tuple[int, str]]] = None, depth: int = 0, content: Optional[IOrderedMultiMap] = None, id: Optional[int] = None):
        CppIfContentPart.__init__(self, src = src, depth = depth, content = content)
        IfTemplatePart.__init__(self, id = id)

    def clone(self, newId: bool = False) -> "IfContentPart":
        """
        Creates a deep copy of this part, at the same depth

        Parameters
        ----------
        newId: :class:`bool`
            Whether to generate a new id for the part :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``False``

        Returns
        -------
        :class:`IfContentPart`
            The cloned part
        """

        return type(self)(depth = self.depth, content = self.content.clone(), id = None if (newId) else self._id)

    def __copy__(self) -> "IfContentPart":
        """
        Creates a copy of this part (equivalent to :meth:`clone`); supports ``copy.copy()``
        """

        return self.clone()

    def __deepcopy__(self, memo: dict) -> "IfContentPart":
        """
        Creates a deep copy of this part (equivalent to :meth:`clone`); supports ``copy.deepcopy()``
        """

        return self.clone()
##### EndScript