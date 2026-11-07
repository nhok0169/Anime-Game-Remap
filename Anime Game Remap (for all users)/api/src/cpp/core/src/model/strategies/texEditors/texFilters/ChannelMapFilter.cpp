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

#include "AGRemapCore/model/strategies/texEditors/texFilters/ChannelMapFilter.h"

#include <optional>
#include <stdexcept>
#include <utility>

#include "AGRemapCore/model/files/TextureFile.h"

namespace AGRemapCore {

    std::vector<std::uint8_t> ChannelMapFilter::constant(std::uint8_t value) {
        return std::vector<std::uint8_t>(256, value);
    }

    ChannelMapFilter ChannelMapFilter::codeMap(std::vector<std::uint8_t> table) {
        ChannelMapFilter filter;
        filter.read = Read::ShaderCode;
        filter.data = true;
        for (std::size_t c = 0; c < 3; ++c) {
            filter.channels[c] = {0, table};
        }
        return filter;
    }

    void ChannelMapFilter::transform(TextureFile &texFile) {
        for (const Channel& channel : channels) {
            if (channel.source > 3 || (!channel.table.empty() && channel.table.size() != 256)) {
                throw std::invalid_argument("ChannelMapFilter: a channel's source must be -1 to 3 and its table 256 entries");
            }
        }

        // every source read before anything is written -- and, for a shader's reading, before the gamma goes
        const std::vector<std::uint8_t>& stored = texFile.getPixels();
        std::array<std::optional<std::vector<std::uint8_t>>, 4> sources;
        for (const Channel& channel : channels) {
            if (channel.source < 0 || sources[channel.source].has_value()) {
                continue;
            }

            if (read == Read::Raw) {
                std::vector<std::uint8_t> values(stored.size() / 4);
                for (std::size_t i = 0; i < values.size(); ++i) {
                    values[i] = stored[i * 4 + channel.source];
                }
                sources[channel.source] = std::move(values);
            } else {
                sources[channel.source] = texFile.shaderChannel(static_cast<std::size_t>(channel.source), read == Read::ShaderCode);
            }
        }

        if (data) {
            texFile.setGamma(std::nullopt);
        }

        std::vector<std::uint8_t> px = texFile.getPixels();
        for (std::size_t c = 0; c < channels.size(); ++c) {
            const Channel& channel = channels[c];
            if (channel.source < 0) {
                continue;
            }

            const std::vector<std::uint8_t>& values = *sources[channel.source];
            for (std::size_t i = 0; i < values.size() && i * 4 + c < px.size(); ++i) {
                px[i * 4 + c] = channel.table.empty() ? values[i] : channel.table[values[i]];
            }
        }

        texFile.setPixels(std::move(px), texFile.getWidth(), texFile.getHeight());
    }
}
