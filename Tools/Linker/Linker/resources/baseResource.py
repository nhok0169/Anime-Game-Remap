from typing import Optional


# BaseResource: Base class for retrieving a specific resource
class BaseResource():
    def __init__(self):
        self.savedResource: Optional[str] = None


    # _get(): Retrieves the resource from the raw source
    def _get(self) -> str:
        pass


    # get(varName): Retrieves the resource and stores it into 'varName'
    def get(self, varName: str):
        if (self.savedResource is None):
            self._get()

        return f"{varName} = {self.savedResource}"

        