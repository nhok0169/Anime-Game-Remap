from enum import Enum


class Commands(Enum):
    RunSuite = "runSuite"
    ProduceOutputs = "produceOutputs"
    PrintOutputs = "printOutputs"
    clearOutputs = "clearOutputs"
    
    # get(commandName): Retrieves the corresponding command
    @classmethod
    def get(cls, commandName: str):
        for command in cls:
            if (command.value == commandName):
                return command
        return None