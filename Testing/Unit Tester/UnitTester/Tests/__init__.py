from .test_DictTools import DictToolsTest
from .test_FileService import FileServiceTest
from .test_GIMIFixer import GIMIFixerTest
from .test_GIMIObjMergeFixer import GIMIObjMergeFixerTest
from .test_GIMIObjRegEditFixer import GIMIObjRegEditFixerTest
from .test_GIMIObjSplitFixer import GIMIObjSplitFixerTest
from .test_GIMIParser import GIMIParserTest
from .test_GIMIObjParser import GIMIObjParserTest
from .test_IniRemover import IniRemoverTest
from .test_Logger import LoggerTest
from .test_IniResourceModel import IniResourceModelTest
from .test_IfTemplate import IfTemplateTest
from .test_IniFile import IniFileTest
from .test_Mod import ModTest
from .test_ModTypes import ModTypesTest
from .test_ModType import ModTypeTest
from .test_MultiModFixer import MultiModFixersTest
from .test_RemapService import RemapServiceTest
#from .test_ModDictAssets import ModDictAssetsTest

__all__ = ["DictToolsTest", "FileServiceTest", "LoggerTest", "IniResourceModelTest", "IfTemplateTest", "IniFileTest"]
__all__ += ["ModTest", "ModTypesTest", "ModTypeTest", "RemapServiceTest", "GIMIFixerTest", "GIMIParserTest", "IniRemoverTest"]
__all__ += ["GIMIObjParserTest", "GIMIObjMergeFixerTest", "GIMIObjSplitFixerTest", "MultiModFixersTest", "GIMIObjRegEditFixerTest"]