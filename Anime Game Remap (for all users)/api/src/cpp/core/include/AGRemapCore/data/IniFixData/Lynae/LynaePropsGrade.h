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


#ifndef LYNAE_PROPS_GRADE_H
#define LYNAE_PROPS_GRADE_H

#include "AGRemapCore/data/IniFixData/WWMIFixer.h"
#include "AGRemapCore/model/strategies/texEditors/TexEditor.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Strengthens a mod's recolour of Lynae's props diffuse for LynaePeppermint's coat shader :raw-html:`<br />` :raw-html:`<br />`

     Lynae draws her props (component 6) through a glass layer of four passes, two of which tint the
     surface by the diffuse; LynaePeppermint's coat slot, where the remap sends them, draws two passes
     that mute it under the coat's holographic sheen. Lynae's own texture renders alike on both, so the
     difference is in how far a mod's texture STRAYS from it: every texel of component 6's UV island
     (rasterised from the mod's own mesh) is moved to ``own + strength * (mod - own)``, ``own`` being
     Lynae's game texture of the role (:cpp:member:`WWMIFixerConfig::TexEditContext::gameFileOfRole`).
     A texel the mod left as hers is unchanged, and so is every texel outside the island. With no
     game texture or no island, the texture is left as it is
     @endrst

     @param ctx The mod being fixed
     @param role The role the edit is registered for, whose game texture is the reference
     @param strength How much further the mod's own difference is pushed; ``1`` changes nothing
     */
    TexEditor::Filter lynaePropsGrade(const WWMIFixerConfig::TexEditContext& ctx, const std::string& role, double strength);
}

#endif
