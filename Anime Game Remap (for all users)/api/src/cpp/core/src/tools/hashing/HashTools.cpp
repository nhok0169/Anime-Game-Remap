#include "AGRemapCore/tools/hashing/HashTools.h"

#include "AGRemapCore/tools/IntTools.h"


namespace AGRemapCore {
    std::unordered_map<std::uint64_t, std::uint64_t> HashTools::shortHashFrequency_;

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

    void HashTools::clear() {
        // The single reset point for every piece of internal state this class accumulates --
        // add any future saved state's own .clear() here too, alongside shortHashFrequency_'s.
        shortHashFrequency_.clear();
    }
}
