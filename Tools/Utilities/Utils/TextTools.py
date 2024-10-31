from typing import List


class TextTools():

    # getTextLines(text): Retrieves the lines of text, split by the newline character, similar to python's readlines function
    @classmethod
    def getTextLines(cls, txt: str) -> List[str]:
        txtLines = txt.split("\n")

        if (txt):
            txtLinesLen = len(txtLines)
            for i in range(txtLinesLen):
                if (i < txtLinesLen - 1):
                    txtLines[i] += "\n"
        else:
            txtLines = []

        return txtLines