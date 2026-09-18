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

#include "AGRemapCore/tools/parsing/Token.h"


namespace AGRemapCore {
    Token::Token(std::optional<std::string> type, std::string val, size_t lineNo, size_t charNo):
        type(std::move(type)), val(std::move(val)), lineNo(lineNo), charNo(charNo) {

    }
}
