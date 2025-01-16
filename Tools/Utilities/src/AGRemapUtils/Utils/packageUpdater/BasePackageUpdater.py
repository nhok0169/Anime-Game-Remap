# BasePackageUpdater: Base class for updating the latest version of some software package
class BasePackageUpdater():
    def __init__(self, packageName: str):
        self.packageName = packageName

    # update(): Updates the package
    def update(self):
        pass