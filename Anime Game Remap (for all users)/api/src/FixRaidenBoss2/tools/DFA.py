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
from functools import lru_cache 
from typing import Hashable, Dict, Set, Tuple, Type
##### EndExtImports

##### LocalImports
from .Node import Node
##### EndLocalImports


##### Script
class DFA():
    """
    Class for a `DFA (Deterministic Finite Automaton)`_

    Attributes
    ----------
    _states: Dict[Hashable, :class:`Node`]
        The states in the `DFA`_ :raw-html:`<br />` :raw-html:`<br />`

        The keys are the ids of the states and values are the nodes for the states

    _neighbours: Dict[Hashable, Dict[Hashable, Hashable]]
        The out-neighbour nodes of a state :raw-html:`<br />` :raw-html:`<br />`

        * The outer keys are the ids of the states
        * The inner keys are the transition from one state to another
        * The inner values are the ids of the neighbour states

    _accept: Set[Hashable]
        The ids of the states that are considered as accepting states

    _startId: Hashable
        The id for the start state

    _currentStateId: Hashable
        The id for the current state
    """

    def __init__(self, nodeCls: Type[Node] = Node):
        self._states: Dict[Hashable, Node] = {}
        self._neighbours: Dict[Hashable, Dict[Hashable, Hashable]] = {}
        self._accept: Set[Hashable] = set()

        self._nodeCls = nodeCls

        self._startId: Hashable = []
        self._currentStateId: Hashable = []

    @property
    def startId(self) -> Hashable:
        """
        The id to the start state

        .. warning::
            The setter may raise a :class:`KeyError` if the newly given start id does not correspond
            to any state within the `DFA`_

        :getter: Retrieves the start id
        :setter: Sets the new start id
        :type: Hashable
        """
        
        return self._startId
    
    @startId.setter
    def startId(self, newStartId: Hashable):
        if (newStartId not in self._states):
            raise KeyError(f"The id, '{newStartId}' cannot be set as the new start state since the id does not correspond to a valid state in the DFA")

        self._startId = newStartId

    @property
    def currentStateId(self) -> Hashable:
        """
        The id of the state the `DFA`_ is currently at

        .. warning::
            The setter may raise a :class:`KeyError` if the newly current id does not correspond
            to any state within the `DFA`_

        :getter: Retrieves the id of the current state
        :setter: Sets the new id of the current state the `DFA`_ is on
        :type: Hashable
        """

        return self._currentStateId
    
    @currentStateId.setter
    def currentStateId(self, newCurrentId: Hashable):
        if (newCurrentId not in self._states):
            raise KeyError(f"The id, '{newCurrentId}' cannot be set as the new current state since the id does not correspond to a valid state in the DFA")

        self._currentStateId = newCurrentId

    def clear(self):
        """
        Clears the `DFA`_
        """

        self._transition.cache_clear()
        self._states = {}
        self._neighbours = {}
        self._accept = set()

        self._startId = []
        self._currentStateId = []

    def _constructNode(self, id: Hashable, *args, **kwargs) -> Node:
        """
        Constructs a node for the `DFA`_

        Parameters
        ----------
        id: Hashable
            The id for the node

        *args:
            Any extra arguments used to construct the node

        **kwargs:
            Any extra keyword arguments used to construct the node

        Returns
        -------
        :class:`Node`
            The contructed node
        """

        return self._nodeCls(id, *args, **kwargs)

    def addState(self, id: Hashable, isAccept: bool = False, isStart: bool = False) -> Tuple[Node, bool]:
        """
        Add a new state to the `DFA`

        Parameters
        ----------
        id: Hashable
            The id for the state

        isAccept: :class:`bool`
            Whether the state is an accepting state

        isStart: :class:`bool`
            Whether to set the state as the new starting state

            .. warning::
                A `DFA`_ can only have 1 start state

            .. warning::
                If the `DFA`_ is empty and you add a new state, will set this state as the start state

        Returns
        -------
        Tuple[:class:`Node`, :class:`bool`]
            Retrieves the data about the newly added state, including:

            #. The corresponding state
            #. Whether the state was newly added
        """

        isEmpty = not bool(self._states)
        if (isEmpty):
            isStart = True

        state = self._states.get(id)
        isNewlyAdded = state is None

        if (isNewlyAdded):
            state = self._constructNode(id)
            self._states[id] = state

        if (not isAccept and id in self._accept):
            self._accept.remove(id)
        elif (isAccept):
            self._accept.add(id)
        
        if (isStart):
            self._startId = id

        if (isEmpty):
            self._currentStateId = id

        self._transition.cache_clear()
        return (state, isNewlyAdded)
    
    def addTransition(self, srcId: Hashable, keyword: Hashable, destId: Hashable):
        """
        Adds a transition to the `DFA`_

        Parameters
        ----------
        srcId: Hashable
            The id of the source state for the transition

            .. caution::
                The id to the source state must refer to an existing state to the `DFA`_

        keyword: Hashable
            The keyword that will trigger a transition from the source state to the destination state

            .. warning::
                If the source state already has such a transition, then will overwrite the destination state for this transition

        destId: Hashable
            The id of the destionation state for the transition

            .. note::
                The id of this state does not need to exist yet in the `DFA`_ . If the id of this state does not exist, then
                will create a new state in the `DFA`_
        """

        if (srcId not in self._states):
            raise KeyError(f"The id, '{srcId}' cannot be set as the source state of a new transition since the id does not correspond to a valid state in the DFA")
        
        neighbours = self._neighbours.get(srcId)
        if (neighbours is None):
            neighbours = {}
            self._neighbours[srcId] = neighbours

        destState = self._states.get(destId)
        if (destState is None):
            destState, _ = self.addState(destId, isAccept = False, isStart = False)

        neighbours[keyword] = destId
        self._transition.cache_clear()

    def reset(self):
        """
        Resets the `DFA`_ to return back to its starting state
        """

        self._currentStateId = self._startId

    @lru_cache(maxsize = 256)
    def _transition(self, currentStateId: Hashable, keyword: Hashable):
        resultStateId = currentStateId
        isAccept = currentStateId in self._accept
        transitionTaken = False

        neighbours = self._neighbours.get(currentStateId)
        if (neighbours is None):
            return (resultStateId, isAccept, transitionTaken)
        
        resultStateId = neighbours.get(keyword, [])
        if (isinstance(resultStateId, list)):
            return (currentStateId, isAccept, transitionTaken)
        
        self._currentStateId = resultStateId
        isAccept = resultStateId in self._accept
        transitionTaken = True
        
        return (resultStateId, isAccept, transitionTaken)

    def transition(self, keyword: Hashable) -> Tuple[Hashable, bool, bool]:
        """
        Transitions to a new state

        Parameters
        ----------
        keyword: Hashable
            The keyword to trigger the transition to the new state

        Returns
        -------
        Tuple[Hashable, :class:`bool`, :class:`bool`]
            Resultant data regarding the new transitioned state, which includes:

            #. The id of the new state
            #. Whether the new state is an accepting state
            #. Whether a transition was taken 
        """

        result = self._transition(self._currentStateId, keyword)
        self._currentStateId = result[0]
        return result
##### EndScript