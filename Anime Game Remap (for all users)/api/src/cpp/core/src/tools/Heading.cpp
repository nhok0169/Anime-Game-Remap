#include "AGRemapCore/tools/Heading.h"

#include <utility>

#include "AGRemapCore/tools/StringTools.h"


namespace AGRemapCore {
    Heading::Heading(std::string title, std::size_t sideLen, std::string sideChar):
        title(std::move(title)), sideLen(sideLen), sideChar(std::move(sideChar)) {}


    std::string Heading::open() const {
        std::string side;
        side.reserve(sideLen * sideChar.size());

        for (std::size_t i = 0; i < sideLen; ++i) {
            side += sideChar;
        }

        return side + " " + title + " " + side;
    }


    std::string Heading::close() const {
        // The width is counted in *graphemes* of the title (what a terminal renders as one
        // character), not bytes or code points -- so a non-ASCII title, which a
        // BaseLogger::openHeading caller can hand in from Python, still lines up with its own
        // open() line. This deliberately goes one step further than the pure-Python original's
        // len() (code points): a combining mark or an emoji ZWJ sequence is one column, not two.
        std::size_t len = 2 * (sideLen + 1) + StringTools::countGrapheme(title);

        std::string result;
        result.reserve(len * sideChar.size());

        for (std::size_t i = 0; i < len; ++i) {
            result += sideChar;
        }

        return result;
    }
}
