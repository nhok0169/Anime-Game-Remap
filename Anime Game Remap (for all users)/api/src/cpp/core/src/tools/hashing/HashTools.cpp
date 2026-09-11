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

#include "AGRemapCore/tools/hashing/HashTools.h"

#include "AGRemapCore/tools/IntTools.h"


namespace AGRemapCore {
    std::unordered_map<std::uint64_t, std::uint64_t> HashTools::shortHashFrequency_;
    std::unordered_map<std::string, std::string> HashTools::stableShortHash_;

    Hash128 HashTools::getDeterministicHash(const void *data, std::size_t size) noexcept {
        return Hash128::hash(data, size);
    }

    Hash128 HashTools::getDeterministicHash(std::string_view str) noexcept {
        return Hash128::hash(str);
    }

    std::string HashTools::getDeterministicHashStr(const void *data, std::size_t size) {
        return HashTools::getDeterministicHash(data, size).toBase64();
    }

    std::string HashTools::getDeterministicHashStr(std::string_view str) {
        return HashTools::getDeterministicHash(str).toBase64();
    }

    std::string HashTools::disambiguateShortHash(std::uint64_t shortHash) {
        bool error = false;
        std::string result = IntTools::toBase64(static_cast<long long>(shortHash), &error);

        std::uint64_t &frequency = shortHashFrequency_[shortHash];
        if (frequency > 0) {
            std::string freqStr = IntTools::toBase64(static_cast<long long>(frequency), &error);
            result += "_" + freqStr;
        }

        frequency++;
        return result;
    }

    std::string HashTools::getShortDeterministicHashStr(const void *data, std::size_t size) {
        std::uint64_t shortHash = HashTools::getDeterministicHash(data, size).getLow() % ShortHashMaxVal;
        return HashTools::disambiguateShortHash(shortHash);
    }

    std::string HashTools::getShortDeterministicHashStr(std::string_view str) {
        return HashTools::getShortDeterministicHashStr(str.data(), str.size());
    }

    std::string HashTools::getStableShortHashStr(std::string_view str) {
        // A repeat of the SAME input is served from the memo, so it never reaches
        // disambiguateShortHash and never consumes a frequency slot. A first sighting falls
        // through to the ordinary path, which is what keeps two DIFFERENT strings that collide
        // on the same short value apart.
        std::string key(str);

        auto found = stableShortHash_.find(key);
        if (found != stableShortHash_.end()) {
            return found->second;
        }

        std::string token = HashTools::getShortDeterministicHashStr(str);
        stableShortHash_.emplace(std::move(key), token);
        return token;
    }

    void HashTools::clear() {
        // The single reset point for every piece of internal state this class accumulates --
        // add any future saved state's own .clear() here too, alongside shortHashFrequency_'s.
        shortHashFrequency_.clear();
        stableShortHash_.clear();
    }
}
