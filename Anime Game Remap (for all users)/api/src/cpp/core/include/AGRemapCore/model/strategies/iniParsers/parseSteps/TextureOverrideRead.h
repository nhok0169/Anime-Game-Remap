#ifndef AGRemapCore_TextureOverrideRead_H
#define AGRemapCore_TextureOverrideRead_H

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
     Reads whether the mod replaces textures by their hash: a ``TextureOverride`` `section`_ with a
     ``hash`` and a ``this =`` puts its art on every draw that binds that texture, whatever the draw's own
     `section`_ binds -- see :cpp:func:`IniParseData::replacesTexturesByHash`
     @endrst
     */
    class TextureOverrideRead: public BaseIniParseStep<> {
        public:
            /**
             * @param draws
             @rst
             The mod objects whose root `sections`_ are draws rather than texture overrides, even with a
             ``hash`` and a ``this =`` :raw-html:`<br />` :raw-html:`<br />`

             **Default**: none
             @endrst
             */
            explicit TextureOverrideRead(ObjFilter draws = {});

            /**
             * @brief The mod objects whose root `sections`_ are draws -- see the constructor
             */
            ObjFilter draws;

            void read(Context& ctx, const SectionTargets& targets, IniParseData& data) override;
    };
}

#endif
