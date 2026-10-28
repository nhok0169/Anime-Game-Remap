#ifndef AGRemapCore_ZZMIParser_H
#define AGRemapCore_ZZMIParser_H

// ##### Credits

// ===== Anime Game Remap (AG Remap) =====
// Authors: Albert Gold#2696, NK#1321
//
// if you used it to remap your mods pls give credit for "Albert Gold#2696" and "Nhok0169"
// Special Thanks:
//   nguen#2011 (for support)
//   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
//   HazrateGolabi#1364 (for being awesome, and improving the code)

// ##### EndCredits

#include "AGRemapCore/data/IniFixData/ZZMIFixer.h"
#include "AGRemapCore/model/strategies/iniParsers/IniParseBuilder.h"


namespace AGRemapCore {
    /**
     * @brief
     @rst
     Builds the parser of a ZZMI character's mods, from its pair's config -- one
     :cpp:class:`GIMISectionClassifier` per component, filtered to that component's own hash rows, and
     the drawn objects resolved from :cpp:member:`ZZMIFixerConfig::Component::slots`
     @endrst
     *
     * @param config The pair's config
     */
    IniParseBuilder::Factory makeZZMIParser(ZZMIFixerConfig config);
}

#endif
