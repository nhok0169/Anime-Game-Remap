#ifndef AGRemapCore_ZZMIModObjs_H
#define AGRemapCore_ZZMIModObjs_H

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

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include "AGRemapCore/data/IniFixData/ZZMIFixer.h"


namespace AGRemapCore {
    /**
     * @brief
     @rst
     The mod objects a ZZMI character's parser sorts a mod into and its fixers edit, named the same
     on both sides -- :cpp:func:`makeZZMIParser` and :cpp:func:`makeZZMIFixer` share them
     @endrst
     */
    namespace ZZMIModObjs {
        /**
         * @brief The hash types that name a component's graphs, and the object each sorts into
         */
        inline const std::vector<std::pair<std::string, std::string>> HashKinds = {
            {"ib", "ib"}, {"blend_vb", "blend"}, {"texcoord_vb", "texcoord"}, {"draw_vb", "vertexlimit"},
            {"position_vb", "position"}};

        /**
         * @brief
         @rst
         A component's mod object. The ZZZ component goes into the OBJECT name and the mod object's
         own component stays ``""``: a non-empty component means "the same object drawn more than
         once" to the graph groups
         @endrst
         *
         * @param comp The component, by its source name
         * @param name The object within it
         */
        inline std::pair<std::string, std::string> objOf(const std::string& comp, const std::string& name) {
            return {"", comp + "_" + name};
        }

        /**
         * @brief
         @rst
         Every object of one component: its draw slots, one per :cpp:var:`HashKinds` entry, and one
         ``tex_<role>`` per texture role
         @endrst
         *
         * @param c The component
         */
        inline std::vector<std::string> compObjs(const ZZMIFixerConfig::Component& c) {
            std::vector<std::string> objs;
            for (const ZZMIFixerConfig::Slot& slot : c.slots) {
                objs.push_back(slot.name);
            }
            for (const auto& kind : HashKinds) {
                objs.push_back(kind.second);
            }
            for (const ZZMIFixerConfig::Texture& tex : c.textures) {
                if (std::find(objs.begin(), objs.end(), "tex_" + tex.role) == objs.end()) {
                    objs.push_back("tex_" + tex.role);
                }
            }
            return objs;
        }
    }
}

#endif
