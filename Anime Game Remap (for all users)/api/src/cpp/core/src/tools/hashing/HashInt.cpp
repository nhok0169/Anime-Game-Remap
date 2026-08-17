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
