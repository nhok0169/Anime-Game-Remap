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
             *
             * @param compress
             @rst
             Whether to re-encode to that compressed format, or write the RGBA8 buffer out as a
             **plain 32-bit uncompressed** ``.dds`` :raw-html:`<br />` :raw-html:`<br />`

             **This is a speed/size trade, and a large one.** BCn encoding dominates the cost of
             editing a texture: on Jean's 4096x2048 ``BC7_UNORM`` body lightmap, decoding takes
             ~1.5s and re-encoding ~15.5s, so ``false`` turns a 17-second edit into a 2-second one.
             What it costs is file size and format -- that same texture comes out at 32MB
             uncompressed against 8MB as BC7 :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                ``false`` is exactly what the pure-Python `Pillow`_ engine always did:
                ``img.save(src, 'DDS')`` writes 32-bit uncompressed and never encodes BCn at all.
                That is worth knowing when comparing the two implementations' speed -- they were
                never doing the same work

             **Default**: ``true``
             @endrst
             */
            void save(bool compress = true);

            /**
             * @brief
             @rst
             Saves #getPixels to 'dest', leaving both #getSrc and #getPixels untouched -- the
             on-disk format is chosen from 'dest's own file extension :raw-html:`<br />`
             :raw-html:`<br />`

             A ``.dds`` destination is re-encoded to the same compressed format #save would use;
             **any other extension is written uncompressed**, straight from the RGBA8 buffer.
             `Compressonator`_ handles ``.png``, ``.bmp`` and ``.jpg`` itself this way, which is
             what makes this the "convert a texture into something an ordinary image viewer can
             open" entry point :raw-html:`<br />` :raw-html:`<br />`

             Unlike #save, this **never** applies #getGamma. Gamma here is a pre-correction for the
             ``.dds``/BCn sRGB round trip specifically (and #save applies it destructively, in
             place, to #getPixels) -- neither is wanted when the point is to look at the texture's
             actual decoded pixels
             @endrst
             *
             * @param dest The file path to write to
             * @return Whether the file was actually written
             */
            bool saveAs(const std::string &dest) const;

            /**
             * @brief
             @rst
             The compressed format used for a texture file that's never been successfully #open-ed
             (so there's no original format to preserve on #save)
             @endrst
             */
            static constexpr CMP_FORMAT DefaultFormat = CMP_FORMAT_BC7;

        private:
            /**
             * @brief Writes #getPixels to 'dest', optionally compressing it to #format_ first
             *
             * @param dest The file path to write to
             * @param compress Whether to re-encode to the remembered compressed format first
             * @return Whether the file was actually written
             */
            bool writeTo(const std::string &dest, bool compress) const;

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
