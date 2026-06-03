#include "AGRemapCore/tools/IntTools.h"


static const unsigned int Base64BaseNum = 64;
static const std::vector<std::string> Base64Digits = {"A", "B", "C", "D", "E", "F", "G", "H", 
                                               "I", "J", "K", "L", "M", "N", "O", "P", 
                                               "Q", "R", "S", "T", "U", "V", "W", "X", 
                                               "Y", "Z", "a", "b", "c", "d", "e", "f", 
                                               "g", "h", "i", "j", "k", "l", "m", "n", 
                                               "o", "p", "q", "r", "s", "t", "u", "v", 
                                               "w", "x", "y", "z", "0", "1", "2", "3", 
                                               "4", "5", "6", "7", "8", "9", "+", "_"};


namespace AGRemapCore {
    std::vector<unsigned int> IntTools::toBase(long long num, unsigned int base, bool *isNegative, bool *error) {
        std::vector<unsigned int> digits;

        if (base <= 1) {
            *error = true;
            *isNegative = false;
            return digits;
        }

        *error = false;

        if (num == 0) {
            *isNegative = false;
            digits.push_back(0);
            return digits;
        }

        *isNegative = num < 0;
        if (*isNegative) {
            num *= -1;
        }

        unsigned int digit;

        while (num) {
            digit = num % base;
            digits.push_back(digit);
            num /= base;
        }

        std::reverse(digits.begin(), digits.end());
        return digits;
    }

    std::string IntTools::toStrBase(long long num, unsigned int base, const std::vector<std::string>& getDigit, const std::string& negativeChar, bool *error) {
        bool isNegative;
        std::string result;

        std::vector<unsigned int> digits = IntTools::toBase(num, base, &isNegative, error);
        if (*error) {
            return result;
        }

        size_t resultLen = isNegative ? negativeChar.length() : 0;

        for (auto digit: digits) {
            resultLen += getDigit[digit].length();
        }

        result.reserve(resultLen);

        if (isNegative) {
            result += negativeChar;
        }

        for (auto digit : digits) {
            result += getDigit[digit];
        }

        return result;
    }

    std::string IntTools::toBase64(long long num, bool *error, const std::optional<std::vector<std::string>>& getDigit, const std::string& negativeChar) {
        return IntTools::toStrBase(num, Base64BaseNum, getDigit.has_value() ? *getDigit : Base64Digits, negativeChar, error);
    }
}