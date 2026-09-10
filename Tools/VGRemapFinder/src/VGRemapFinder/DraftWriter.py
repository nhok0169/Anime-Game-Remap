import os
from typing import Dict, List, Optional, Tuple

from .VGMatcher import VGMatch


class DraftWriter():
    """
    Writes proposed remaps into an Excel workbook in the format of the drafts under
    ``Data/RemapDrafts/`` (see that folder's ``README.md``): one sheet per remap direction, with
    the columns ``[from mod] | [to mod] | Uncertainty | Comments`` and duplicate ``to`` indices
    highlighted in yellow

    Parameters
    ----------
    path: :class:`str`
        Where to write the workbook. If the file exists, sheets for the same remap directions are
        replaced and every other sheet is kept

    Attributes
    ----------
    path: :class:`str`
        Where the workbook is written
    """

    MaxSheetTitleLen = 31
    DuplicateFill = "FFFF00"

    ColumnWidths = {"A": 14, "B": 16, "C": 12, "D": 110}

    # Every workbook this writer produces carries this sheet, so a proposal can be told apart from
    #   a hand-made draft (the benchmark refuses to score the tool against its own output)
    AboutSheet = "About"
    AboutText = "Proposed by Tools/VGRemapFinder. Remove this sheet once the proposal has been checked and is a draft in its own right."

    # A single proposed sheet added to a hand-made workbook is marked in this cell instead, so the
    #   benchmark can skip the sheet without disowning the workbook
    ProposedCell = "E1"
    ProposedMarker = "Proposed by Tools/VGRemapFinder"

    def __init__(self, path: str):
        self.path = path

    @classmethod
    def sheetTitle(cls, fromName: str, toName: str, version: Optional[str] = None) -> str:
        """
        Names a sheet the way the hand-made drafts do (``"V4.4 - Ganyu to GanyuTwilight"``),
        trimmed to Excel's 31 character limit

        Parameters
        ----------
        fromName: :class:`str`
            The name of the mod being remapped

        toName: :class:`str`
            The name of the remapped mod

        version: Optional[:class:`str`]
            The game version the remap targets

        Returns
        -------
        :class:`str`
            The sheet's title
        """

        body = f"{fromName} to {toName}"
        title = body if (version is None) else f"V{version} - {body}"

        if (len(title) > cls.MaxSheetTitleLen):
            title = title[:cls.MaxSheetTitleLen]
        return title

    def write(self, sheets: List[Tuple[str, str, Optional[str], List[VGMatch]]]):
        """
        Writes the workbook

        Parameters
        ----------
        sheets: List[Tuple[:class:`str`, :class:`str`, Optional[:class:`str`], List[:class:`VGMatch`]]]
            The sheets to write, each as ``(fromName, toName, version, matches)``
        """

        try:
            import openpyxl
            from openpyxl.formatting.rule import Rule
            from openpyxl.styles import Font, PatternFill
            from openpyxl.styles.differential import DifferentialStyle
        except ImportError as e:
            raise ImportError("Writing the draft workbook needs 'openpyxl' (pip install openpyxl)") from e

        if (os.path.isfile(self.path)):
            workbook = openpyxl.load_workbook(self.path)
        else:
            workbook = openpyxl.Workbook()
            workbook.remove(workbook.active)

        if (self.AboutSheet not in workbook.sheetnames):
            about = workbook.create_sheet(self.AboutSheet, 0)
            about.append([self.AboutText])
            about.column_dimensions["A"].width = 120

        for fromName, toName, version, matches in sheets:
            rows = []
            for match in matches:
                uncertainty: Optional[float] = round(match.uncertainty, 3)
                if (uncertainty == 0):
                    uncertainty = None
                rows.append((match.fromIndex, match.toIndex, uncertainty, match.comment()))
            self.addSheet(workbook, fromName, toName, version, rows)

        folder = os.path.dirname(os.path.abspath(self.path))
        os.makedirs(folder, exist_ok = True)
        workbook.save(self.path)

    @classmethod
    def addSheet(cls, workbook, fromName: str, toName: str, version: Optional[str], rows: List[Tuple[int, Optional[int], Optional[float], Optional[str]]],
                 marker: Optional[str] = None, title: Optional[str] = None):
        """
        Adds one remap-direction sheet, in the drafts' format, to an open ``openpyxl`` workbook,
        replacing a sheet of the same title

        Parameters
        ----------
        workbook: :class:`openpyxl.Workbook`
            The workbook to add to

        fromName: :class:`str`
            The name of the mod being remapped (column A's header)

        toName: :class:`str`
            The name of the remapped mod (column B's header)

        version: Optional[:class:`str`]
            The game version, for the sheet's title

        rows: List[Tuple[:class:`int`, Optional[:class:`int`], Optional[:class:`float`], Optional[:class:`str`]]]
            The rows, each ``(fromIndex, toIndex, uncertainty, comment)``

        marker: Optional[:class:`str`]
            Text to put in :attr:`ProposedCell`, marking the sheet as a proposal rather than a
            hand-made draft (``None`` for no mark)

        title: Optional[:class:`str`]
            The sheet's title. Defaults to :meth:`sheetTitle`

        Returns
        -------
        The sheet
        """

        from openpyxl.formatting.rule import Rule
        from openpyxl.styles import Font, PatternFill
        from openpyxl.styles.differential import DifferentialStyle

        if (title is None):
            title = cls.sheetTitle(fromName, toName, version)
        title = title[:cls.MaxSheetTitleLen]
        if (title in workbook.sheetnames):
            workbook.remove(workbook[title])

        sheet = workbook.create_sheet(title)
        sheet.append([fromName, toName, "Uncertainty", "Comments"])
        for cell in sheet[1]:
            cell.font = Font(bold = True)

        if (marker):
            sheet[cls.ProposedCell] = marker

        for row in rows:
            sheet.append(list(row))

        lastRow = len(rows) + 1
        if (lastRow >= 2):
            fill = PatternFill(start_color = cls.DuplicateFill, end_color = cls.DuplicateFill, fill_type = "solid")
            rule = Rule(type = "duplicateValues", dxf = DifferentialStyle(fill = fill))
            sheet.conditional_formatting.add(f"B2:B{lastRow}", rule)

        for column, width in cls.ColumnWidths.items():
            sheet.column_dimensions[column].width = width

        sheet.freeze_panes = "A2"
        return sheet

    @classmethod
    def isProposedSheet(cls, sheet) -> bool:
        """
        Whether a sheet was marked as a proposal by :meth:`addSheet`

        Parameters
        ----------
        sheet: :class:`openpyxl.worksheet.worksheet.Worksheet`
            The sheet (a read-only one works too)

        Returns
        -------
        :class:`bool`
            Whether the sheet carries the :attr:`ProposedMarker`
        """

        try:
            header = next(sheet.iter_rows(min_row = 1, max_row = 1, values_only = True), None)
        except Exception:
            return False
        if (header is None or len(header) < 5 or header[4] is None):
            return False
        return str(header[4]).startswith(cls.ProposedMarker)

    @classmethod
    def isProposal(cls, path: str) -> bool:
        """
        Whether a workbook was written by this tool (it carries the :attr:`AboutSheet`), as
        opposed to being a hand-made draft

        Parameters
        ----------
        path: :class:`str`
            The workbook to check

        Returns
        -------
        :class:`bool`
            Whether the workbook is one of this tool's proposals
        """

        try:
            import openpyxl
        except ImportError as e:
            raise ImportError("Reading a draft workbook needs 'openpyxl' (pip install openpyxl)") from e

        workbook = openpyxl.load_workbook(path, read_only = True)
        return cls.AboutSheet in workbook.sheetnames

    @classmethod
    def readSheet(cls, path: str, fromName: str, toName: str) -> Dict[int, Optional[int]]:
        """
        Reads one remap direction back out of a draft workbook, found by its header row

        Parameters
        ----------
        path: :class:`str`
            The workbook to read

        fromName: :class:`str`
            The name in the header of column A

        toName: :class:`str`
            The name in the header of column B

        Returns
        -------
        Dict[:class:`int`, Optional[:class:`int`]]
            The remap, source index to target index (``None`` where the draft left it blank)

        Raises
        ------
        :class:`KeyError`
            If no sheet has that header
        """

        try:
            import openpyxl
        except ImportError as e:
            raise ImportError("Reading a draft workbook needs 'openpyxl' (pip install openpyxl)") from e

        workbook = openpyxl.load_workbook(path, read_only = True, data_only = True)
        for sheet in workbook.worksheets:
            rows = sheet.iter_rows(min_row = 1, max_col = 2, values_only = True)
            header = next(rows, None)
            if (header is None or len(header) < 2):
                continue
            if (str(header[0]).strip() != fromName or str(header[1]).strip() != toName):
                continue

            result: Dict[int, Optional[int]] = {}
            for row in rows:
                if (row[0] is None or str(row[0]).strip() == ""):
                    continue
                try:
                    fromIndex = int(row[0])
                except (TypeError, ValueError):
                    continue

                toIndex: Optional[int] = None
                if (row[1] is not None and str(row[1]).strip() != ""):
                    try:
                        toIndex = int(row[1])
                    except (TypeError, ValueError):
                        toIndex = None
                result[fromIndex] = toIndex
            return result

        raise KeyError(f"'{path}' has no sheet whose header row is '{fromName}' | '{toName}'")
