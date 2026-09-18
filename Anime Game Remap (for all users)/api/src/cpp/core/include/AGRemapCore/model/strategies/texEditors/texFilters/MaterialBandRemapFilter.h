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

#ifndef MATERIAL_BAND_REMAP_FILTER_H
#define MATERIAL_BAND_REMAP_FILTER_H

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "AGRemapCore/model/strategies/texEditors/TexEditor.h"
#include "AGRemapCore/model/strategies/texEditors/texFilters/BaseTexFilter.h"

namespace AGRemapCore {

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BaseTexFilter`

     Moves a light map's MATERIAL BANDS from one skin's legend onto another's, optionally
     conditioned on the DIFFUSE underneath each pixel :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        **A light map's alpha channel is a material band** -- the shader reads it to decide how to
        shade the pixel (skin, hair, cloth, metal, fur) -- and **the legend differs per skin**. Two
        characters remapped onto one another agree on some bands and disagree on others, so a remap
        that copies a light map across unchanged shades the wrong material: a shawl painted on the
        fur band renders as skin, grey stockings come out beige.

     .. note::
        **Why the diffuse gate exists.** A mod need not follow its own character's legend at all,
        because **a mod that is itself a PORT carries some THIRD character's bands** -- a Clorinde
        port of a Yelan skin keeps Clorinde's legend, with hair where the target expects skin.
        Lifting that hair onto the skin ramp put speckles all over it. So a move may be conditioned
        on the colour under the pixel actually looking like the material being moved. A move whose
        source band cannot be mistaken for anything else needs no gate; a move off band ``0`` -- the
        DEFAULT that a lazy or ported mod leaves everything on -- always does.

     .. warning::
        Every decision is made from the **ORIGINAL** alpha, never from a value an earlier band
        wrote. The moves are typically a *permutation* of the bands, and applied in sequence a
        permutation chases itself: ``0 -> 255``, then ``255 -> 121``, and band 0 has landed on 121.

     Where the light map and the diffuse differ in size the diffuse is sampled nearest-neighbour,
     and **a diffuse that is absent or cannot be read makes every gate PASS** -- the band legend is
     the better of the two guesses when there is nothing to check against
     @endrst
     */
    class MaterialBandRemapFilter: public BaseTexFilter {
        public:

            /**
             * @brief A test on the diffuse colour under a pixel, as ``(red, green, blue)`` in
             *      0-255
             */
            using ColourPredicate = std::function<bool(int, int, int)>;

            /**
             * @brief One band move: a source band (or range) onto a target band, optionally gated
             *      on the diffuse
             */
            struct Band {

                /**
                 * @brief The lowest alpha of the SOURCE band, inclusive
                 */
                std::uint8_t low = 0;

                /**
                 * @brief The highest alpha of the SOURCE band, inclusive
                 */
                std::uint8_t high = 0;

                /**
                 * @brief The alpha to write
                 */
                std::uint8_t to = 0;

                /**
                 * @brief
                 @rst
                 The test the diffuse under the pixel must pass for the move to happen. An empty
                 predicate moves the band unconditionally
                 @endrst
                 */
                ColourPredicate when = nullptr;

                /**
                 * @brief
                 @rst
                 Whether #when is inverted -- the move happens where the diffuse does **not** pass
                 it. A hair band is often best described as "not skin"
                 @endrst
                 */
                bool negate = false;

                Band() = default;

                /**
                 * @brief Constructs a move off a single band
                 *
                 * @param band The source band
                 * @param to The alpha to write
                 * @param when The test the diffuse must pass; empty to move unconditionally
                 * @param negate Whether 'when' is inverted
                 */
                Band(std::uint8_t band, std::uint8_t to, ColourPredicate when = nullptr, bool negate = false);

                /**
                 * @brief Constructs a move off a range of bands
                 *
                 * @param low The lowest alpha of the source band, inclusive
                 * @param high The highest alpha of the source band, inclusive
                 * @param to The alpha to write
                 * @param when The test the diffuse must pass; empty to move unconditionally
                 * @param negate Whether 'when' is inverted
                 */
                Band(std::uint8_t low, std::uint8_t high, std::uint8_t to, ColourPredicate when = nullptr, bool negate = false);
            };

            /**
             * @brief Constructs a new material band remap filter
             *
             * @param bands The moves to apply, all from the ORIGINAL alpha; the FIRST whose source
             *      range contains a pixel's band is the one that applies
             * @param diffusePath The diffuse to read the gates against. If this names no readable
             *      file, every gate passes
             */
            explicit MaterialBandRemapFilter(std::vector<Band> bands = {}, std::string diffusePath = "");

            /**
             * @brief The moves to apply, all from the ORIGINAL alpha
             */
            std::vector<Band> bands;

            /**
             * @brief The diffuse to read the gates against
             */
            std::string diffusePath;

            void transform(TextureFile &texFile) override;

            /**
             * @brief
             @rst
             Whether a colour looks like SKIN -- warm, not too dark, red at least green at least
             blue, and separated enough to not be a grey :raw-html:`<br />` :raw-html:`<br />`

             Measured off two characters' identity mods rather than chosen; white fur and a white
             eye sclera are the same colour, so this deliberately does not try to separate them
             @endrst
             *
             * @param red The red channel, 0-255
             * @param green The green channel, 0-255
             * @param blue The blue channel, 0-255
             */
            static bool skinColoured(int red, int green, int blue);

            /**
             * @brief
             @rst
             Whether a colour looks like WHITE FUR -- bright, and close to grey
             @endrst
             *
             * @param red The red channel, 0-255
             * @param green The green channel, 0-255
             * @param blue The blue channel, 0-255
             */
            static bool whiteFurColoured(int red, int green, int blue);

            /**
             * @brief
             @rst
             The per-object light map edit both fixer templates' ``lightMapEdit`` field wants: a
             function that, given an object's diffuse, returns the filter to run over its light map
             :raw-html:`<br />` :raw-html:`<br />`

             So a character's whole band legend is one table handed to one call, rather than a
             hand-written closure per direction
             @endrst
             *
             * @param bands The moves to apply
             */
            static std::function<TexEditor::Filter(const std::string&)> lightMapEdit(std::vector<Band> bands);
    };
}

#endif
