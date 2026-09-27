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

#ifndef AGRemapCore_YaoyaoBambooFixer_H
#define AGRemapCore_YaoyaoBambooFixer_H

#include "AGRemapCore/model/strategies/iniFixers/IniFixBuilder.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     YaoyaoBamboo's own fixers :raw-html:`<br />` :raw-html:`<br />`

     The SIXTH remap of a skin of SEVERAL components onto a target of ONE, and the inverse of
     :cpp:class:`YaoyaoFixer`'s direction. See :cpp:func:`makeGIMIMergeFixer`. Pair this with
     :cpp:class:`YaoyaoBambooParser`. Two template options came out of it -- a diffuse edit per target object
     (``GIMIMergeFixerConfig::diffuseEdits``) and a face copied only when it has to move
     (``GIMIMergeFixerConfig::faceOnlyWhenMoved``) -- and two merged-master fixes in shared code
     @endrst
     */
    class YaoyaoBambooFixer {
        public:

            YaoyaoBambooFixer() = delete;

            /**
             * @brief The fix onto Yaoyao -- what :cpp:func:`IniFixBuilderFuncs::yaoyaoBambooToYaoyao6_3` returns
             */
            static IniFixBuilder::Factory toYaoyao6_3();
    };
}

#endif
