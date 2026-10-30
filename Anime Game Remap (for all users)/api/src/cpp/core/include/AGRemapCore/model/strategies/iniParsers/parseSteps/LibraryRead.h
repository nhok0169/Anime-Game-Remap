#ifndef AGRemapCore_LibraryRead_H
#define AGRemapCore_LibraryRead_H

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

#include <string>
#include <utility>
#include <vector>

#include "AGRemapCore/model/strategies/iniParsers/parseSteps/BaseIniParseStep.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Reads which libraries each mod object's root `sections`_ use: a library is used when any key or value
     of those `sections`_ names it -- see :cpp:func:`IniParseData::libraries`
     @endrst
     */
    class LibraryRead: public BaseIniParseStep<> {
        public:
            /**
             * @brief A library: its name in :cpp:func:`IniParseData::libraries`, and the text that names it in a line, ignoring case
             */
            using Library = std::pair<std::string, std::string>;

            /**
             * @param libraries The libraries to look for, in the order they are filed
             * @param objs The mod objects to read. **Default**: every one
             */
            explicit LibraryRead(std::vector<Library> libraries, ObjFilter objs = {});

            /**
             * @brief The libraries to look for
             */
            std::vector<Library> libraries;

            /**
             * @brief The mod objects to read
             */
            ObjFilter objs;

            void read(Context& ctx, const SectionTargets& targets, IniParseData& data) override;
    };
}

#endif
