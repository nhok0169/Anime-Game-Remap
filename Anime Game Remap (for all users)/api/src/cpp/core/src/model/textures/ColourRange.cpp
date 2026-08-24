#include "AGRemapCore/model/textures/ColourRange.h"

namespace AGRemapCore {

    ColourRange::ColourRange(Colour min, Colour max): min(min), max(max) {}

    std::size_t ColourRange::hash() const {
        return std::hash<std::string>{}(getId());
    }

    std::string ColourRange::getId() const {
        return min.getId() + max.getId();
    }

    bool ColourRange::match(const Colour &colour) const {
        return min.red <= colour.red && colour.red <= max.red &&
               min.green <= colour.green && colour.green <= max.green &&
               min.blue <= colour.blue && colour.blue <= max.blue &&
               min.alpha <= colour.alpha && colour.alpha <= max.alpha;
    }

    bool matchesColourOrRange(const ColourOrRange &colourOrRange, const Colour &colour) {
        if (std::holds_alternative<Colour>(colourOrRange)) {
            return std::get<Colour>(colourOrRange).match(colour);
        }
        return std::get<ColourRange>(colourOrRange).match(colour);
    }
}
