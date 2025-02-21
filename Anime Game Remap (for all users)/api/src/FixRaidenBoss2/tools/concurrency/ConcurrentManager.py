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
from typing import List, Optional, Generic, Type
##### EndExtImports

##### LocalImports
from ...constants.GenericTypes import T
##### EndLocalImports


##### Script
class ConcurrentManager(Generic[T]):
    """
    Base class to manage running many executions

    Paramaters
    ----------
    executionCls: Type[T]
        The class for building the executions

    jobNo: Optional[:class:`int`]
        The number of executions to run at once :raw-html:`<br />` :raw-html:`<br />`

        If this argument is ``None``, will run all the executions at once :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    Attributes
    ----------
    executionCls: Type[T]
        The class for building the executions

    execution: List[T]
        The executions to run

    jobNo: Optional[:class:`int`]
        The number of threads to run at once
    """

    def __init__(self, executionCls: Type[T], jobNo: Optional[int] = None):
        self.executionCls = executionCls
        self.execs: List[T] = []
        self.jobNo = jobNo

    def clear(self):
        """
        Clears all the executions
        """

        self.execs.clear()

    def add(self, *args, **kwargs):
        """
        Adds an execution

        Parameters
        ----------
        *args:
            The arguments to provide into the class at :attr:`executionCls`

        **kwargs:
            The keyword arguments to provide into :attr:`executionCls`
        """

        self.execs.append(self.executionCls(*args, **kwargs))

    def waitAll(self):
        """
        Runs all the executions at once and waits until all the executions have finished running
        """

        if (self.jobNo is None):
            for exec in self.execs:
                exec.start()

            for exec in self.execs:
                exec.join()
            
            return
        
        numOfCycles, remainingJobs = divmod(len(self.execs), self.jobNo)

        for i in range(numOfCycles):
            for j in range(self.jobNo):
                ind = self.jobNo * i + j
                self.execs[ind].start()

            for j in range(self.jobNo):
                ind = self.jobNo * i + j
                self.execs[ind].join()

        for i in range(remainingJobs):
            ind = self.jobNo * numOfCycles + i
            self.execs[ind].start()

        for i in range(remainingJobs):
            ind = self.jobNo * numOfCycles + i
            self.execs[ind].join()
##### EndScript
