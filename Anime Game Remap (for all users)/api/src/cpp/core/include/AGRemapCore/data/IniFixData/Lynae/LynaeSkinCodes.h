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


#ifndef LYNAE_SKIN_CODES_H
#define LYNAE_SKIN_CODES_H

#include "AGRemapCore/data/IniFixData/WWMIFixer.h"
#include "AGRemapCore/model/strategies/texEditors/TexEditor.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Swaps the skin code between Lynae's and LynaePeppermint's body material-code maps :raw-html:`<br />` :raw-html:`<br />`

     Both skins' upper and lower body shaders read an 8-bit code per texel at ``ps-t2``, the low four
     bits a material and the high four flags. Lynae marks bare skin with material 0 and LynaePeppermint
     with material 4, and each uses the other's number for something small (trims, logos). A code map
     carried across unchanged shades the body's skin as cloth: whiter, with grey-lavender shadows,
     beside a face that stays warm. Materials 0 and 4 are exchanged and every other bit is kept, so
     the same filter serves both directions :raw-html:`<br />` :raw-html:`<br />`

     A map stored in an sRGB format reaches the shader decoded, and the shader truncates the decoded
     value to its code; such a map is decoded the same way first, and every edit of a code map here
     writes the codes the shader would have read
     @endrst
     */
    TexEditor::Filter lynaeSkinCodeSwap();

    /**
     * @brief
     @rst
     Sets the red channel of a Lynae lower-body or jacket mask to 255 for LynaePeppermint :raw-html:`<br />` :raw-html:`<br />`

     Lynae's own lower-body and jacket masks are ``R = 255`` throughout and her shaders show no use of
     the channel there: a mod that paints ``R = 0`` over a whole outfit renders normally on her.
     LynaePeppermint's body shaders read ``R = 0`` as a dark material, which turned such an outfit
     dark navy. Writing 255 shows the target what Lynae showed; every other channel is kept
     @endrst
     */
    TexEditor::Filter lynaeMaskRedFill();

    /**
     * @brief
     @rst
     Bakes Lynae's lower-body shading bands into a lower-body diffuse for LynaePeppermint :raw-html:`<br />` :raw-html:`<br />`

     Lynae's lower-body shader reads the normal map's alpha as a shading band (around 102 a fabric
     renders about a third as bright as from 128 up); LynaePeppermint's does not, so a satin dress came
     out lighter and glossier. Every texel is scaled, in linear colour, by the measured ratio of the two
     renders at its band. Nothing is decided from the diffuse's own colour. The normal map is the mod's
     own ``lowerNormal`` when it binds one by default, otherwise Lynae's game map
     (``gameFileOfRole``); without either the texture is left as it is
     @endrst

     @param ctx The mod being fixed
     */
    TexEditor::Filter lynaeSheerBake(const WWMIFixerConfig::TexEditContext& ctx);

    /**
     * @brief
     @rst
     Moves Lynae's coat film code to LynaePeppermint's in a jacket code map :raw-html:`<br />` :raw-html:`<br />`

     Lynae's coat shader draws material 4 of her jacket's code map (the coat lining) as a holographic
     film; LynaePeppermint's coat shader draws its own film as code 49 (material 1, flags 3) and code 4 as
     plain diffuse. Every code with material 4 becomes 49
     @endrst
     */
    TexEditor::Filter lynaeFilmCode();

    /**
     * @brief
     @rst
     Tints the texels of a jacket diffuse under Lynae's coat film code (4) for LynaePeppermint's film
     :raw-html:`<br />` :raw-html:`<br />`

     Lynae's film renders the cyan lining texture as a pink-red rainbow; LynaePeppermint's film (see
     :cpp:func:`lynaeFilmCode`) tints its foil by the diffuse, so the lining came out cyan. Those texels
     are multiplied by the measured ratio of the two renders. The code map is chosen as for
     :cpp:func:`lynaeSheerBake`
     @endrst

     @param ctx The mod being fixed
     */
    TexEditor::Filter lynaeFilmBake(const WWMIFixerConfig::TexEditContext& ctx);

    /**
     * @brief
     @rst
     Moves Lynae's plain props materials to LynaePeppermint's plain hip-pouch material :raw-html:`<br />` :raw-html:`<br />`

     Lynae's props go onto LynaePeppermint's hip pouch, whose shader draws materials 0-4 (all that
     Lynae's props maps use, with or without flags) as the pouch's holographic foil: a red satin dress
     (code 0) came out dark red with blue-violet and green rims. Its material 15 is plain. Lynae marks
     her see-through FILM with flag ``0x10`` (her own sweater is 16), and such a code is kept, so the
     film lands on the pouch's foil; every other code's material becomes 15 and its flags are kept
     @endrst
     */
    TexEditor::Filter lynaePropsPlainCode();
}

#endif
