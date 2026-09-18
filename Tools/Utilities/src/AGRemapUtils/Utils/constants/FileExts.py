from enum import Enum

from ..enums.CommentPrefixes import CommentPrefixes


# FileExts: Enum for different file extensions
class FileExts(Enum):
    Py = ".py"
    Pyx = ".pyx"
    Pxd = ".pxd"
    H = ".h"
    Hpp = ".hpp"
    Cpp = ".cpp"
    Tpp = ".tpp"


# SrcFileCommentPrefixes: The prefix used to write a single line comment within the source files of each supported file extension
SrcFileCommentPrefixes = {
    FileExts.Py.value: CommentPrefixes.Py.value,
    FileExts.Pyx.value: CommentPrefixes.Py.value,
    FileExts.Pxd.value: CommentPrefixes.Py.value,
    FileExts.H.value: CommentPrefixes.Cpp.value,
    FileExts.Hpp.value: CommentPrefixes.Cpp.value,
    FileExts.Cpp.value: CommentPrefixes.Cpp.value,
    FileExts.Tpp.value: CommentPrefixes.Cpp.value
}
