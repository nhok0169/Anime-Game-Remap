#ifndef AGRemapCore_ResourceRead_H
#define AGRemapCore_ResourceRead_H

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

#include "AGRemapCore/model/strategies/iniParsers/parseSteps/BaseIniParseStep.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Reads the file every resource `section`_ names: each `section`_ with a ``filename`` goes into
     :cpp:func:`IniParseData::resourceFiles`, under its own name
     @endrst
     */
    class ResourceRead: public BaseIniParseStep<> {
        public:
            void read(Context& ctx, const SectionTargets& targets, IniParseData& data) override;
    };
}

#endif
