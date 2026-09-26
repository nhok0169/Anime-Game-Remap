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


#include "AGRemapCore/model/textures/TexThumbprint.h"

#include <cmath>
#include <exception>

#include "AGRemapCore/model/files/TextureFile.h"


namespace AGRemapCore {

    std::optional<std::vector<double>> TexThumbprint::of(const std::string& path, int size) {
        TextureFile texture(path);
        try {
            texture.open();
        } catch (const std::exception&) {
            return std::nullopt;
        }

        const int width = texture.getWidth();
        const int height = texture.getHeight();
        if (!texture.hasImage() || size <= 0 || width < size || height < size) {
            return std::nullopt;
        }

        // The same arithmetic as Tools/Misc/Diagnostics/wwmiTextureThumbs.py, which is what
        // generates the tables the fixers carry -- keep the two in step
        const std::vector<std::uint8_t>& pixels = texture.getPixels();
        const int bw = width / size;
        const int bh = height / size;
        std::vector<double> thumb(static_cast<std::size_t>(size) * static_cast<std::size_t>(size), 0.0);
        for (int by = 0; by < size; ++by) {
            for (int bx = 0; bx < size; ++bx) {
                double sum = 0.0;
                for (int y = by * bh; y < (by + 1) * bh; ++y) {
                    const std::uint8_t* row = pixels.data()
                        + (static_cast<std::size_t>(y) * static_cast<std::size_t>(width)
                           + static_cast<std::size_t>(bx) * static_cast<std::size_t>(bw)) * 4;
                    for (int x = 0; x < bw; ++x) {
                        sum += (static_cast<double>(row[x * 4]) + static_cast<double>(row[x * 4 + 1])
                                + static_cast<double>(row[x * 4 + 2])) / 3.0;
                    }
                }

                thumb[static_cast<std::size_t>(by) * static_cast<std::size_t>(size) + static_cast<std::size_t>(bx)]
                    = std::round(sum / (static_cast<double>(bw) * static_cast<double>(bh)));
            }
        }

        return thumb;
    }


    double TexThumbprint::correlation(const std::vector<double>& thumb, const std::vector<std::uint8_t>& stored) {
        if (thumb.size() != stored.size() || thumb.empty()) {
            return 0.0;
        }

        double meanA = 0.0;
        double meanB = 0.0;
        for (std::size_t i = 0; i < thumb.size(); ++i) {
            meanA += thumb[i];
            meanB += static_cast<double>(stored[i]);
        }

        meanA /= static_cast<double>(thumb.size());
        meanB /= static_cast<double>(thumb.size());
        double dot = 0.0;
        double normA = 0.0;
        double normB = 0.0;
        for (std::size_t i = 0; i < thumb.size(); ++i) {
            const double da = thumb[i] - meanA;
            const double db = static_cast<double>(stored[i]) - meanB;
            dot += da * db;
            normA += da * da;
            normB += db * db;
        }

        const double norm = std::sqrt(normA) * std::sqrt(normB);
        return norm > 0.0 ? dot / norm : 0.0;
    }


    std::optional<std::string> TexThumbprint::identify(const std::vector<double>& thumb, const Table& table,
                                                       double minScore, double maxRunnerUp) {
        if (table.empty()) {
            return std::nullopt;
        }

        // -2 is below any correlation, so the first entry always takes the lead
        std::string best;
        double bestScore = -2.0;
        double secondScore = -2.0;
        for (const auto& entry : table) {
            const double score = correlation(thumb, entry.second);
            if (score > bestScore) {
                secondScore = bestScore;
                bestScore = score;
                best = entry.first;
            } else if (score > secondScore) {
                secondScore = score;
            }
        }

        if (bestScore >= minScore && secondScore < maxRunnerUp) {
            return best;
        }

        return std::nullopt;
    }


    bool TexThumbprint::channelIsConstant(const std::string& path, int channel) {
        if (channel < 0 || channel > 3) {
            return false;
        }

        TextureFile texture(path);
        try {
            texture.open();
        } catch (const std::exception&) {
            return false;
        }

        if (!texture.hasImage()) {
            return false;
        }

        const std::vector<std::uint8_t>& pixels = texture.getPixels();
        if (pixels.size() < 4) {
            return false;
        }

        const std::uint8_t first = pixels[static_cast<std::size_t>(channel)];
        for (std::size_t i = static_cast<std::size_t>(channel); i < pixels.size(); i += 4) {
            if (pixels[i] != first) {
                return false;
            }
        }

        return true;
    }


    std::optional<std::string> TexThumbprint::identifyFile(const std::string& path, const Table& table, int size,
                                                           double minScore, double maxRunnerUp) {
        if (table.empty()) {
            return std::nullopt;
        }

        std::optional<std::vector<double>> thumb = of(path, size);
        if (!thumb.has_value()) {
            return std::nullopt;
        }

        return identify(*thumb, table, minScore, maxRunnerUp);
    }
}
