from .Error import Error


# InvalidCommand: Exception when an invalid command is entered
class InvalidCommand(Error):
    def __init__(self, commandName: str):
        super().__init__(f"Unable to find command by the name '{commandName}'")