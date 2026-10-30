#ifndef AGRemapCore_DrawRead_H
#define AGRemapCore_DrawRead_H

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
#include <vector>

#include "AGRemapCore/model/strategies/iniParsers/parseSteps/BaseIniParseStep.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Reads what each mod object's root `sections`_ draw:

     * :cpp:func:`IniParseData::drawParts`: the parts holding a ``drawindexed`` (every part of the
       `section`_ counted, its ``if`` lines too)
     * :cpp:func:`IniParseData::drawRanges`: each ``drawindexed = <count>, <start>, <base>``
     * :cpp:func:`IniParseData::lateBindings`: the texture registers a `section`_ binds only after it has
       drawn -- read only when the mod does not replace textures by hash
       (:cpp:func:`IniParseData::replacesTexturesByHash`, so run :cpp:class:`TextureOverrideRead` first),
       because such a mod's art reaches those draws anyway
     * :cpp:func:`IniParseData::undrawnObjs`: a mod object whose `sections`_ draw nothing and skip the
       game's draw (``handling = skip``) -- the mod hides it. A mod object that draws nothing and leaves the
       game's geometry on screen is not hidden, and when EVERY read mod object would be undrawn none is,
       since that is a mod this reading does not understand

     :raw-html:`<br />`

     Whether a `section`_ draws follows ``run =``: a called `section`_ that draws counts, and so does a
     call this ``.ini`` file cannot resolve, unless its name holds one of #silentLibraries
     @endrst
     */
    class DrawRead: public BaseIniParseStep<> {
        public:
            /**
             * @param objs The mod objects to read. **Default**: every one
             * @param texRegPrefix
             @rst
             What the key of a texture register starts with, for #lateBindings. **Default**: ``ps-t``
             @endrst
             * @param silentLibraries
             @rst
             Text that names a library whose command lists draw nothing, in a ``run =`` this ``.ini`` file
             cannot resolve, ignoring case. **Default**: none
             @endrst
             */
            explicit DrawRead(ObjFilter objs = {}, std::string texRegPrefix = "ps-t", std::vector<std::string> silentLibraries = {});

            /**
             * @brief The mod objects to read
             */
            ObjFilter objs;

            /**
             * @brief What the key of a texture register starts with
             */
            std::string texRegPrefix;

            /**
             * @brief Text that names a library whose command lists draw nothing -- see the constructor
             */
            std::vector<std::string> silentLibraries;

            void read(Context& ctx, const SectionTargets& targets, IniParseData& data) override;
    };
}

#endif
