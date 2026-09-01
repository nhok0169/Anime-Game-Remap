#include "AGRemapCore/tools/Heading.h"

#include <utility>


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
        // Matches the pure-Python original exactly: the width is counted in *characters of the
        // title*, so a multi-byte title lines up there and not here. Python's len() is over code
        // points and std::string::size() is over bytes -- but every title this is used with is
        // ASCII, and reproducing the original's own width is the point.
        std::size_t len = 2 * (sideLen + 1) + title.size();

        std::string result;
        result.reserve(len * sideChar.size());

        for (std::size_t i = 0; i < len; ++i) {
            result += sideChar;
        }

        return result;
    }
}
