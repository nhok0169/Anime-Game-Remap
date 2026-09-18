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

#include "AGRemapCore/tools/parsing/ParseContext.h"
#include "AGRemapCore/tools/StringTools.h"


namespace AGRemapCore {
    ParseContext::ParseContext(std::string src, std::optional<std::string> file, size_t startLineNo):
        file(std::move(file)), startLineNo(startLineNo) {

        // StringTools::splitlines() returns string_views into 'src' -- materialize each into an
        // owned std::string before 'src' (this constructor's own local copy of the source text)
        // goes out of scope, so 'lines' never ends up holding dangling views.
        for (std::string_view line : StringTools::splitlines(src)) {
            lines.emplace_back(line);
        }
    }

    ParseContext::ParseContext(std::vector<std::string> lines, std::optional<std::string> file, size_t startLineNo):
        lines(std::move(lines)), file(std::move(file)), startLineNo(startLineNo) {

    }

    size_t ParseContext::getEndLineNo() const {
        return startLineNo + lines.size();
    }
}
