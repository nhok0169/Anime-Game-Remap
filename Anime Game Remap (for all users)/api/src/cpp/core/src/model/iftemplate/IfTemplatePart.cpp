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

#include "AGRemapCore/model/iftemplate/IfTemplatePart.h"


namespace AGRemapCore {

    namespace {
        IncIdGenerator<size_t>& idGenerator() {
            static IncIdGenerator<size_t> generator(0);
            return generator;
        }
    }

    size_t IfTemplatePart::generateId() {
        size_t result;
        idGenerator().getId(result);
        return result;
    }

    IfTemplatePart::IfTemplatePart(const std::optional<size_t>& id): id_(id.has_value() ? *id : generateId()) {

    }

    size_t IfTemplatePart::id() const {
        return id_;
    }

    size_t IfTemplatePart::refreshId() {
        id_ = generateId();
        return id_;
    }

}
