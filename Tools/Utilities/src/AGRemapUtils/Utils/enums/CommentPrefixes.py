from .StrEnum import StrEnum


# CommentPrefixes: enum for the prefixes used to write a single line comment in a particular language
class CommentPrefixes(StrEnum):
    Py = "#"
    Cpp = "//"
