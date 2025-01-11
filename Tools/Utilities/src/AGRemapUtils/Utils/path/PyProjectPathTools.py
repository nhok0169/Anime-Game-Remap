import os


class ProjectPathTools():
    @classmethod
    def getPythonTomlConfigPath(cls, folder: str):
        return os.path.join(folder, "pyproject.toml")
    
    @classmethod
    def getLicensePath(cls, folder: str):
        return os.path.join(folder, "LICENSE")
    
    @classmethod
    def getReadMePath(cls, folder: str):
        return os.path.join(folder, "README.md")