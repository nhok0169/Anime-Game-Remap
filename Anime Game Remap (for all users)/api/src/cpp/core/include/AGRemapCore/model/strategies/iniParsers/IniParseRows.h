#ifndef AGRemapCore_IniParseRows_H
#define AGRemapCore_IniParseRows_H

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


namespace AGRemapCore {
    /**
     * @brief
     @rst
     The names the standard row sets of a parser are filed under (:cpp:func:`IniParseData::collected`) -- the
     :cpp:class:`GraphCollect`\s :cpp:func:`GIMIParser::addRowSteps` adds
     @endrst
     */
    struct IniParseRows {
        /**
         * @brief Every resource `section`_ the mod binds anywhere, by its ``filename``
         */
        static inline const std::string Resources = "IniParseRows.resources";

        /**
         * @brief Every `section`_'s ``filename``, ``type``, ``format`` and ``stride`` (and any key the parser adds), over the whole file
         */
        static inline const std::string Declared = "IniParseRows.declared";

        /**
         * @brief Every key the `sections`_ of the mod objects the parser chose reach, through ``run =``
         */
        static inline const std::string Objs = "IniParseRows.objs";

        /**
         * @brief
         @rst
         Every key of every `section`_ over the whole file. A section ``run =`` reaches is met again under its
         caller, so a reader of one section's own lines keeps each line once
         @endrst
         */
        static inline const std::string Sections = "IniParseRows.sections";
    };
}

#endif
