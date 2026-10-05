#ifndef AGRemapCore_TexCreator_H
#define AGRemapCore_TexCreator_H

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

#include "AGRemapCore/model/strategies/texEditors/BaseTexEditor.h"
#include "AGRemapCore/model/textures/Colour.h"

namespace AGRemapCore {

    class TextureFile;

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BaseTexEditor`

     Creates a brand new ``.dds`` file if the file does not already exist
     @endrst
     */
    class TexCreator: public BaseTexEditor {
        public:

            /**
             * @brief The width, in pixels, of the texture to create
             */
            int width;

            /**
             * @brief The height, in pixels, of the texture to create
             */
            int height;

            /**
             * @brief The fill colour of the texture to create
             */
            Colour colour;

            /**
             * @brief
             @rst
             Whether the created texture is written in a compressed format, or as a plain 32-bit
             uncompressed ``.dds`` :raw-html:`<br />` :raw-html:`<br />`

             The :cpp:class:`TexCreator` counterpart of :cpp:func:`TexEditor::getCompress`, and
             carried for the same reason: BCn encoding is almost the entire cost of writing a
             texture, so a caller that would rather have a bigger file sooner can say so. A created
             texture has never been :cpp:func:`TextureFile::open`-ed, so the format it compresses to
             is :cpp:member:`TextureFile::DefaultFormat` rather than a remembered one
             :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``true``
             @endrst
             */
            bool compress;

            /**
             * @brief
             @rst
             Whether the created texture is written with its full mip chain -- the
             :cpp:class:`TexCreator` counterpart of :cpp:func:`TexEditor::getMipmaps`, handed to
             :cpp:func:`TextureFile::save` :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``false``
             @endrst
             */
            bool mipmaps;

            /**
             * @brief Constructs a new texture creator
             *
             * @param width The width, in pixels, of the texture to create
             * @param height The height, in pixels, of the texture to create
             * @param colour The fill colour of the texture to create, defaulting to opaque white
             * @param compress Whether the created texture is compressed -- see #compress. **Default**: ``true``
             * @param mipmaps Whether the created texture carries its mip chain -- see #mipmaps. **Default**: ``false``
             */
            TexCreator(int width, int height, Colour colour = Colour(), bool compress = true, bool mipmaps = false);

            /**
             * @brief
             @rst
             No-op if a file already exists at ``texFile.getSrc()``. Otherwise, builds a new
             #width by #height texture, filled entirely with #colour, and saves it to
             'fixedTexFile'
             @endrst
             *
             * @param texFile The texture ``.dds`` file to be modified
             * @param fixedTexFile The name of the fixed texture file
             */
            void fix(TextureFile &texFile, const std::string &fixedTexFile) override;
    };
}

#endif
