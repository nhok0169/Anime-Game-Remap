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

#include "AGRemapCore/tools/hashing/HashInt.h"


namespace AGRemapCore {
    namespace detail {
        // Standard base64 alphabet, but with the 63rd digit ('/') replaced by '_' -- matches
        // the alphabet IntTools::toBase64 uses, so this project has one consistent notion of
        // "base64".
        static const char Base64Digits[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+_";

        std::string encodeBase64(const std::uint8_t *bytes, std::size_t size) {
            std::string result;
            result.reserve((size * 8 + 5) / 6);

            unsigned int buffer = 0;
            int bitsInBuffer = 0;

            for (std::size_t i = 0; i < size; i++) {
                buffer = (buffer << 8) | bytes[i];
                bitsInBuffer += 8;

                while (bitsInBuffer >= 6) {
                    bitsInBuffer -= 6;
                    result += Base64Digits[(buffer >> bitsInBuffer) & 0x3F];
                }
            }

            if (bitsInBuffer > 0) {
                result += Base64Digits[(buffer << (6 - bitsInBuffer)) & 0x3F];
            }

            return result;
        }
    }
}
