# A variant of the library's Lynae -> LynaePeppermint fix, the way an API user tweaks a shipped remap: start from the
# config the compiled fix is built from (read off Lynae's own builder row), change what you want, and register it in
# place of the default.
#
#   py -3 lynaeBangsOnHair.py <mod folder>
import os
import sys

APISrc = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "..", "Anime Game Remap (for all users)", "api", "src", "py")
sys.path.insert(0, APISrc)
import FixRaidenBoss2 as AGR    # noqa: E402

B = AGR.WWMIFixerConfig.Binding


# ==== Override how Lynae is fixed onto LynaePeppermint ======

lynae = AGR.ModTypes["Lynae"].value
config = AGR.WWMIFixerConfig.ofFactory(lynae.iniFixBuilder.factoryOf("Lynae", "LynaePeppermint"))
parser = lynae.iniParseBuilder.factoryOf("Lynae")

# Lynae's bangs (component 0) are drawn through LynaePeppermint's HAIR slot (1) instead of her bangs slot (0). Both slots
# draw on the same pixel shader, so the bangs keep their registers; the bangs slot, with nothing of the mod left in it,
# is hidden by the fix. Lynae's hair already goes through slot 1, so the bangs are a second section on that draw.
plan = config.plan
plan[0].slot = 1
plan[0].sameFileSection = True
config.plan = plan

# the hair slot's outline pass (its "other" pass in LynaePeppermint's DrawPasses) takes the diffuse of whichever part it draws
extraPassRegs = config.extraPassRegs
extraPassRegs[1]["other"] = [B("ps-t0", "hairDiffuse", 1), B("ps-t0", "bangsDiffuse", 0)]
config.extraPassRegs = extraPassRegs

AGR.CppStrategyOverrides.setParser("Lynae", parser)
AGR.CppStrategyOverrides.setFixer("Lynae", "LynaePeppermint", AGR.makeWWMIFixer(config))

# ========================================

# fix the mod
AGR.RemapService(path = sys.argv[1]).fix()
AGR.CppStrategyOverrides.clear()
