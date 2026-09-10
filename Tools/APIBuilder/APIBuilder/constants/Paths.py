import os
import sys

UtilitiesPath = os.path.join("..", "Utilities", "src", "AGRemapUtils")

sys.path.insert(1, UtilitiesPath)
from Utils.constants.Paths import APIFolder, APIPyFolder, APICyFolder, APIPyBindFolder, APICoreFolder, APIExternFolder, APIPySrcFolder, APICySrcFolder, APIPyBindSrcFolder, APICoreSrcFolder, APICoreIncludeFolder, APICoreTestsFolder


PathToProject = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "..", ".."))
APIPath = os.path.join(PathToProject, APIFolder)

APIPyFolderPath = os.path.join(PathToProject, APIPyFolder)
APICyFolderPath = os.path.join(PathToProject, APICyFolder)
APIPyBindFolderPath = os.path.join(PathToProject, APIPyBindFolder)
APICoreFolderPath = os.path.join(PathToProject, APICoreFolder)

APIExternFolderPath = os.path.join(PathToProject, APIExternFolder)

# the folders containing all the source code for the API, over all of its layers
#   (pure python, Cython, pybind11 and the C++ core)
APISrcFolderPaths = [
    os.path.join(PathToProject, APIPySrcFolder),
    os.path.join(PathToProject, APICySrcFolder),
    os.path.join(PathToProject, APIPyBindSrcFolder),
    os.path.join(PathToProject, APICoreIncludeFolder),
    os.path.join(PathToProject, APICoreSrcFolder),
    os.path.join(PathToProject, APICoreTestsFolder)
]

RemoveAllFolder = "*"
BuildFolder = "cbuild"
APITopBuildFolderPath = os.path.join(PathToProject, BuildFolder)

PreBuildFolder = "cebuild"
APITopPreBuildFolderPath = os.path.join(PathToProject, PreBuildFolder)

PreInstallFolder = "cext"
APITopPreInstallFolderPath = os.path.join(PathToProject, PreInstallFolder)

XMLFolder = "xml"
APICoreXMLFolderPath = os.path.join(APICoreFolderPath, XMLFolder)