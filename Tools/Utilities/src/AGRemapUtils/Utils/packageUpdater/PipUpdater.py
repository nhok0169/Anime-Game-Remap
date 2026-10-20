from typing import Optional
import subprocess
import sys

from .BasePackageUpdater import BasePackageUpdater


# PipUpdater: Class to update the latest pip package
class PipUpdater(BasePackageUpdater):
    def __init__(self, packageName: str, installName: Optional[str] = None):
        super().__init__(packageName)
        self.installName = self.packageName if (installName is None) else installName

    def update(self):
        print(f"Upgrading pip package by the name: {self.packageName}")
        subprocess.run([sys.executable, "-m", "pip", "install", "-U", self.installName])