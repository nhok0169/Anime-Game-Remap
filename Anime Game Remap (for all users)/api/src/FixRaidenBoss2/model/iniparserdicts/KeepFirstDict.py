##### Credits

# ===== Anime Game Remap (AG Remap) =====
# Authors: NK#1321, Albert Gold#2696
#
# if you used it to remap your mods pls give credit for "Nhok0169" and "Albert Gold#2696"
# Special Thanks:
#   nguen#2011 (for support)
#   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
#   HazrateGolabi#1364 (for being awesome, and improving the code)

##### EndCredits


##### ExtImports
from collections import OrderedDict
##### EndExtImports


##### Script
# KeepFirstDict: Dictionary used to only keep the value of the first instance of a key
class KeepFirstDict(OrderedDict):
    def __setitem__(self, key, value):
        # All values updated into the dictionary of ConfigParser will first updated as a list of values, then
        #    the list of values will be turned into a string
        #
        # eg. the 'value' argument for the __setitem__ method in the case a key has 2 duplicates
        # >> value = ["val1"]           <----------- we only want this list
        # >> value = ["val1", "", "val2"]
        # >> value = ["val1", "", "val2", "", "val3"]
        # >> value = "val1\nval2\nval3"
        #
        # Note:
        #   For the case of duplicate keys, GIMI will only keep the value of the first valid instance of the key.
        #       Since checking for correct syntax and semantics is out of the scope of this program, we only get 
        #        the value of the first instance of the key
        if (key in self and isinstance(self[key], list) and isinstance(value, list)):
            return

        super().__setitem__(key, value)
##### EndScript