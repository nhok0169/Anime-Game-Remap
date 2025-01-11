import os

from ..constants.FileExts import FileExts


class PyPathTools():
    @classmethod
    def getInitPath(cls, folder: str):
        return os.path.join(folder, f"__init__{FileExts.Py.value}")
    
    @classmethod
    def getMainPath(cls, folder: str):
        return os.path.join(folder, f"__main__{FileExts.Py.value}")