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
from multiprocessing import Process
from typing import Optional
##### EndExtImports

##### LocalImports
from .ConcurrentManager import ConcurrentManager
##### EndLocalImports



##### Script
class ProcessManager(ConcurrentManager[Process]):
    """
    Class to manage running many processes

    .. danger::
        This class is susceptible to keyboard interrupts. However, this vulnerability seem to
        be rooted to a bug in Python itself:
        https://stackoverflow.com/questions/1408356/keyboard-interrupts-with-pythons-multiprocessing-pool

    Paramaters
    ----------
    jobNo: Optional[:class:`int`]
        The number of processes to run at once :raw-html:`<br />` :raw-html:`<br />`

        If this argument is ``None``, will run all the processes at once :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``
    """

    def __init__(self, jobNo: Optional[int] = None):
        super().__init__(Process, jobNo = jobNo)

    def waitAll(self):
        """
        Runs all the processes at once and waits until all the processes have finished running
        """

        if (self.jobNo is None):
            for exec in self.execs:
                exec.start()

            try:
                for exec in self.execs:
                    exec.join()
            except KeyboardInterrupt:
                for exec in self.execs:
                    exec.terminate()
            
            return
        
        numOfCycles, remainingJobs = divmod(len(self.execs), self.jobNo)

        for i in range(numOfCycles):
            try:
                for j in range(self.jobNo):
                    ind = self.jobNo * i + j
                    self.execs[ind].daemon = True
                    self.execs[ind].start()

                for j in range(self.jobNo):
                    ind = self.jobNo * i + j
                    self.execs[ind].join()
            except KeyboardInterrupt:
                for exec in self.execs:
                    exec.terminate()
                    exec.join()

                return

        try:
            for i in range(remainingJobs):
                ind = self.jobNo * numOfCycles + i
                self.execs[ind].daemon = True
                self.execs[ind].start()

            for i in range(remainingJobs):
                ind = self.jobNo * numOfCycles + i
                self.execs[ind].join()
        except KeyboardInterrupt:
            for exec in self.execs:
                exec.terminate()
                exec.join()
##### EndScript
