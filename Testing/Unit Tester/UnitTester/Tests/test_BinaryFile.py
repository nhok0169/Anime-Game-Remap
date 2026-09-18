import os
import sys
import tempfile

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class BinaryFileTest(BaseUnitTest):
    """
    Tests for :class:`BinaryFile`
    """

    def test_construction_doesNotAutoRead(self):
        # Matches this class's contract: __init__ only sets 'src' and an empty 'data' -- it never
        # calls read() itself (unlike BufFile, which does). Only 'src' should be populated right
        # after construction.
        binFile = FRB.BinaryFile(b"\x01\x02\x03")
        self.assertEqual(binFile.data, b"")
        self.assertEqual(binFile.src, b"\x01\x02\x03")

    def test_bytesSrc_readReturnsAndStoresSrc(self):
        binFile = FRB.BinaryFile(b"\x01\x02\x03")
        result = binFile.read()
        self.assertEqual(result, b"\x01\x02\x03")
        self.assertEqual(binFile.data, b"\x01\x02\x03")
        self.assertIsInstance(binFile.data, bytes)

    def test_fileSrc_readReadsFileContent(self):
        with tempfile.NamedTemporaryFile(delete = False) as f:
            f.write(b"\xAA\xBB\xCC")
            path = f.name

        try:
            binFile = FRB.BinaryFile(path)
            self.assertEqual(binFile.read(), b"\xAA\xBB\xCC")
            self.assertEqual(binFile.data, b"\xAA\xBB\xCC")
            self.assertEqual(binFile.src, path)
        finally:
            os.remove(path)

    def test_missingFile_readRaisesRuntimeError(self):
        binFile = FRB.BinaryFile("this_file_should_never_exist_98765.bin")
        with self.assertRaises(RuntimeError):
            binFile.read()

    def test_setSrc_readReturnsNewData(self):
        binFile = FRB.BinaryFile(b"\x01")
        binFile.src = b"\x02\x03"
        self.assertEqual(binFile.read(), b"\x02\x03")
        self.assertEqual(binFile.data, b"\x02\x03")
