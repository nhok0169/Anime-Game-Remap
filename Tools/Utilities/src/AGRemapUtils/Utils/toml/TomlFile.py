import re
from collections import OrderedDict
from typing import Dict, List, Optional

from ..FileTools import FileTools


SectionHeaderPattern = re.compile(r"^\[([^\[\]]+)\]\s*$")
KeyPattern = re.compile(r"^([A-Za-z0-9_\-.\"']+)\s*=")
QuotedPattern = re.compile(r"\"[^\"]*\"|'[^']*'")


# TomlFile: A .toml file, kept as text and divided into its sections and the keys within them
#
# note: this deliberately does not fully parse .toml -- it only needs to find, read and replace whole
#   'key = value' assignments, and keeping the original text means every value's formatting and every
#   comment around it survives a rewrite. Do not reach for it to interpret a value; for that, read the
#   file with a real .toml parser.
class TomlFile():
    def __init__(self, file: Optional[str] = None):
        self._file = file
        self.preamble: List[str] = []
        self.sections: "OrderedDict[str, List[str]]" = OrderedDict()


    @property
    def file(self) -> Optional[str]:
        return self._file


    # read(): Reads the .toml file
    def read(self) -> str:
        txt = FileTools.readFile(self._file, lambda filePtr: filePtr.read())
        self.parse(txt)
        return txt


    # write(txt): Writes back to the .toml file
    def write(self, txt: Optional[str] = None):
        if (txt is None):
            txt = self.toStr()

        FileTools.writeFile(self._file, lambda filePtr: filePtr.write(txt))


    # parse(txt): Divides the text of a .toml file into its sections
    def parse(self, txt: str):
        self.preamble = []
        self.sections = OrderedDict()

        currentSection = None
        for line in txt.split("\n"):
            headerMatch = SectionHeaderPattern.match(line)

            if (headerMatch is not None):
                currentSection = headerMatch.group(1).strip()
                self.sections[currentSection] = []
                continue

            if (currentSection is None):
                self.preamble.append(line)
            else:
                self.sections[currentSection].append(line)


    def toStr(self) -> str:
        result = list(self.preamble)
        for name, body in self.sections.items():
            result.append(f"[{name}]")
            result += body

        return "\n".join(result)


    # _getBracketDepth(line): Retrieves how much 'line' opens or closes a multi-line value
    #
    # note: quoted text and comments are dropped first, so a bracket inside a string does not count
    def _getBracketDepth(self, line: str) -> int:
        line = QuotedPattern.sub("", line)
        line = line.split("#")[0]
        return line.count("[") + line.count("{") - line.count("]") - line.count("}")


    def getSection(self, section: str) -> Optional[List[str]]:
        """
        The lines of a section's body, or ``None`` when the section is not in the file
        """

        try:
            return self.sections[section]
        except KeyError:
            return None


    def setSection(self, section: str, body: List[str]):
        self.sections[section] = list(body)


    def removeSection(self, section: str) -> bool:
        try:
            self.sections.pop(section)
            return True
        except KeyError:
            return False


    # _getKeySpans(section): Retrieves the [start, end) line span of each 'key = value' assignment
    #   within a section, keyed by its key
    def _getKeySpans(self, section: str) -> "OrderedDict[str, List[int]]":
        result = OrderedDict()
        body = self.getSection(section)
        if (body is None):
            return result

        currentKey = None
        depth = 0

        for i, line in enumerate(body):
            if (depth <= 0):
                depth = 0
                keyMatch = KeyPattern.match(line)
                if (keyMatch is not None):
                    currentKey = keyMatch.group(1).strip("\"'")
                    result[currentKey] = [i, i + 1]

            if (currentKey is not None):
                result[currentKey][1] = i + 1

            depth += self._getBracketDepth(line)

        # drop the blank lines that trail the last key of a section, since those separate the section
        #   from whatever comes after it rather than belonging to the key
        for key, span in result.items():
            while (span[1] > span[0] + 1 and not body[span[1] - 1].strip()):
                span[1] -= 1

        return result


    # getKeys(section): Retrieves each 'key = value' assignment within a section, keyed by its key
    def getKeys(self, section: str) -> "OrderedDict[str, List[str]]":
        body = self.getSection(section)
        if (body is None):
            return OrderedDict()

        result = OrderedDict()
        for key, span in self._getKeySpans(section).items():
            result[key] = body[span[0]: span[1]]

        return result


    def getKey(self, section: str, key: str) -> Optional[List[str]]:
        """
        The lines of a single ``key = value`` assignment, or ``None`` when it is not in the section
        """

        try:
            return self.getKeys(section)[key]
        except KeyError:
            return None


    # setKey(section, key, lines): Replaces a single 'key = value' assignment, adding it to the end of
    #   the section when the section does not have that key yet
    def setKey(self, section: str, key: str, lines: List[str]):
        body = self.getSection(section)
        if (body is None):
            self.setSection(section, list(lines))
            return

        span = self._getKeySpans(section).get(key)

        if (span is None):
            # place it after the section's last key, ahead of any trailing blank lines
            insertInd = len(body)
            while (insertInd > 0 and not body[insertInd - 1].strip()):
                insertInd -= 1

            self.sections[section] = body[:insertInd] + list(lines) + body[insertInd:]
            return

        self.sections[section] = body[:span[0]] + list(lines) + body[span[1]:]
