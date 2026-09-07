#ifndef AGRemapCore_TexEditor_H
#define AGRemapCore_TexEditor_H

#include <functional>
#include <string>
#include <vector>

#include "AGRemapCore/model/strategies/texEditors/BaseTexEditor.h"

namespace AGRemapCore {

    class TextureFile;

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BaseTexEditor`

     Class for editing a texture file by running a fixed sequence of filters over it
     @endrst
     */
    class TexEditor: public BaseTexEditor {
        public:

            /**
             * @brief A single edit applied to a :cpp:class:`TextureFile` by #fix
             */
            using Filter = std::function<void(TextureFile&)>;

            /**
             * @brief
             @rst
             Sets the alpha channel of **every** pixel of 'texFile' to 'alpha', the way the
             pure-Python ``TexEditor.setTransparency`` does (``texFile.img.putalpha(alpha)``)
             :raw-html:`<br />` :raw-html:`<br />`

             ``0`` is fully transparent and ``255`` fully opaque, and values outside that range are
             clamped into it :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                This **overwrites** the alpha channel rather than adjusting it, which is what
                separates it from :cpp:class:`Transparency` -- and in these textures alpha is very
                often not opacity at all but a *mask* the shader reads (a blush mask, on a face
                diffuse), so "make it uniform" and "adjust it" are genuinely different operations.
                See `Texture Editing <../TextureEditing/CLAUDE.md>`_
                :raw-html:`<br />` :raw-html:`<br />`

                A no-op when 'texFile' has no image loaded -- there are no pixels to set
             @endrst
             *
             * @param texFile The texture file to edit, in place
             * @param alpha The alpha value to give every pixel, from 0 (transparent) to 255 (opaque)
             */
            static void setTransparency(TextureFile &texFile, int alpha);

            /**
             * @brief Constructs a new texture editor
             *
             * @param filters The filters for editing the image, applied in order
             * @param compress
             @rst
             Whether the edited texture is written back **compressed** -- see #getCompress.
             **Default**: ``true``
             @endrst
             */
            explicit TexEditor(std::vector<Filter> filters = {}, bool compress = true);

            /**
             * @brief The filters for editing the image
             */
            const std::vector<Filter>& getFilters() const;

            /**
             * @brief Sets #getFilters
             */
            void setFilters(std::vector<Filter> filters);

            /**
             * @brief
             @rst
             Whether #fix writes the edited texture back in its original compressed format, or as a
             plain 32-bit uncompressed ``.dds`` :raw-html:`<br />` :raw-html:`<br />`

             Handed straight to :cpp:func:`TextureFile::save`, which documents the trade in full.
             The short version: BCn encoding is almost the entire cost of an edit (~15.5s of a
             17-second round trip on a 4096x2048 BC7 texture), and turning it off makes the edit
             roughly eight times faster in exchange for a file about four times larger
             :raw-html:`<br />` :raw-html:`<br />`

             ``false`` is what the pure-Python `Pillow`_ engine did unconditionally -- it never
             encoded BCn at all :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``true``
             @endrst
             */
            bool getCompress() const;

            /**
             * @brief Sets #getCompress
             */
            void setCompress(bool compress);

            /**
             * @brief
             @rst
             Edits the texture file :raw-html:`<br />` :raw-html:`<br />`

             No-op if #getFilters is empty, or if 'texFile' does not exist on disk. Otherwise, opens
             'texFile', runs every filter in #getFilters over it in order, then saves it to
             'fixedTexFile'
             @endrst
             *
             * @param texFile The texture ``.dds`` file to be modified
             * @param fixedTexFile The name of the fixed texture file
             */
            void fix(TextureFile &texFile, const std::string &fixedTexFile) override;

        private:
            std::vector<Filter> filters_;
            bool compress_;
    };
}

#endif
