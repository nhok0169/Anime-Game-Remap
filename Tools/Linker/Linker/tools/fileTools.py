from typing import Callable, TypeVar, Any

TextIoWrapper = TypeVar('TextIoWrapper')


IniFileEncoding = "utf-8"
ReadEncodings = [IniFileEncoding, "latin1"]


# FileTools: Tools for dealing with files
class FileTools():

    # openFile(file, postProcessor, fileCode): Opens a file using different encodings
    @classmethod
    def openFile(cls, file, postProcessor: Callable[[TextIoWrapper], Any], fileCode: str) -> Any:
        error = None
        for encoding in ReadEncodings:
            try:
                with open(file, fileCode, encoding = encoding) as f:
                    return postProcessor(f)
            except UnicodeDecodeError as e:
                error = e

        if (error is not None):
            raise UnicodeDecodeError(f"Cannot decode the file using any of the following encodings: {ReadEncodings}")


    # readFile(file, postProcessor, fileCode): Reads a file using different encodings
    @classmethod
    def readFile(cls, file, postProcessor: Callable[[TextIoWrapper], Any], fileCode: str =  "r"):
        return cls.openFile(file, postProcessor = postProcessor, fileCode = fileCode)


    # writeFile(file, postProcessor, fileCode): Writes a file using different encodings
    @classmethod
    def writeFile(cls, file, postProcessor: Callable[[TextIoWrapper], Any], fileCode: str =  "w"):
        return cls.openFile(file, postProcessor = postProcessor, fileCode = fileCode)