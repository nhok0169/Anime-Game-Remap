#include "AGRemapCore/model/VGRemap.h"

#include <algorithm>
#include <utility>


namespace AGRemapCore {
    VGRemap::VGRemap(std::unordered_map<long long, long long> remap) {
        setRemap(std::move(remap));
    }

    const std::unordered_map<long long, long long>& VGRemap::getRemap() const {
        return remap_;
    }

    void VGRemap::setRemap(std::unordered_map<long long, long long> remap) {
        remap_ = std::move(remap);

        if (remap_.empty()) {
            maxIndex_ = std::nullopt;
            return;
        }

        long long maxIndex = remap_.begin()->first;
        for (const auto& [key, value] : remap_) {
            maxIndex = std::max(maxIndex, key);
        }
        maxIndex_ = maxIndex;
    }

    std::optional<long long> VGRemap::getMaxIndex() const {
        return maxIndex_;
    }
}
