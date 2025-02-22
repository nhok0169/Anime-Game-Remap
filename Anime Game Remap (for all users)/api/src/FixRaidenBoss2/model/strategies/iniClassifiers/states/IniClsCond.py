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
from typing import Optional, List, Union, Callable, Any
##### EndExtImports

##### LocalImports
from .IniClsActionArgs import IniClsActionArgs
from .IniClsAction import IniClsAction
##### EndLocalImports


##### Script
class IniClsCond(IniClsAction):
    """
    This class inherits from :class:`IniClsAction`

    An action for the :class:`IniClassifier` to handle branching conditions

    :raw-html:`<br />`

    .. container:: operations

        **Supported Operations:**

        .. describe:: x(classifier, line, keyword, prevStateId, currentStateId, isAccept, transtionMade)

            Calls :meth:`run` for the :class:`IniClsCond`, ``x``

    Parameters
    ----------
    conds: List[Callable[[:class:`IniClsActionArgs`], :class:`bool`]]
        A list of predicates to evaluate. :raw-html:`<br />` :raw-html:`<br />`

        .. tip::
            For a condition at position `i` in `conds` (`conds[i]`), you can assume the set of values that will be evaulated at this condition will be the values that
            do not satisfy the previous conditions (does not satisfy any condition at position `j`, where `j < i`) :raw-html:`<br />` :raw-html:`<br />`

            Simply, the standard `if ... else ...` structure you expect from other programming languages

    actions: List[Union[:class:`IniClsAction`, Callable[[:class:`IniClsActionArgs`], Any]]]
        The actions to run after its corresponding predicate at 'conds' is evaluated to be true

    default:  Union[Optional[:class:`IniClsAction`], Callable[[:class:`IniClsActionArgs`], Any]]
        The default action to run if none of the predicates are satisfied

    Attributes
    ----------
    conds: List[Callable[[:class:`IniClsActionArgs`], :class:`bool`]]
        A list of predicates to evaluate.

    actions: List[Union[:class:`IniClsAction`, Callable[[:class:`IniClsActionArgs`], Any]]]
        The actions to run after its corresponding predicate at 'conds' is evaluated to be true

    default:  Union[Optional[:class:`IniClsAction`], Callable[[:class:`IniClsActionArgs`], Any]]
        The default action to run if none of the predicates are satisfied
    """

    def __init__(self, conds: List[Callable[[IniClsActionArgs], bool]],
                 actions: List[Union[IniClsAction, Callable[[IniClsActionArgs], Any]]],
                 default: Union[Optional[IniClsAction], Callable[[IniClsActionArgs], Any]]):
        self.conds = conds
        self.actions = actions
        self.default = (lambda actionArgs: None) if (default is None) else default

    def __call__(self, args):
        minLen = min(len(self.conds), len(self.actions))

        for i in range(minLen):
            if (self.conds[i](args)):
                self.actions[i](args)
                return
        
        self.default(args)
##### EndScript