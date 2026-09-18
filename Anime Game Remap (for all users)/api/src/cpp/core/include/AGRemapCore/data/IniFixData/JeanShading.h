#ifndef AGRemapCore_JeanShading_H
#define AGRemapCore_JeanShading_H

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


namespace AGRemapCore {

    class TextureFile;

    /**
     * @brief
     @rst
     The texture edit Jean and JeanCN both apply when remapping onto JeanSea :raw-html:`<br />`
     :raw-html:`<br />`

     Its own translation unit because both characters use it and neither owns it -- the same reason
     :cpp:func:`makeGIMICharFixer` is shared rather than copied per character
     @endrst
     */
    class JeanShading {
        public:

            JeanShading() = delete;

            /**
             * @brief
             @rst
             The alpha value at or below which a pixel is lifted, and the amount it is lifted by --
             one constant serving as both, exactly as the pure-Python
             ``_jeanEditBodyLightMap5_5`` uses ``77`` for both
             @endrst
             */
            static const int LowAlpha;

            /**
             * @brief
             @rst
             Lifts the darkest part of a lightmap's alpha channel :raw-html:`<br />`
             :raw-html:`<br />`

             Every pixel whose alpha is at or below \\ref LowAlpha gains \\ref LowAlpha; everything
             brighter is left exactly as it was. In a `GIMI`_ lightmap that channel drives how
             strongly the shader shades a surface, so this lightens only what was nearly black --
             which is what stops JeanSea's cape reading as a hard edge against the body under it
             :raw-html:`<br />` :raw-html:`<br />`

             A direct port of the pure-Python ``IniParseBuilderFuncs._jeanEditBodyLightMap5_5``,
             whose ``alphaImg.point(lambda a: bound(a + 77) if a <= 77 else a)`` says the same thing.
             The bound never actually binds -- ``77 + 77`` is 154 -- but it is kept because the
             original has it and a future caller may pass a larger constant
             @endrst
             *
             * @param texFile The texture to edit, modified in place. A file with no image is left alone
             */
            static void liftLowAlpha(TextureFile& texFile);
    };
}

#endif
