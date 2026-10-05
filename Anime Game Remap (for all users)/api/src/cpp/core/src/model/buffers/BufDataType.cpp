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

#include "AGRemapCore/model/buffers/BufDataType.h"

#include <stdexcept>
#include <utility>


namespace AGRemapCore {
    void BufDataType::validateSize(std::size_t size) {
        if (size == 0 || size > 8) {
            throw std::invalid_argument("BufDataType size must be between 1 and 8 bytes, got " + std::to_string(size));
        }
    }

    BufDataType::BufDataType(std::string name, std::size_t size, bool isBigEndian):
        BufType(std::move(name)), size_(0), isBigEndian_(isBigEndian) {

        setSize(size);
    }

    std::size_t BufDataType::getSize() const {
        return size_;
    }

    void BufDataType::setSize(std::size_t size) {
        validateSize(size);
        size_ = size;
    }

    bool BufDataType::getIsBigEndian() const {
        return isBigEndian_;
    }

    void BufDataType::setIsBigEndian(bool isBigEndian) {
        isBigEndian_ = isBigEndian;
    }
}
