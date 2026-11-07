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

#include <array>
#include <cstdint>
#include <vector>


namespace AGRemapCore {

    /**
     * @brief
     @rst
     The table that swaps the skin code between Lynae's and LynaePeppermint's body material-code maps, for
     :cpp:func:`ChannelMapFilter::codeMap` :raw-html:`<br />` :raw-html:`<br />`

     Both skins' upper and lower body shaders read an 8-bit code per texel at ``ps-t2``, the low four
     bits a material and the high four flags. Lynae marks bare skin with material 0 and LynaePeppermint
     with material 4, and each uses the other's number for something small (trims, logos). A code map
     carried across unchanged shades the body's skin as cloth: whiter, with grey-lavender shadows,
     beside a face that stays warm. Materials 0 and 4 are exchanged and every other bit is kept, so
     the same table serves both directions
     @endrst
     */
    std::vector<std::uint8_t> lynaeSkinCodeTable();

    /**
     * @brief
     @rst
     The table that moves Lynae's coat film code to LynaePeppermint's in a jacket code map, for
     :cpp:func:`ChannelMapFilter::codeMap` :raw-html:`<br />` :raw-html:`<br />`

     Lynae's coat shader draws material 4 of her jacket's code map (the coat lining) as a holographic
     film; LynaePeppermint's coat shader draws its own film as code 49 (material 1, flags 3) and code 4 as
     plain diffuse. Every code with material 4 becomes 49
     @endrst
     */
    std::vector<std::uint8_t> lynaeFilmCodeTable();

    /**
     * @brief
     @rst
     The table that moves Lynae's plain props materials to LynaePeppermint's plain hip-pouch material, for
     :cpp:func:`ChannelMapFilter::codeMap` :raw-html:`<br />` :raw-html:`<br />`

     Lynae's props go onto LynaePeppermint's hip pouch, whose shader draws materials 0-4 (all that
     Lynae's props maps use, with or without flags) as the pouch's holographic foil: a red satin dress
     (code 0) came out dark red with blue-violet and green rims. Its material 15 is plain. Lynae marks
     her see-through FILM with flag ``0x10`` (her own sweater is 16), and such a code is kept, so the
     film lands on the pouch's foil; every other code's material becomes 15 and its flags are kept
     @endrst
     */
    std::vector<std::uint8_t> lynaePropsPlainCodeTable();

    /**
     * @brief
     @rst
     Lynae's lower-body shading bands, as the factors of a :cpp:class:`GuidedScaleFilter` guided by the normal
     map's alpha, in linear colour :raw-html:`<br />` :raw-html:`<br />`

     Lynae's lower-body shader reads the normal map's alpha as a shading band (around 102 a fabric
     renders about a third as bright as from 128 up); LynaePeppermint's does not, so a satin dress came
     out lighter and glossier. Each band's factor is the measured ratio of the two renders. Nothing is
     decided from the diffuse's own colour
     @endrst
     */
    std::vector<std::array<double, 3>> lynaeShadeBandFactors();

    /**
     * @brief
     @rst
     Lynae's coat film colour, as the factors of a :cpp:class:`GuidedScaleFilter` guided by a jacket code map's
     codes :raw-html:`<br />` :raw-html:`<br />`

     Lynae's film renders the cyan lining texture as a pink-red rainbow; LynaePeppermint's film (see
     :cpp:func:`lynaeFilmCodeTable`) tints its foil by the diffuse, so the lining came out cyan. The texels of
     material 4 are multiplied by the measured ratio of the two renders; every other material by 1
     @endrst
     */
    std::vector<std::array<double, 3>> lynaeFilmFactors();
}

#endif
