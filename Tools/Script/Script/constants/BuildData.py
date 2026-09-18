##### Credits

# ===== Anime Game Remap (AG Remap) =====
# Authors: Albert Gold#2696, NK#1321
#
# if you used it to remap your mods pls give credit for "Albert Gold#2696" and "Nhok0169"
# Special Thanks:
#   nguen#2011 (for support)
#   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
#   HazrateGolabi#1364 (for being awesome, and improving the code)

##### EndCredits


##### Script
# note: every value below is filled in by the ScriptBuilder when this script is compiled. Running
#   this module straight out of the repo leaves the placeholders as they are, which is why
#   Tools/Script/main.py hands in its own API reference instead of relying on them.

# note: every placeholder below is a STRING on purpose. This module still has to be valid python
#   before anything is filled in -- the ScriptBuilder imports this package to work out the order to
#   write it out in, so a placeholder that is not itself parseable stops the script being built at
#   all.

# EnvName: The environment this script was built for. See BuildEnv
EnvName = "{{Build Env}}"

# ApiRelPath: Where the API's source folder sits relative to this script
#
# note: separated by '/' rather than by os.sep, so that a script compiled on one OS still finds the
#   API on another. Only a 'dev' build uses this.
ApiRelPath = "{{API Rel Path}}"

# ApiPackage: The API, both as a module to import and as an installation name on pypi
ApiPackage = "{{API Package}}"
##### EndScript