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

#include "AGRemapCore/model/buffers/BufType.h"

#include <utility>


namespace AGRemapCore {
    BufType::BufType(std::string name): name_(std::move(name)) {}

    const std::string& BufType::getName() const {
        return name_;
    }

    void BufType::setName(std::string name) {
        name_ = std::move(name);
    }
}
