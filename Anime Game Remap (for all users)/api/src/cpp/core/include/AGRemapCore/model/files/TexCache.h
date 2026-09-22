#ifndef AGRemapCore_TexCache_H
#define AGRemapCore_TexCache_H

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

#include <cstddef>
#include <cstdint>
#include <list>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "compressonator.h"
#include "AGRemapCore/tools/hashing/Hash128.h"

namespace AGRemapCore {

    /**
     * @brief
     @rst
     The two things a run repeats when it edits textures: decoding the same source file, and
     writing the same pixels :raw-html:`<br />` :raw-html:`<br />`

     A fix edits a texture once per DESTINATION, not once per distinct image, so a mod whose
     toggles all reference the same few textures pays the full cost every time. Measured on a
     194-``.ini`` Ayaka6 mod: **579 edit operations producing 7 distinct texture contents**, and
     on a 1024x1024 ``BC7`` source at the CLI's default (uncompressed) settings each of those is
     ~0.20s to decode plus ~0.26s to write, against ~0.003s for the disk I/O the write actually
     needs :raw-html:`<br />` :raw-html:`<br />`

     **Both halves are keyed on CONTENT, and neither assumes anything about the filters.** That
     is the whole design constraint: :cpp:type:`TexEditor::Filter` is a
     ``std::function<void(TextureFile&)>``, so a chain cannot be introspected for purity or for
     what else it reads -- and at least one real filter
     (:cpp:class:`MaterialBandRemapFilter`) reads a *second* file, its diffuse gate, where an
     absent diffuse changes the result. So:

     - the **decode** side is keyed on the source file's bytes. Decoding is a pure function of
       those bytes; no filter has run yet.
     - the **write** side is keyed on the finished pixel buffer plus the format/compress/mipmaps
       the write would use. Every filter still runs, every time -- only the encode-and-write of
       an image we have already written is replaced, with a file copy.

     .. note::
        Not synchronized, and deliberately so -- one lives per :cpp:class:`RemapService`, which
        walks its folders sequentially. Sharing one across threads needs a lock added here first,
        exactly as :cpp:class:`DownloadCache` says of itself :raw-html:`<br />` :raw-html:`<br />`

        Nothing here checks that a remembered output is still ON DISK. It does not need to:
        :cpp:func:`TextureFile::writeTo` falls back to a real write when the copy fails, so a
        remembered path that has since been deleted costs one failed ``copy_file`` and self-heals
     @endrst
     */
    class TexCache {
        public:

            /**
             * @brief
             @rst
             How many bytes of decoded pixels to keep before evicting the least recently used
             :raw-html:`<br />` :raw-html:`<br />`

             Decoded pixels are RGBA8 and so far larger than the file they came from -- a
             4096x4096 texture is 64MB decoded. This is a budget rather than an entry count for
             that reason: a handful of big textures must not be allowed to hold half a gigabyte
             on the 16GB laptop this library is also developed on
             @endrst
             */
            static constexpr std::size_t DefaultMaxBytes = 256u * 1024u * 1024u;

            /**
             * @brief One decoded texture, exactly the state #open leaves a :cpp:class:`TextureFile` in
             */
            struct Decoded {
                /** @brief The flat, uncompressed RGBA8 pixels */
                std::vector<std::uint8_t> pixels;

                /** @brief The width, in pixels */
                int width = 0;

                /** @brief The height, in pixels */
                int height = 0;

                /** @brief The format the file was originally stored in, which a save re-encodes to */
                CMP_FORMAT format = CMP_FORMAT_Unknown;

                /**
                 * @brief
                 @rst
                 The gamma :cpp:func:`TextureFile::open` took from the source's sRGB header bit,
                 if it set one :raw-html:`<br />` :raw-html:`<br />`

                 Part of the decoded state and **not** an optional extra: :cpp:func:`TextureFile::save`
                 runs a :cpp:class:`GammaFilter` when this is set, so a cache hit that restored the
                 pixels but not this would hand the next texture the PREVIOUS one's gamma
                 @endrst
                 */
                std::optional<double> gamma;
            };

            /**
             * @brief Constructs a new texture cache
             *
             * @param maxBytes The budget for decoded pixels -- see #DefaultMaxBytes. ``0`` disables the decode half entirely
             */
            explicit TexCache(std::size_t maxBytes = DefaultMaxBytes);

            /**
             * @brief
             @rst
             The decoded pixels for a source file whose bytes hash to 'source', if they are still
             held :raw-html:`<br />` :raw-html:`<br />`

             Promotes the entry to most-recently-used. The pointer is invalidated by the next
             :cpp:func:`rememberDecoded`
             @endrst
             *
             * @param source The hash of the source file's bytes
             */
            const Decoded* decoded(const Hash128& source);

            /**
             * @brief Records the decoded pixels for the source file whose bytes hash to 'source'
             *
             * @param source The hash of the source file's bytes
             * @param value The decoded texture
             */
            void rememberDecoded(const Hash128& source, Decoded value);

            /**
             * @brief
             @rst
             Where an identical output was already written during this run, if anywhere
             :raw-html:`<br />` :raw-html:`<br />`

             Non-``const`` because a hit is counted here -- see :cpp:func:`getWriteHits`. Calling
             it from a ``const`` method is still fine: what a caller holds is a
             ``TexCache*``, and it is the POINTER that is const there, not the cache
             @endrst
             *
             * @param output The hash of the finished pixels plus the write's own settings
             */
            std::optional<std::string> writtenAs(const Hash128& output);

            /**
             * @brief Records that an output hashing to 'output' now sits at 'path'
             *
             * @param output The hash of the finished pixels plus the write's own settings
             * @param path The full path the texture was written to
             */
            void rememberWritten(const Hash128& output, std::string path);

            /**
             * @brief Forgets a remembered output -- used when copying from it failed, so the next
             *        caller writes it properly rather than retrying a copy that cannot work
             *
             * @param output The hash to forget
             */
            void forgetWritten(const Hash128& output);

            /** @brief How many decodes this run skipped */
            std::size_t getDecodeHits() const;

            /** @brief How many decodes this run actually performed */
            std::size_t getDecodeMisses() const;

            /** @brief How many texture writes this run served with a file copy */
            std::size_t getWriteHits() const;

            /** @brief How many texture writes this run actually encoded */
            std::size_t getWriteMisses() const;

            /** @brief How many bytes of decoded pixels are currently held */
            std::size_t getBytes() const;

        private:
            void evict();

            struct Entry {
                Decoded value;
                std::list<Hash128>::iterator order;
            };

            // AGREMAP_TEXCACHE=0 turns both halves off, for measuring the cache against itself
            // in one binary -- see the constructor
            bool enabled_ = true;
            std::size_t maxBytes_;
            std::size_t bytes_ = 0;

            // front is most-recently-used
            std::list<Hash128> order_;
            std::unordered_map<Hash128, Entry> decoded_;

            std::unordered_map<Hash128, std::string> written_;

            // The reverse of written_, so a second write to a destination can invalidate the
            // entry that still claims the old image lives there -- see rememberWritten
            std::unordered_map<std::string, Hash128> pathOwner_;

            std::size_t decodeHits_ = 0;
            std::size_t decodeMisses_ = 0;
            std::size_t writeHits_ = 0;
            std::size_t writeMisses_ = 0;
    };
}

#endif
