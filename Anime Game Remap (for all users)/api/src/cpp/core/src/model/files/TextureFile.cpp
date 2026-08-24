#include "AGRemapCore/model/files/TextureFile.h"

#include <cstring>
#include <filesystem>
#include <mutex>
#include <utility>

#include "AGRemapCore/model/strategies/texEditors/texFilters/GammaFilter.h"

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
        if (!std::filesystem::is_regular_file(src_, ec)) {
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

    void TextureFile::save() {
        if (gamma_.has_value()) {
            GammaFilter(*gamma_).transform(*this);
        }

        ensureFrameworkInit();

        CMP_MipSet mipSetSrc{};
        if (CMP_CreateMipSet(&mipSetSrc, width_, height_, 1, CF_8bit, TT_2D) != CMP_OK) {
            return;
        }
        mipSetSrc.m_format = CMP_FORMAT_RGBA_8888;

        CMP_MipLevel *srcLevel = nullptr;
        CMP_GetMipLevel(&srcLevel, &mipSetSrc, 0, 0);
        if (srcLevel != nullptr && srcLevel->m_pbData != nullptr) {
            std::memcpy(srcLevel->m_pbData, pixels_.data(), pixels_.size());
        }

        CMP_CompressOptions options{};
        options.dwSize = sizeof(options);
        options.DestFormat = format_;
        options.fquality = 0.8f;
        options.dwnumThreads = 0;

        CMP_MipSet mipSetOut{};
        CMP_ERROR status = CMP_ConvertMipTexture(&mipSetSrc, &mipSetOut, &options, nullptr);
        CMP_FreeMipSet(&mipSetSrc);

        if (status == CMP_OK) {
            CMP_SaveTexture(src_.c_str(), &mipSetOut);
            CMP_FreeMipSet(&mipSetOut);
        }
    }
}
