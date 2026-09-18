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
import uuid
from typing import Type, Any, List, Dict, Hashable, Optional
##### EndExtImports

##### LocalImports
from ..constants.GenericTypes import BuildCls
from .Builder import Builder
##### EndLocalImports


##### Script
class FlyweightBuilder(Builder[BuildCls]):
    """
    This class inherits from :class:`Builder`

    A flyweight builder for building the same reusable objects (based off `flyweight design pattern`_)

    Parameters
    ----------
    buildCls: Type[T]
        The class for the objects to be built from

    args: Optional[List[Any]]
        The constant arguments used to build the object :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    kwargs: Optional[Dict[str, Any]]
        The constant keyword arguments used to build the object :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``
    """

    def __init__(self, buildCls: Type[BuildCls], args: Optional[List[Any]] = None, kwargs: Optional[Dict[str, Any]] = None):
        super().__init__(buildCls, args, kwargs)
        self._cache = {}

    def build(self, args: Optional[List[Any]] = None, kwargs: Optional[Dict[str, Any]] = None, id: Optional[Hashable] = None, cache: bool = True) -> BuildCls:
        """
        Builds the object

        Parameters
        ----------
        args: Optional[List[Any]]
            arguments to build the object :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        kwargs: Optional[Dict[str, Any]]
            keyword arguments to build the object :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        id: Optional[Hashable]
            The id for the repeating states to be built by the object :raw-html:`<br />` :raw-html:`<br />`

            If this value is ``None``, then will auto-generate an id :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        cache: :class:`bool`
            Whether to cache the built object

            .. note::
                If this value is set to ``False``, then this function behaves the same as :meth:`Builder.build`

            **Default**: ``True``

        Returns
        -------
        T
            The built objects
        """

        if (args is None):
            args = []

        if (kwargs is None):
            kwargs = {}

        if (not cache):
            return super().build(*args, **kwargs)

        if (id is None):
            id = str(uuid.uuid4())

        result = None
        try:
            result = self._cache[id]
        except KeyError:
            result = super().build(*args, **kwargs)
            self._cache[id] = result

        return result
##### EndScript