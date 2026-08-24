#ifndef AGRemapCore_TextureFile_H
#define AGRemapCore_TextureFile_H

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "compressonator.h"
#include "AGRemapCore/model/textures/Colour.h"

namespace AGRemapCore {

    /**
     * @brief
     @rst
     Class to handle ``.dds`` texture files, backed by `Compressonator`_ instead of an image library
     such as `Pillow`_ -- this keeps texture editing usable on every OS `Compressonator`_ itself
     supports (Windows, Linux, Mac), unlike a Windows-only codec such as `DirectXTex`_ :raw-html:`<br />`
     :raw-html:`<br />`

     The texture's pixels are kept internally as a flat, uncompressed RGBA8 buffer (see #getPixels/
     #setPixels) -- #open decodes whatever compressed/uncompressed format the file was saved in into
     this buffer (remembering the original format), and #save re-encodes the (possibly edited)
     buffer back to that same remembered format
     @endrst
     */
    class TextureFile {
        public:

            /**
             * @brief Constructs a new texture file. Does not read anything from disk yet -- see #open
             *
             * @param src The source file path for the texture file
             */
            explicit TextureFile(std::string src);

            /**
             * @brief The source file path for the texture file
             */
            const std::string& getSrc() const;

            /**
             * @brief Sets the source file path for the texture file
             */
            void setSrc(std::string src);

            /**
             * @brief Whether a texture is currently loaded (#open succeeded and found a real file)
             */
            bool hasImage() const;

            /**
             * @brief The width, in pixels, of the currently loaded texture (0 if #hasImage is ``false``)
             */
            int getWidth() const;

            /**
             * @brief The height, in pixels, of the currently loaded texture (0 if #hasImage is ``false``)
             */
            int getHeight() const;

            /**
             * @brief
             @rst
             The luminance parameter used to gamma-correct the R/G/B channels on the next #save
             (see :cpp:class:`GammaFilter`), or ``std::nullopt`` to skip gamma correction entirely
             @endrst
             */
            std::optional<double> getGamma() const;

            /**
             * @brief Sets #getGamma
             */
            void setGamma(std::optional<double> gamma);

            /**
             * @brief The current pixel buffer, as flat RGBA8 bytes (4 bytes per pixel, row-major,
             *      size = #getWidth * #getHeight * 4)
             */
            const std::vector<std::uint8_t>& getPixels() const;

            /**
             * @brief Replaces the current pixel buffer, eg. with pixels edited outside of this class
             *
             * @param pixels The new flat RGBA8 pixel buffer (4 bytes per pixel, row-major)
             * @param width The width, in pixels, of 'pixels'
             * @param height The height, in pixels, of 'pixels'
             */
            void setPixels(std::vector<std::uint8_t> pixels, int width, int height);

            /**
             * @brief The colour of the pixel at ('x', 'y'). No bounds checking is performed
             *
             * @param x The x-coordinate of the pixel
             * @param y The y-coordinate of the pixel
             */
            Colour getPixel(int x, int y) const;

            /**
             * @brief Sets the colour of the pixel at ('x', 'y'). No bounds checking is performed
             *
             * @param x The x-coordinate of the pixel
             * @param y The y-coordinate of the pixel
             * @param colour The new colour for the pixel
             */
            void setPixel(int x, int y, const Colour &colour);

            /**
             * @brief
             @rst
             Opens the texture file at #getSrc, decoding it into #getPixels :raw-html:`<br />`
             :raw-html:`<br />`

             If the file does not exist, #hasImage becomes ``false`` and #getPixels is cleared
             @endrst
             */
            void open();

            /**
             * @brief
             @rst
             Saves #getPixels to the texture file at #getSrc :raw-html:`<br />` :raw-html:`<br />`

             If #getGamma is set, the R/G/B channels of #getPixels are gamma-corrected first (see
             :cpp:class:`GammaFilter`), in place. The file is re-encoded to whatever compressed
             format it was originally #open-ed with -- or, for a texture file that was never
             successfully opened (eg. a brand new file), :cpp:member:`DefaultFormat`
             @endrst
             */
            void save();

            /**
             * @brief
             @rst
             The compressed format used for a texture file that's never been successfully #open-ed
             (so there's no original format to preserve on #save)
             @endrst
             */
            static constexpr CMP_FORMAT DefaultFormat = CMP_FORMAT_BC7;

        private:
            std::string src_;
            std::vector<std::uint8_t> pixels_;
            int width_ = 0;
            int height_ = 0;
            bool hasImage_ = false;
            CMP_FORMAT format_ = DefaultFormat;
            std::optional<double> gamma_;
    };
}

#endif
