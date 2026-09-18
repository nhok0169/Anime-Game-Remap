#ifndef AGRemapCore_DarkDiffuse_H
#define AGRemapCore_DarkDiffuse_H

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
     The ``DarkDiffuse`` head-diffuse edit, shared by every character whose remap needs it
     :raw-html:`<br />` :raw-html:`<br />`

     Its own translation unit because more than one character uses it and none of them owns it --
     Ningguang and Ganyu both declare it, with identical filters, in the pure-Python
     ``IniParseBuilderFuncs._ningguangEditHeadDiffuse`` / ``_ganyuEditHeadDiffuse``
     @endrst
     */
    class DarkDiffuse {
        public:

            DarkDiffuse() = delete;

            /**
             * @brief
             @rst
             The gamma written into the texture's metadata, and the reason this is not simply "set
             alpha to 0" :raw-html:`<br />` :raw-html:`<br />`

             ``1 / 2.2`` -- the sRGB exponent, the pure-Python ``1 /
             ColourConsts.StandardGamma``. :cpp:func:`TextureFile::save` applies it through
             :cpp:class:`GammaFilter` right before writing, so the R/G/B channels are pre-corrected
             for the ``.dds``/BCn sRGB round trip
             @endrst
             */
            static const double Gamma;

            /**
             * @brief
             @rst
             Makes a head diffuse fully transparent and gamma-corrects what is left
             :raw-html:`<br />` :raw-html:`<br />`

             Two steps, and both matter:

             #. alpha to **0** everywhere. In these textures alpha is a mask the shader reads rather
                than opacity, so this is "the mask says nothing", not "the texture is invisible" --
                see :cpp:func:`TexEditor::setTransparency`
             #. \\ref Gamma into the texture's metadata, which :cpp:func:`TextureFile::save` applies
                on the way out

             A direct port of the pure-Python pair
             ``[_xxxEditHeadDiffuse, TexMetadataFilter({Gamma: 1 / StandardGamma})]``
             @endrst
             *
             * @param texFile The texture to edit, modified in place. A file with no image is left alone
             */
            static void edit(TextureFile& texFile);
    };
}

#endif
