#include "AGRemapCore/tools/files/FileService.h"
#include "AGRemapCore/model/files/TextureFile.h"

#include <cstring>
#include <filesystem>
#include <mutex>
#include <utility>

#include "AGRemapCore/model/strategies/texEditors/texFilters/GammaFilter.h"
#include "AGRemapCore/tools/StringTools.h"

namespace AGRemapCore {

    namespace {
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

    void TextureFile::open() {
        std::error_code ec;
        if (!std::filesystem::is_regular_file(FileService::strToPath(src_), ec)) {
            hasImage_ = false;
            pixels_.clear();
            width_ = 0;
            height_ = 0;
            return;
        }

        ensureFrameworkInit();

        CMP_MipSet mipSetIn{};
        if (CMP_LoadTexture(src_.c_str(), &mipSetIn) != CMP_OK) {
            hasImage_ = false;
            pixels_.clear();
            width_ = 0;
            height_ = 0;
            return;
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
        hasImage_ = true;
    }

    void TextureFile::save(bool compress) {
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
        writeTo(src_, compress);
    }

    bool TextureFile::saveAs(const std::string &dest) const {
        // Compressonator picks its writer off the file extension, and only registers a real plugin
        // for DDS -- everything else falls through to its own built-in stb writers (.png/.bmp/.jpg),
        // which take the uncompressed RGBA8 buffer directly. So "compress" here is exactly
        // "is this a .dds".
        return writeTo(dest, StringTools::endsWithIgnoreCase(dest, ".dds"));
    }

    bool TextureFile::writeTo(const std::string &dest, bool compress) const {
        if (pixels_.empty() || width_ <= 0 || height_ <= 0) {
            return false;
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

        bool saved = false;

        if (!compress) {
            // CMP_SaveTexture's stb fallback reads MipSetIn->pData/m_nWidth/m_nHeight, all of
            // which CMP_CreateMipSet already pointed at this same mip-0 buffer for us.
            saved = (CMP_SaveTexture(dest.c_str(), &mipSetSrc) == CMP_OK);
            CMP_FreeMipSet(&mipSetSrc);
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
            saved = (CMP_SaveTexture(dest.c_str(), &mipSetOut) == CMP_OK);
            CMP_FreeMipSet(&mipSetOut);
        }

        return saved;
    }
}
