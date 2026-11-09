# Lynae -> LynaePeppermint (Wuthering Waves) from Python through the WWMI templates: the library's own configs for the
# pair (read off Lynae's own builder rows: IniParseBuilder.factoryOf / IniFixBuilder.factoryOf), handed to
# makeWWMIParser / makeWWMIFixer and run by RemapService.
# lynaeBangsOnHair.py beside it shows a variant of the same config.
#
#   py -3 lynaePeppermintFix.py <mod folder> [--download disabled]
import os
import sys

APISrc = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "..", "Anime Game Remap (for all users)", "api", "src", "py")
sys.path.insert(0, APISrc)
import FixRaidenBoss2 as AGR    # noqa: E402

lynae = AGR.ModTypes["Lynae"].value
parseConfig = AGR.WWMIParserConfig.ofFactory(lynae.iniParseBuilder.factoryOf("Lynae"))
fixConfig = AGR.WWMIFixerConfig.ofFactory(lynae.iniFixBuilder.factoryOf("Lynae", "LynaePeppermint"))
AGR.CppStrategyOverrides.setParser("Lynae", AGR.makeWWMIParser(parseConfig))
AGR.CppStrategyOverrides.setFixer("Lynae", "LynaePeppermint", AGR.makeWWMIFixer(fixConfig))

kwargs = {"downloadMode": sys.argv[sys.argv.index("--download") + 1]} if "--download" in sys.argv else {}
service = AGR.RemapService(path = sys.argv[1], **kwargs)
service.fix()
for path, error in service.stats.ini.skipped.items():
    print("SKIPPED", path, error)
AGR.CppStrategyOverrides.clear()
