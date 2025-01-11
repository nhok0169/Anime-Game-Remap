import uuid
import traceback
from typing import TypeVar, Optional, Set, DefaultDict, Callable, Tuple, Dict, Optional, Iterable
from collections import defaultdict

from .DFSData import DFSData

T = TypeVar("T")


class Algo():
    __DFSTimers = {}

    # topoSort(root, getNeighbours, vertices, reverse): Finds the topological ordering for a graph
    @classmethod
    def topoSort(cls, root: T, getNeighbours: Callable[[T], Iterable[T]], vertices: Optional[Set[T]] = None, reverse: bool = False) -> Tuple[bool, Dict[T, DFSData]]:
        error, dfsData = cls.dfs(root, getNeighbours, vertices = vertices)
        if (error is not None):
            return [error, dfsData]
        
        result = dict(sorted(dfsData.items(), key = lambda d: d[1].endTime, reverse = not reverse))
        return [error, result]


    # dfs(root, getNeighbours, vertices): Performs DFS search on a graph 
    @classmethod
    def dfs(cls, root: T, getNeighbours: Callable[[T], Iterable[T]], vertices: Optional[Set[T]] = None) -> Tuple[Optional[Exception], Dict[T, DFSData]]:
        # setup the result
        result = defaultdict(lambda: DFSData())
        if (vertices is None):
            vertices = set()

        for vertex in vertices:
            result[vertex] = DFSData()

        # set the timer
        timerId = str(uuid.uuid4())
        cls.__DFSTimers[timerId] = 0

        # Actual DFS Explore algorithm
        error = None
        result[root].visisted = True

        try:
            cls._dfsExplore(timerId, root, result, getNeighbours)
        except Exception as e:
            print(traceback.format_exc())
            error = e

        # remove the timer
        cls.__DFSTimers.pop(timerId)
        return [error, result]

    
    # _getDFSTime(id): Retrieves the current time from the timer used in DFS
    @classmethod
    def _getDFSTime(cls, id: str) -> int:
        result = cls.__DFSTimers[id]
        cls.__DFSTimers[id] += 1
        return result


    # _dfsExplore(id, vertex, dfsData, getNeighbours): The DFS Explore function
    @classmethod
    def _dfsExplore(cls, id: str, vertex: T, dfsData: DefaultDict[T, DFSData], getNeighbours: Callable[[T], Iterable[T]]):
        dfsData[vertex].startTime = cls._getDFSTime(id)
        neighbours = getNeighbours(vertex)

        for neighbour in neighbours:
            visited = dfsData[neighbour].visisted
            if (not visited):
                dfsData[neighbour].visisted = True
                cls._dfsExplore(id, neighbour, dfsData, getNeighbours)

        dfsData[vertex].endTime = cls._getDFSTime(id)

