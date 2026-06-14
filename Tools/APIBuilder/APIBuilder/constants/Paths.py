import os
import sys

UtilitiesPath = os.path.join("..", "Utilities", "src", "AGRemapUtils")

sys.path.insert(1, UtilitiesPath)
from Utils.constants.Paths import APIFolder, APIPyFolder, APICyFolder, APIPyBindFolder, APICoreFolder, APIExternFolder


PathToProject = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "..", ".."))
APIPath = os.path.join(PathToProject, APIFolder)

APIPyFolderPath = os.path.join(PathToProject, APIPyFolder)
APICyFolderPath = os.path.join(PathToProject, APICyFolder)
APIPyBindFolderPath = os.path.join(PathToProject, APIPyBindFolder)
APICoreFolderPath = os.path.join(PathToProject, APICoreFolder)

APIExternFolderPath = os.path.join(PathToProject, APIExternFolder)

RemoveAllFolder = "*"
BuildFolder = "cbuild"
APITopBuildFolderPath = os.path.join(PathToProject, BuildFolder)

PreBuildFolder = "cebuild"
APITopPreBuildFolderPath = os.path.join(PathToProject, PreBuildFolder)

PreInstallFolder = "cext"
APITopPreInstallFolderPath = os.path.join(PathToProject, PreInstallFolder)

XMLFolder = "xml"
APICoreXMLFolderPath = os.path.join(APICoreFolderPath, XMLFolder)