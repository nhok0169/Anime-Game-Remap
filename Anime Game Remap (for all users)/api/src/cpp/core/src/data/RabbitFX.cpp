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

#include "AGRemapCore/data/RabbitFX.h"

#include <optional>
#include <unordered_map>

#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/model/files/TextureFile.h"
#include "AGRemapCore/tools/StringTools.h"


namespace AGRemapCore {

    int RabbitFX::textureTag(const std::string& path) {
        const std::optional<TextureFile::DDSHeader> header = TextureFile::readDDSHeader(path);
        if (!header.has_value()) {
            return 0;
        }
        return tagOf(header->dxgiFormat, header->width, header->height, header->mipCount);
    }

    int RabbitFX::tagOf(const std::string& format, std::uint32_t width, std::uint32_t height, std::uint32_t mips) {
        // the DXGI formats the rule tells apart, by name (the rest tag 0)
        static const std::unordered_map<std::string, std::uint32_t> formats = {
            {"bc7_typeless", 97}, {"bc7_unorm", 98}, {"bc7_unorm_srgb", 99}, {"r8_typeless", 60}, {"r8_unorm", 61},
            {"b8g8r8a8_unorm", 87}, {"b8g8r8a8_typeless", 90}, {"b8g8r8a8_unorm_srgb", 91}};
        const auto found = formats.find(StringTools::toLower(format));
        return tagOf(found == formats.end() ? 0 : found->second, width, height, mips);
    }

    int RabbitFX::tagOf(std::uint32_t dxgi, std::uint32_t width, std::uint32_t height, std::uint32_t mips) {
        if (width != height) {
            return 0;
        }
        if (dxgi >= 97 && dxgi <= 99) {                                     // BC7
            if (height > 1 && mips > 10) {
                return 3;
            }
            if (height < 513 && mips < 11) {
                return 6;
            }
            return mips == 1 ? 4 : 0;
        }
        if (dxgi == 60 || dxgi == 61) {                                     // R8
            return (height > 1 && mips > 1) ? 4 : 0;
        }
        if (dxgi == 87 || dxgi == 90 || dxgi == 91) {                       // B8G8R8A8
            if (height < 512 && mips > 1) {
                return 3;
            }
            return (height == 1 && mips == 1) ? 5 : 0;
        }
        return 0;
    }

    std::array<int, 4> RabbitFX::routingFlags(const std::array<int, 4>& tags) {
        const int full = (tags[0] == 6 || tags[1] == 6) ? 6 : 3;
        std::array<int, 4> flags{tags[0] == full ? 1 : 0, tags[1] == full ? 1 : 0,
                                 tags[2] == full ? 1 : 0, tags[2] == 4 ? 1 : 0};
        if (tags[1] == 5) {
            if (tags[2] == full) {
                flags[1] = 1;
            } else if (tags[3] == full) {
                flags[2] = 1;
            }
        }
        return flags;
    }

    bool RabbitFX::names(const std::string& text) {
        return StringTools::toLower(text).find("rabbitfx") != std::string::npos;
    }

    bool RabbitFX::isOwnKVP(const std::string& key, const std::string& value) {
        const std::string lowerKey = StringTools::toLower(key);
        return StringTools::startsWith(lowerKey, StringTools::toLower(IniKeywords::RabbitFXResourceFolder) + "\\")
               || (lowerKey == IniKeywords::Run && names(value));
    }
}
