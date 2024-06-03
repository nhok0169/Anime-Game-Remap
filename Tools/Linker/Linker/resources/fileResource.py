from .baseResource import BaseResource
from ..tools import FileTools


# FileResource: Resource that comes from a text-based file
class FileResource(BaseResource):
    def __init__(self, file: str):
        super().__init__()
        self.file = file


    # _get(): Retrieves the resource from a text-based file
    def _get(self):
        result = FileTools.readFile(self.file, lambda filePtr: filePtr.read())
        self.savedResource = result.strip()
        return self.savedResource
