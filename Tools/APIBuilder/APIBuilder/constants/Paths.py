import os
import sys

UtilitiesPath = os.path.join("..", "Utilities", "src", "AGRemapUtils")

sys.path.insert(1, UtilitiesPath)
from Utils.constants.Paths import APIFolder, APIPyFolder, APICyFolder, APIPyBindFolder, APICoreFolder


PathToProject = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "..", ".."))
APIPath = os.path.join(PathToProject, APIFolder)

APIPyFolderPath = os.path.join(PathToProject, APIPyFolder)
APICyFolderPath = os.path.join(PathToProject, APICyFolder)
APIPyBindFolderPath = os.path.join(PathToProject, APIPyBindFolder)
APICoreFolderPath = os.path.join(PathToProject, APICoreFolder)

BuildFolder = "build"
APITopBuildFolderPath = os.path.join(APIPath, BuildFolder)
APICyBuildFolderPath = os.path.join(APICyFolderPath, BuildFolder)
APIPyBindBuildFolderPath = os.path.join(APIPyBindFolderPath, BuildFolder)
APICoreBuildFolderPath = os.path.join(APICoreFolderPath, BuildFolder)

XMLFolder = "xml"
APICoreXMLFolderPath = os.path.join(APICoreFolderPath, XMLFolder)