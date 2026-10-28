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

#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/model/files/TextureFile.h"
#include "AGRemapCore/tools/StringTools.h"


namespace AGRemapCore {

    int RabbitFX::textureTag(const std::string& path) {
        const std::optional<TextureFile::DDSHeader> header = TextureFile::readDDSHeader(path);
        if (!header.has_value() || header->width != header->height) {
            return 0;
        }

        const std::uint32_t dxgi = header->dxgiFormat;
        const std::uint32_t height = header->height;
        const std::uint32_t mips = header->mipCount;
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
