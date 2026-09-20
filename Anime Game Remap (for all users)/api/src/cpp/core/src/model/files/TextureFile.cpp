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

#include "AGRemapCore/tools/files/FileService.h"
#include "AGRemapCore/model/files/TextureFile.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <optional>
#include <thread>
#include <utility>
#include <vector>

// Compressonator's block-level codec library, for the BC7 decoder below. The framework's
// CMP_ConvertMipTexture is a whole-texture call and the only one this file needed until the decode
// turned out to be ~85% of what editing a texture costs.
#include "cmp_core.h"

#include "AGRemapCore/model/strategies/texEditors/texFilters/GammaFilter.h"
#include "AGRemapCore/tools/StringTools.h"


namespace {
    // ===== COMPRESSONATOR CANNOT BE HANDED A NON-ASCII PATH =====
    //
    // CMP_LoadTexture and CMP_SaveTexture take a narrow const char*, and Windows decodes that in
    // the ACTIVE CODE PAGE. Every path in this codebase is UTF-8, so a mod folder named in Korean
    // reaches the library as mojibake naming nothing: CMP_SaveTexture returns non-OK and writes no
    // file. FileService::strToPath cannot help -- there is no wide overload to give the path to.
    //
    // So the path never goes to Compressonator at all when it is not pure ASCII. The library reads
    // and writes a temporary file with an ASCII name, and std::filesystem -- which IS unicode-safe
    // -- moves the bytes the rest of the way. An ASCII path (very nearly every real one) takes the
    // direct route exactly as before, so this costs nothing in the common case.

    bool isAsciiPath(const std::string& path) {
        for (unsigned char c : path) {
            if (c >= 0x80) {
                return false;
            }
        }
        return true;
    }

    // An ASCII scratch path next to the system temp folder. Returns nullopt when even THAT is not
    // ASCII (a user profile named in a non-Latin script), in which case the caller falls back to
    // the direct call -- no worse than the old behaviour, and still honest about failing.
    std::optional<std::filesystem::path> asciiTempPath(const std::string& modelPath) {
        std::error_code ec;
        std::filesystem::path tempDir = std::filesystem::temp_directory_path(ec);
        if (ec) {
            return std::nullopt;
        }

        const std::string tempDirStr = AGRemapCore::FileService::pathToStr(tempDir);
        if (!isAsciiPath(tempDirStr)) {
            return std::nullopt;
        }

        // The EXTENSION is load-bearing: Compressonator picks its reader and its writer off the
        // file extension, so the scratch file has to keep the real one or a .dds would be written
        // as something else entirely.
        std::string ext = AGRemapCore::FileService::pathToStr(
            std::filesystem::path(AGRemapCore::FileService::strToPath(modelPath)).extension());
        if (!isAsciiPath(ext)) {
            ext = ".dds";
        }

        static std::atomic<unsigned long long> counter{0};
        const std::string name = "AGRemapTex" + std::to_string(counter++) + ext;
        return tempDir / AGRemapCore::FileService::strToPath(name);
    }

    // Write via that scratch file, then move the result into place. std::filesystem::rename is
    // the fast path but fails across volumes (the temp folder is often on another drive), so a
    // copy-and-remove stands behind it.
    bool saveThroughAscii(const std::string& dest, CMP_MipSet* mipSet) {
        if (isAsciiPath(dest)) {
            return CMP_SaveTexture(dest.c_str(), mipSet) == CMP_OK;
        }

        const std::optional<std::filesystem::path> scratch = asciiTempPath(dest);
        if (!scratch.has_value()) {
            return CMP_SaveTexture(dest.c_str(), mipSet) == CMP_OK;
        }

        const std::string scratchStr = AGRemapCore::FileService::pathToStr(*scratch);
        if (CMP_SaveTexture(scratchStr.c_str(), mipSet) != CMP_OK) {
            std::error_code rmEc;
            std::filesystem::remove(*scratch, rmEc);
            return false;
        }

        const std::filesystem::path destPath = AGRemapCore::FileService::strToPath(dest);
        std::error_code ec;
        std::filesystem::rename(*scratch, destPath, ec);
        if (ec) {
            ec.clear();
            std::filesystem::copy_file(*scratch, destPath,
                                        std::filesystem::copy_options::overwrite_existing, ec);
            std::error_code rmEc;
            std::filesystem::remove(*scratch, rmEc);
        }

        return !ec;
    }
}


namespace AGRemapCore {

    namespace {
        // ---- the DX10 header, read by hand ----
        //
        // Compressonator's DDS plugin maps a DX10 header's DXGI format through a table that
        // covers the BCn formats and stops: every UNCOMPRESSED DXGI format loads with its
        // pixels intact and comes back CMP_FORMAT_Unknown, which CMP_ConvertMipTexture then
        // refuses (CMP_ERR_UNSUPPORTED_SOURCE_FORMAT). Since the format is right there in the
        // file, reading it is a better answer than guessing.
        //
        // Only the 32-bit-per-pixel unorm formats a GI mod actually ships are mapped -- a game
        // texture is BCn, an uncompressed one is something a mod author exported, and every
        // one seen so far is one of these four. Anything else is left Unknown, which fails
        // exactly as it does today rather than being silently misread.
        constexpr std::size_t MagicOffset = 0;
        constexpr std::size_t FourCCOffset = 84;
        constexpr std::size_t DxgiFormatOffset = 128;
        constexpr std::size_t HeaderSize = DxgiFormatOffset + 4;

        constexpr std::uint32_t DxgiRGBA8Unorm = 28;
        constexpr std::uint32_t DxgiRGBA8UnormSrgb = 29;
        constexpr std::uint32_t DxgiBGRA8Unorm = 87;
        constexpr std::uint32_t DxgiBGRA8UnormSrgb = 91;

        // The COMPRESSED sRGB formats. Compressonator maps these itself, so they never need a
        // format naming -- but they are still sRGB, and that is what decides the gamma below.
        constexpr std::uint32_t DxgiBC1UnormSrgb = 72;
        constexpr std::uint32_t DxgiBC2UnormSrgb = 75;
        constexpr std::uint32_t DxgiBC3UnormSrgb = 78;
        constexpr std::uint32_t DxgiBC7UnormSrgb = 99;

        // The sRGB pre-correction, spelled as the division so it reads as the exponent it is
        // -- the same value, and the same reason, as DarkDiffuse::Gamma.
        constexpr double SrgbGamma = 1.0 / 2.2;

        /**
         * What a DX10 header says the texture is.
         *
         * The two facts are INDEPENDENT and are used independently. 'format' is only set for the
         * uncompressed formats Compressonator cannot map on its own; 'srgb' is set for every sRGB
         * format, compressed or not, because that one decides the gamma and a BCn texture needs it
         * just as much as an uncompressed one does.
         */
        struct DX10Format {
            CMP_FORMAT format = CMP_FORMAT_Unknown;
            bool srgb = false;
        };

        std::uint32_t readLE32(const std::vector<char>& header, std::size_t at) {
            return static_cast<std::uint32_t>(static_cast<unsigned char>(header[at]))
                    | (static_cast<std::uint32_t>(static_cast<unsigned char>(header[at + 1])) << 8)
                    | (static_cast<std::uint32_t>(static_cast<unsigned char>(header[at + 2])) << 16)
                    | (static_cast<std::uint32_t>(static_cast<unsigned char>(header[at + 3])) << 24);
        }

        /**
         * What the DX10 header says, or nullopt for a file that has no DX10 header at all --
         * a legacy header carries no sRGB bit and no DXGI format, so there is nothing to read.
         */
        std::optional<DX10Format> dx10Format(const std::string& src) {
            std::ifstream file(FileService::strToPath(src), std::ios::binary);
            if (!file) {
                return std::nullopt;
            }

            std::vector<char> header(HeaderSize);
            file.read(header.data(), static_cast<std::streamsize>(HeaderSize));
            if (file.gcount() != static_cast<std::streamsize>(HeaderSize)) {
                return std::nullopt;
            }

            if (std::memcmp(header.data() + MagicOffset, "DDS ", 4) != 0
                    || std::memcmp(header.data() + FourCCOffset, "DX10", 4) != 0) {
                return std::nullopt;
            }

            switch (readLE32(header, DxgiFormatOffset)) {
                case DxgiRGBA8Unorm:
                    return DX10Format{CMP_FORMAT_RGBA_8888, false};

                case DxgiRGBA8UnormSrgb:
                    return DX10Format{CMP_FORMAT_RGBA_8888, true};

                case DxgiBGRA8Unorm:
                    return DX10Format{CMP_FORMAT_BGRA_8888, false};

                case DxgiBGRA8UnormSrgb:
                    return DX10Format{CMP_FORMAT_BGRA_8888, true};

                // Compressed and sRGB: no format to supply, but the gamma still applies.
                case DxgiBC1UnormSrgb:
                case DxgiBC2UnormSrgb:
                case DxgiBC3UnormSrgb:
                case DxgiBC7UnormSrgb:
                    return DX10Format{CMP_FORMAT_Unknown, true};

                default:
                    return DX10Format{};
            }
        }

        // ===== DECODING BC7 OURSELVES =====
        //
        // CMP_ConvertMipTexture is the library's whole-texture decode, and on a 4096x4096 BC7
        // texture it takes 1.95s -- about 1.9us for each 4x4 block, which is orders of magnitude
        // more than unpacking a block should cost. Measured against the rest of a texture edit at
        // the CLI's default settings, it IS the edit: 2.45s of decode against 0.30s of gamma and
        // 0.05s to write the 64MB file.
        //
        // Compressonator's own CMP_Core exposes DecompressBlockBC7 for a single block, and calling
        // it per block -- across threads, since the blocks are independent -- is 11.9x faster over
        // the corpus and gives THE SAME BYTES. That second half is the part that made this
        // shippable and the part that had to be earned: 149 distinct BC7 textures decoded both
        // ways, all 149 identical.
        //
        // BC1 IS DELIBERATELY NOT CLAIMED, and this is the finding worth keeping. The corpus's one
        // BC1 texture came back differing from the framework decode on 1456 of 67108864 bytes --
        // off by one, on interpolated colours -- so the two decoders round BC1's 2/3-1/3 blend
        // differently. Nothing warns about that; it was found only by decoding every texture both
        // ways and comparing. Anything that is not BC7 falls through to the framework, which is
        // also what happens for an odd size or a file whose data is shorter than its own
        // dimensions claim. This can only ever be faster, never a new way to be wrong.

        /**
         * ``AGREMAP_BC7_DECODE=0`` forces the framework path, for an A/B inside one binary
         *
         * TRIMMED before comparing, which is not fussiness. ``cmd``'s ``set VAR=0 && prog`` hands
         * the child ``"0 "`` -- everything up to the ``&&``, trailing space included -- so an exact
         * comparison leaves the switch ON while the caller believes it is off. That turned this
         * option's own test into one that compared the fast path against itself and passed against
         * a deliberately broken build.
         */
        bool bc7DecodeEnabled() {
            static const bool enabled = []() {
                const char *raw = std::getenv("AGREMAP_BC7_DECODE");
                if (raw == nullptr) {
                    return true;
                }

                return StringTools::strip(std::string(raw)) != "0";
            }();

            return enabled;
        }

        /** One horizontal band of 4x4 blocks. Bands never overlap, so threads never share a pixel */
        void decodeBc7Rows(const std::uint8_t *blocks, std::uint8_t *pixels, int width,
                            int blocksX, int rowStart, int rowEnd) {
            std::array<std::uint8_t, 64> block{};

            for (int blockY = rowStart; blockY < rowEnd; ++blockY) {
                for (int blockX = 0; blockX < blocksX; ++blockX) {
                    const std::uint8_t *compressed =
                        blocks + ((static_cast<std::size_t>(blockY) * blocksX + blockX) * 16u);

                    DecompressBlockBC7(compressed, block.data(), nullptr);

                    // 16 RGBA pixels, row-major within the 4x4 -- so four rows of four pixels,
                    // each landing 'width' pixels apart in the destination.
                    for (int row = 0; row < 4; ++row) {
                        const std::size_t destRow = static_cast<std::size_t>(blockY) * 4u + row;
                        const std::size_t destCol = static_cast<std::size_t>(blockX) * 4u;

                        std::memcpy(pixels + ((destRow * width + destCol) * 4u),
                                    block.data() + (static_cast<std::size_t>(row) * 16u), 16u);
                    }
                }
            }
        }

        /**
         * Decodes mip 0 of a BC7 mip set into 'pixels', or answers false for anything this does
         * not claim -- in which case the caller uses the framework exactly as it did before
         */
        bool decodeBc7Level(CMP_MipSet &mipSet, std::vector<std::uint8_t> &pixels,
                            int &width, int &height) {
            if (!bc7DecodeEnabled() || mipSet.m_format != CMP_FORMAT_BC7) {
                return false;
            }

            CMP_MipLevel *level = nullptr;
            CMP_GetMipLevel(&level, &mipSet, 0, 0);
            if (level == nullptr || level->m_pbData == nullptr) {
                return false;
            }

            const int levelWidth = level->m_nWidth;
            const int levelHeight = level->m_nHeight;

            // A BCn texture is stored as whole 4x4 blocks, so a size that is not a multiple of 4
            // has a partially used block at each edge. Real game textures are powers of two; rather
            // than write an edge case nothing here can test, hand those to the framework.
            if (levelWidth <= 0 || levelHeight <= 0 || (levelWidth % 4) != 0 || (levelHeight % 4) != 0) {
                return false;
            }

            const std::size_t blocksX = static_cast<std::size_t>(levelWidth) / 4u;
            const std::size_t blocksY = static_cast<std::size_t>(levelHeight) / 4u;

            // A truncated file would otherwise be read past its end. 16 bytes is BC7's block size.
            if (static_cast<std::size_t>(level->m_dwLinearSize) < blocksX * blocksY * 16u) {
                return false;
            }

            pixels.assign(static_cast<std::size_t>(levelWidth) * levelHeight * 4u, 0u);

            const std::uint8_t *blocks = level->m_pbData;
            std::uint8_t *out = pixels.data();

            // ONE BLOCK BEFORE ANY THREAD STARTS, and it is not a cache warm-up.
            //
            // DecompressBlockBC7 with no options runs init_BC7ramps(), which guards on a plain
            // non-atomic static and fills a global table. Two threads arriving together both see it
            // unset and both write -- a data race that happens to produce the right answer only
            // because they write identical values. Decoding one block here forces that
            // initialisation to happen exactly once, under call_once, so every call afterwards
            // only reads it. call_once rather than a bare call because it also covers two
            // TextureFile::open calls racing each other, which nothing does today and which would
            // otherwise be a silent trap for whoever first decodes textures in parallel.
            //
            // Passing a CreateOptionsBC7 object instead would also avoid the race, but that hands
            // the decoder a differently-initialised struct, and the bytes were only ever verified
            // for this call.
            static std::once_flag rampsFlag;
            std::call_once(rampsFlag, [blocks]() {
                std::array<std::uint8_t, 64> first{};
                DecompressBlockBC7(blocks, first.data(), nullptr);
            });

            unsigned hardware = std::thread::hardware_concurrency();
            if (hardware == 0) {
                hardware = 1;
            }

            // Enough block rows each to be worth starting a thread for. A small texture decodes in
            // under a millisecond, which is the same order as the spawn.
            constexpr std::size_t MinRowsPerThread = 16u;
            const std::size_t useful = std::max<std::size_t>(blocksY / MinRowsPerThread, 1u);
            const unsigned threads =
                static_cast<unsigned>(std::min<std::size_t>(hardware, useful));

            if (threads <= 1) {
                decodeBc7Rows(blocks, out, levelWidth, static_cast<int>(blocksX), 0,
                              static_cast<int>(blocksY));
            } else {
                std::vector<std::thread> pool;
                pool.reserve(threads);

                const std::size_t perThread = (blocksY + threads - 1u) / threads;

                for (unsigned worker = 0; worker < threads; ++worker) {
                    const std::size_t rowStart = worker * perThread;
                    const std::size_t rowEnd = std::min(rowStart + perThread, blocksY);
                    if (rowStart >= rowEnd) {
                        break;
                    }

                    pool.emplace_back(decodeBc7Rows, blocks, out, levelWidth,
                                      static_cast<int>(blocksX), static_cast<int>(rowStart),
                                      static_cast<int>(rowEnd));
                }

                for (std::thread &worker : pool) {
                    worker.join();
                }
            }

            width = levelWidth;
            height = levelHeight;
            return true;
        }

        void ensureFrameworkInit() {
            static std::once_flag flag;
            std::call_once(flag, []() {
                CMP_InitFramework();
            });
        }
    }

    TextureFile::TextureFile(std::string src): src_(std::move(src)) {}

    const std::string& TextureFile::getSrc() const {
        return src_;
    }

    void TextureFile::setSrc(std::string src) {
        src_ = std::move(src);
    }

    bool TextureFile::hasImage() const {
        return hasImage_;
    }

    int TextureFile::getWidth() const {
        return width_;
    }

    int TextureFile::getHeight() const {
        return height_;
    }

    std::optional<double> TextureFile::getGamma() const {
        return gamma_;
    }

    void TextureFile::setGamma(std::optional<double> gamma) {
        gamma_ = gamma;
    }

    const std::vector<std::uint8_t>& TextureFile::getPixels() const {
        return pixels_;
    }

    void TextureFile::setPixels(std::vector<std::uint8_t> pixels, int width, int height) {
        pixels_ = std::move(pixels);
        width_ = width;
        height_ = height;
        // #hasImage previously was only ever set true by #open -- meaning a TextureFile that got
        // its pixel data via #setPixels alone (eg. TexCreator.fix, or a Python-side save() call
        // with no prior open()) reported hasImage() == false despite holding real, valid pixel
        // data. #setPixels is the general "this object now holds real pixel data" seam, so it's
        // the right place for this, not just #open.
        hasImage_ = !pixels_.empty();
    }

    Colour TextureFile::getPixel(int x, int y) const {
        std::size_t i = (static_cast<std::size_t>(y) * width_ + x) * 4;
        return Colour(pixels_[i], pixels_[i + 1], pixels_[i + 2], pixels_[i + 3]);
    }

    void TextureFile::setPixel(int x, int y, const Colour &colour) {
        std::size_t i = (static_cast<std::size_t>(y) * width_ + x) * 4;
        pixels_[i] = static_cast<std::uint8_t>(colour.red);
        pixels_[i + 1] = static_cast<std::uint8_t>(colour.green);
        pixels_[i + 2] = static_cast<std::uint8_t>(colour.blue);
        pixels_[i + 3] = static_cast<std::uint8_t>(colour.alpha);
    }

    void TextureFile::setCache(TexCache *cache) {
        cache_ = cache;
    }

    TexCache *TextureFile::getCache() const {
        return cache_;
    }

    std::optional<Hash128> TextureFile::hashSource() const {
        std::ifstream in(FileService::strToPath(src_), std::ios::binary);
        if (!in) {
            return std::nullopt;
        }

        // Read whole rather than streamed: these are a few MB, the hash is only worth doing at all
        // because it replaces a ~0.20s decode, and XXH3 has no incremental wrapper in this repo.
        std::vector<char> bytes((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        if (!in && !in.eof()) {
            return std::nullopt;
        }

        return Hash128::hash(bytes.data(), bytes.size());
    }

    Hash128 TextureFile::writeKey(bool compress, bool mipmaps) const {
        // The pixels decide most of it, but two writes of the SAME pixels still differ when the
        // format or either flag differs -- an uncompressed write and a BC7 one of one image are
        // not interchangeable files.
        const Hash128 pixelHash = Hash128::hash(pixels_.data(), pixels_.size());

        std::string key = pixelHash.toHexString();
        key += "|" + std::to_string(width_);
        key += "|" + std::to_string(height_);
        key += "|" + std::to_string(static_cast<int>(format_));
        key += "|" + std::to_string(compress ? 1 : 0);
        key += "|" + std::to_string(mipmaps ? 1 : 0);

        return Hash128::hash(key);
    }

    bool TextureFile::writeUncompressedDds(const std::string &dest) const {
        if (pixels_.empty() || width_ <= 0 || height_ <= 0) {
            return false;
        }

        const std::size_t expected = static_cast<std::size_t>(width_) * static_cast<std::size_t>(height_) * 4u;
        if (pixels_.size() < expected) {
            return false;
        }

        // The exact header CMP_SaveTexture wrote for this case, field for field: a legacy
        // 128-byte DDS header with no DX10 block. The masks are the load-bearing part -- they say
        // BGRA, which is why the payload below is swizzled rather than copied.
        std::array<std::uint8_t, 128> header{};

        auto put32 = [&header](std::size_t at, std::uint32_t value) {
            header[at] = static_cast<std::uint8_t>(value & 0xFFu);
            header[at + 1] = static_cast<std::uint8_t>((value >> 8) & 0xFFu);
            header[at + 2] = static_cast<std::uint8_t>((value >> 16) & 0xFFu);
            header[at + 3] = static_cast<std::uint8_t>((value >> 24) & 0xFFu);
        };

        header[0] = 'D';
        header[1] = 'D';
        header[2] = 'S';
        header[3] = ' ';

        put32(4, 124u);                                     // dwSize
        put32(8, 0x0002100Fu);                              // CAPS|HEIGHT|WIDTH|PITCH|PIXELFORMAT|MIPMAPCOUNT
        put32(12, static_cast<std::uint32_t>(height_));
        put32(16, static_cast<std::uint32_t>(width_));
        put32(20, static_cast<std::uint32_t>(width_) * 4u); // dwPitchOrLinearSize
        put32(24, 0u);                                      // dwDepth
        put32(28, 1u);                                      // dwMipMapCount -- this writer is the single-level case
        // dwReserved1[11] at 32..75 stays zero

        put32(76, 32u);                                     // ddspf.dwSize
        put32(80, 0x41u);                                   // DDPF_ALPHAPIXELS | DDPF_RGB
        put32(84, 0u);                                      // ddspf.dwFourCC -- none, this is uncompressed
        put32(88, 32u);                                     // ddspf.dwRGBBitCount
        put32(92, 0x00FF0000u);                             // red
        put32(96, 0x0000FF00u);                             // green
        put32(100, 0x000000FFu);                            // blue
        put32(104, 0xFF000000u);                            // alpha
        put32(108, 0x1000u);                                // dwCaps = DDSCAPS_TEXTURE
        // dwCaps2/3/4 and dwReserved2 at 112..127 stay zero

        std::ofstream out(FileService::strToPath(dest), std::ios::binary | std::ios::trunc);
        if (!out) {
            return false;
        }

        out.write(reinterpret_cast<const char*>(header.data()), static_cast<std::streamsize>(header.size()));

        // Swizzled a chunk at a time rather than into a second full-size buffer: a 4096x4096
        // texture is 64MB decoded, and this runs on a 16GB laptop with the game often open.
        constexpr std::size_t ChunkPixels = 1u << 16;
        std::vector<std::uint8_t> chunk(ChunkPixels * 4u);

        for (std::size_t pixel = 0; pixel < expected / 4u && out; pixel += ChunkPixels) {
            const std::size_t count = std::min(ChunkPixels, (expected / 4u) - pixel);

            for (std::size_t i = 0; i < count; ++i) {
                const std::uint8_t *src = pixels_.data() + ((pixel + i) * 4u);
                std::uint8_t *dst = chunk.data() + (i * 4u);
                dst[0] = src[2];    // blue
                dst[1] = src[1];    // green
                dst[2] = src[0];    // red
                dst[3] = src[3];    // alpha
            }

            out.write(reinterpret_cast<const char*>(chunk.data()), static_cast<std::streamsize>(count * 4u));
        }

        out.flush();
        return static_cast<bool>(out);
    }

    void TextureFile::open() {
        std::error_code ec;
        if (!std::filesystem::is_regular_file(FileService::strToPath(src_), ec)) {
            hasImage_ = false;
            pixels_.clear();
            width_ = 0;
            height_ = 0;
            return;
        }

        // Keyed on the file's CONTENT, and asked before any Compressonator work happens. Decoding
        // is a pure function of those bytes -- no filter has run yet at this point -- so this half
        // of the cache needs no assumption whatsoever about what the edit is going to do.
        std::optional<Hash128> sourceHash;
        if (cache_ != nullptr) {
            sourceHash = hashSource();

            if (sourceHash.has_value()) {
                if (const TexCache::Decoded *hit = cache_->decoded(*sourceHash)) {
                    pixels_ = hit->pixels;
                    width_ = hit->width;
                    height_ = hit->height;
                    format_ = hit->format;

                    // Restored with the pixels, never skipped: save() runs a GammaFilter when this
                    // is set, so a hit that left it alone would apply the LAST source's gamma to
                    // this one -- see TexCache::Decoded::gamma.
                    gamma_ = hit->gamma;

                    hasImage_ = true;
                    return;
                }
            }
        }

        ensureFrameworkInit();

        // Read through an ASCII scratch copy when this path is not one Compressonator can
        // take -- see the note by isAsciiPath. std::filesystem::copy_file is unicode-safe, so
        // the non-ASCII half of the journey is made by the standard library rather than by a
        // narrow C API.
        std::string loadPath = src_;
        std::optional<std::filesystem::path> loadScratch;
        if (!isAsciiPath(src_)) {
            loadScratch = asciiTempPath(src_);
            if (loadScratch.has_value()) {
                std::error_code copyEc;
                std::filesystem::copy_file(FileService::strToPath(src_), *loadScratch,
                                            std::filesystem::copy_options::overwrite_existing, copyEc);
                if (copyEc) {
                    loadScratch.reset();
                } else {
                    loadPath = FileService::pathToStr(*loadScratch);
                }
            }
        }

        CMP_MipSet mipSetIn{};
        const CMP_ERROR loadStatus = CMP_LoadTexture(loadPath.c_str(), &mipSetIn);

        if (loadScratch.has_value()) {
            std::error_code rmEc;
            std::filesystem::remove(*loadScratch, rmEc);
        }

        if (loadStatus != CMP_OK) {
            hasImage_ = false;
            pixels_.clear();
            width_ = 0;
            height_ = 0;
            return;
        }

        // THE DX10 HEADER IS READ FOR EVERY TEXTURE, not only the ones that failed to map, and
        // that is the fix for a bug this file shipped with: the two facts it carries are
        // independent. The FORMAT is only needed when Compressonator could not work it out
        // (CMP_FORMAT_Unknown); the sRGB BIT matters just as much for a BCn texture it mapped
        // perfectly well.
        const std::optional<DX10Format> headerFormat = dx10Format(src_);

        if (headerFormat.has_value()) {
            // Naming the format has to happen BEFORE format_ is taken, since save() uses format_
            // as the format to write back out and Unknown is not one.
            if (mipSetIn.m_format == CMP_FORMAT_Unknown && headerFormat->format != CMP_FORMAT_Unknown) {
                mipSetIn.m_format = headerFormat->format;
            }

            // AND THE sRGB PRE-CORRECTION, which is the half a reader is most likely to
            // think is somebody else's job.
            //
            // save() writes the edited texture back UNTAGGED -- plain 32-bit unorm, no
            // DX10 header -- so the shader samples the new file WITHOUT the sRGB-to-linear
            // transform the source was written to be read through, and the remapped
            // character renders visibly brighter than the mod does on its own model.
            // Baking the transform into the values is what keeps the two looking the same,
            // and it is what the pure-Python script does here (measured on Keqing's dress
            // diffuse: every distinct source value maps to round(255 * (v/255) ** 2.2),
            // with zero disagreements over the whole texture).
            //
            // As METADATA rather than a pixel pass, for the reason DarkDiffuse gives: it
            // belongs immediately before the encode, not before the fix's own filters, so
            // a filter matching on colour still sees the values the texture actually holds.
            //
            // WHATEVER THE COMPRESSION. This used to be inside the format branch above, on
            // the inference that BCn sRGB was already handled because Ganyu's DarkDiffuse
            // A/B'd byte-identical -- but that edit DECLARES setGamma(1 / 2.2) by hand, so it
            // matched for a different reason. Xiangling's and HuTao's head diffuse edits
            // declare no gamma, their diffuses are BC7_UNORM_SRGB, and the old script corrects
            // them anyway: 52/66/102 comes out 8/13/34. In game the difference is pale hair
            // (Images/Xiangling/XianglingCheerPaleHair.jpg).
            //
            // A fix that declares its own gamma still wins, since setGamma overwrites -- which
            // is what keeps Ganyu unchanged and lets CherryHuTao's deliberate setGamma(1) go on
            // disabling the correction for her lightmap.
            //
            // WHY THIS IS NOT A PILLOW WORKAROUND, though the pure-Python script is where it comes
            // from (nhok0169/Anime-Game-Remap#102). Pillow could not write an sRGB .dds, so the
            // script baked the transform into the values instead -- and Compressonator cannot
            // either. Its format enum has sRGB entries for ETC2 alone: one CMP_FORMAT_BC7 covers
            // both DXGI 98 and 99, so the bit is lost on load and save() has no sRGB format to ask
            // for. Measured on a BC7_UNORM_SRGB source: save(compress = true) writes DXGI 98 and
            // save(compress = false) writes a legacy header, both LINEAR. So --compressTextures
            // changes the size of the output, not its colour space, and does not remove the need
            // for this.
            //
            // The real fix would be to write the DX10 header ourselves for these formats -- the
            // reader above already knows which DXGI format the source was. That stops a 2.2 power
            // being baked into 8-bit values, which costs most of the low end (52 -> 8). It also
            // diverges from the old script byte for byte, so it wants its own in-game check.
            if (headerFormat->srgb) {
                gamma_ = SrgbGamma;
            }
        }

        format_ = mipSetIn.m_format;

        // ONLY MIP 0, and this is load-bearing rather than an optimization.
        //
        // CMP_ConvertMipTexture fails outright -- CMP_ERR_INVALID_SOURCE_TEXTURE (2) -- on a mip
        // CHAIN, while succeeding on the identical texture with a single level. Measured on a
        // 2048x2048 BC7 face diffuse: as loaded (12 levels) convert returns 2 and nothing is
        // decoded; with this one assignment it returns CMP_OK and mip 0 comes out intact. The same
        // file with its header rewritten to say mipMapCount = 1 also converts cleanly, which is
        // what pinned the cause on the chain rather than on the file.
        //
        // Left unfixed this made open() report "no image" for most REAL mod textures -- game
        // textures ship with full mip chains, and only the small or hand-made ones do not -- and
        // every caller downstream treats that as "nothing to do" and silently does nothing. The
        // texture edits are the visible casualty: TexEditor::fix bails on !hasImage(), so a face
        // diffuse fix logged "Editting texture for ..." and wrote no file at all.
        //
        // Nothing is leaked by truncating here: CMP_CMIPS::FreeMipSet frees by m_nMaxMipLevels,
        // which is untouched, so all 12 levels are still released. This class only ever reads
        // level 0 anyway (see CMP_GetMipLevel below), and save() writes a single level back.
        mipSetIn.m_nMipLevels = 1;

        // Everything a successful decode still owes the caller, whichever way the pixels arrived.
        // One copy rather than two: the cache being wired into only one of two places is exactly
        // the bug that left every grouped texture uncached while the summary looked right.
        auto finishDecode = [this, &sourceHash]() {
            hasImage_ = true;

            if (cache_ != nullptr && sourceHash.has_value()) {
                cache_->rememberDecoded(*sourceHash,
                                        TexCache::Decoded{pixels_, width_, height_, format_, gamma_});
            }
        };

        // Ours if this is BC7, the framework's otherwise -- see decodeBc7Level.
        if (decodeBc7Level(mipSetIn, pixels_, width_, height_)) {
            CMP_FreeMipSet(&mipSetIn);
            finishDecode();
            return;
        }

        CMP_CompressOptions options{};
        options.dwSize = sizeof(options);
        options.DestFormat = CMP_FORMAT_RGBA_8888;
        options.fquality = 1.0f;
        options.dwnumThreads = 0;

        CMP_MipSet mipSetRGBA{};
        CMP_ERROR status = CMP_ConvertMipTexture(&mipSetIn, &mipSetRGBA, &options, nullptr);
        CMP_FreeMipSet(&mipSetIn);

        if (status != CMP_OK) {
            hasImage_ = false;
            pixels_.clear();
            width_ = 0;
            height_ = 0;
            return;
        }

        CMP_MipLevel *level = nullptr;
        CMP_GetMipLevel(&level, &mipSetRGBA, 0, 0);
        if (level == nullptr || level->m_pbData == nullptr) {
            CMP_FreeMipSet(&mipSetRGBA);
            hasImage_ = false;
            pixels_.clear();
            width_ = 0;
            height_ = 0;
            return;
        }

        width_ = level->m_nWidth;
        height_ = level->m_nHeight;
        pixels_.assign(level->m_pbData, level->m_pbData + (static_cast<std::size_t>(width_) * height_ * 4));

        CMP_FreeMipSet(&mipSetRGBA);
        finishDecode();
    }

    void TextureFile::save(bool compress, bool mipmaps) {
        if (gamma_.has_value()) {
            GammaFilter(*gamma_).transform(*this);
        }

        // 'compress' regardless of what #getSrc's own extension happens to be -- #save's contract
        // is "write the .dds back where it came from", and every real caller's src_ is a .dds.
        // #saveAs is the extension-driven one.
        //
        // compress = false still writes a .dds; it just writes the RGBA8 buffer straight through
        // Compressonator's DDS plugin as 32-bit uncompressed, skipping the BCn encode that
        // dominates the runtime. See the header for the measured trade.
        // NOT a discarded bool. This used to be `writeTo(src_, compress);`, so a failed write
        // was invisible: a run over a mod folder with a non-ASCII name reported "editted 18
        // *.dds files and skipped 0" having written none of them. A texture that cannot be
        // written is a fix that did not happen, and the caller already has somewhere to put
        // that -- the .ini file is recorded as skipped, with this message.
        if (!writeTo(src_, compress, mipmaps)) {
            throw std::runtime_error("Unable to write the edited texture: " + src_);
        }
    }

    bool TextureFile::saveAs(const std::string &dest) const {
        // Compressonator picks its writer off the file extension, and only registers a real plugin
        // for DDS -- everything else falls through to its own built-in stb writers (.png/.bmp/.jpg),
        // which take the uncompressed RGBA8 buffer directly. So "compress" here is exactly
        // "is this a .dds".
        return writeTo(dest, StringTools::endsWithIgnoreCase(dest, ".dds"));
    }

    bool TextureFile::writeTo(const std::string &dest, bool compress, bool mipmaps) const {
        if (pixels_.empty() || width_ <= 0 || height_ <= 0) {
            return false;
        }

        // Asked AFTER every filter has already run, on the pixels they produced -- so this costs
        // nothing in correctness no matter what the chain did or what else it read. All it claims
        // is that writing identical pixels with identical settings produces an identical file,
        // and so the second one can be a copy.
        std::optional<Hash128> outputKey;
        if (cache_ != nullptr) {
            outputKey = writeKey(compress, mipmaps);

            if (std::optional<std::string> already = cache_->writtenAs(*outputKey)) {
                std::error_code copyEc;
                std::filesystem::copy_file(FileService::strToPath(*already),
                                            FileService::strToPath(dest),
                                            std::filesystem::copy_options::overwrite_existing, copyEc);
                if (!copyEc) {
                    return true;
                }

                // The remembered file has gone, or could not be read. Drop it and write properly --
                // the same self-healing DownloadCache documents for a download it remembered.
                cache_->forgetWritten(*outputKey);
            }
        }

        // Nothing here needs Compressonator: no encode, no chain, just a header and the pixels.
        // Falls through to the library path if our writer cannot do it, so this can only ever be
        // faster, never a new way to fail.
        //
        // THE EXTENSION IS PART OF THE CONDITION, and leaving it out was a real bug caught by
        // test_CppTextureFile's saveAs cases: saveAs passes compress = "does dest end in .dds",
        // so every OTHER format -- the .png/.jpg "let me actually look at this texture" path --
        // arrives here with compress = false and got a DDS written into it, magic bytes and all.
        // Compressonator picks its writer off the extension; this one only knows how to make one
        // kind of file, so it must only claim the case where that is the file wanted.
        if (!compress && !mipmaps && StringTools::endsWithIgnoreCase(dest, ".dds")
                && writeUncompressedDds(dest)) {
            if (cache_ != nullptr && outputKey.has_value()) {
                cache_->rememberWritten(*outputKey, dest);
            }

            return true;
        }

        ensureFrameworkInit();

        CMP_MipSet mipSetSrc{};
        if (CMP_CreateMipSet(&mipSetSrc, width_, height_, 1, CF_8bit, TT_2D) != CMP_OK) {
            return false;
        }
        mipSetSrc.m_format = CMP_FORMAT_RGBA_8888;

        CMP_MipLevel *srcLevel = nullptr;
        CMP_GetMipLevel(&srcLevel, &mipSetSrc, 0, 0);
        if (srcLevel != nullptr && srcLevel->m_pbData != nullptr) {
            std::memcpy(srcLevel->m_pbData, pixels_.data(), pixels_.size());
        }

        // The chain is generated on the RGBA8 source, so both the uncompressed write below and
        // the BCn encode see every level; CMP_CreateMipSet already sized m_nMaxMipLevels for the
        // full pyramid, and a 1 pixel floor means "all the way down". A failure here leaves the
        // single level in place rather than failing the write -- a texture without its chain is
        // what every save wrote before 2026-09-12, not a broken file.
        if (mipmaps && mipSetSrc.m_nMaxMipLevels > 1) {
            CMP_GenerateMIPLevels(&mipSetSrc, 1);
        }

        bool saved = false;

        if (!compress) {
            // CMP_SaveTexture's stb fallback reads MipSetIn->pData/m_nWidth/m_nHeight, all of
            // which CMP_CreateMipSet already pointed at this same mip-0 buffer for us.
            saved = saveThroughAscii(dest, &mipSetSrc);
            CMP_FreeMipSet(&mipSetSrc);

            if (saved && cache_ != nullptr && outputKey.has_value()) {
                cache_->rememberWritten(*outputKey, dest);
            }

            return saved;
        }

        CMP_CompressOptions options{};
        options.dwSize = sizeof(options);
        options.DestFormat = format_;
        // 0.1, not the 0.8 this used to be. BC7 encoding dominates the cost of fixing a texture
        // (measured on a 1024x1024 AmberFaceDiffuse.dds: 0.31s to decode, 14.3s to encode), and
        // Compressonator only actually spends that budget below its own g_qFAST_THRESHOLD: it
        // derives m_partitionSearchSize as (quality * 2) / 0.5, so ANY value >= 0.25 already runs
        // the full partition search and costs the same. A sweep over that same texture measured
        // 0.1 -> 4.6s and 0.8 -> 12.5s while the mean per-channel error stayed flat (0.323 vs
        // 0.377, both well under 1/255) and 0.1's max error was actually the LOWER of the two.
        // Compressonator's own default (AMD_CODEC_QUALITY_DEFAULT) is 0.05.
        options.fquality = 0.1f;
        options.dwnumThreads = 0;

        CMP_MipSet mipSetOut{};
        CMP_ERROR status = CMP_ConvertMipTexture(&mipSetSrc, &mipSetOut, &options, nullptr);
        CMP_FreeMipSet(&mipSetSrc);

        if (status == CMP_OK) {
            saved = saveThroughAscii(dest, &mipSetOut);
            CMP_FreeMipSet(&mipSetOut);
        }

        if (saved && cache_ != nullptr && outputKey.has_value()) {
            cache_->rememberWritten(*outputKey, dest);
        }

        return saved;
    }
}
