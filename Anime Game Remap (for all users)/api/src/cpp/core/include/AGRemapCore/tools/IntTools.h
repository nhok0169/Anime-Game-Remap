#ifndef AGRemapCore_IntTools_H
#define AGRemapCore_IntTools_H

#include <vector>
#include <string>
#include <optional>
#include <tuple>
#include <algorithm>

#define NEGATIVE_STR "-"


namespace AGRemapCore {
    class IntTools {
        public:
            static std::vector<unsigned int> toBase(long long num, unsigned int base, bool *isNegative, bool *error);
            static std::string toStrBase(long long num, unsigned int base, const std::vector<std::string>& getDigit, const std::string& negativeChar, bool *error);
            static std::string toBase64(long long num, bool *error, const std::optional<std::vector<std::string>>& getDigit = std::nullopt, const std::string& negativeChar = NEGATIVE_STR);
    };
}

#endif