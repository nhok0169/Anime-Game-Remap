import sys

from .Paths import UtilitiesPath

sys.path.insert(1, UtilitiesPath)
from Utils.enums.StrEnum import StrEnum


class Commands(StrEnum):
    RunSuite = "runSuite"
    ProduceOutputs = "produceOutputs"
    PrintOutputs = "printOutputs"
    clearOutputs = "clearOutputs"