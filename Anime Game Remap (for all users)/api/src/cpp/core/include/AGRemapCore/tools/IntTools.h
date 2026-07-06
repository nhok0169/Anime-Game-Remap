#ifndef AGRemapCore_IntTools_H
#define AGRemapCore_IntTools_H

#include <vector>
#include <string>
#include <optional>
#include <tuple>
#include <algorithm>

#define NEGATIVE_STR "-"


namespace AGRemapCore {
    /**
     * @brief Tools for handling integers
     */
    class IntTools {
        public:

            /**
             * @brief Converts a base 10 number to an arbitrary base number
             * 
             * @param num The base 10 number to convert
             * @param base The base to convert to
             * @param isNegative A resultant pointer that indicates whether the result is a negative number
             * @param error A resultant pointer that indicates whether there was an error in converting the number
             * 
             * @return The digits in the converted number
             */
            static std::vector<unsigned int> toBase(long long num, unsigned int base, bool *isNegative, bool *error);

            /**
             * @brief Converts a base 10 number to an arbitrary base number, such that the characters in this arbitrary based number
             * are all characters
             * 
             * @param num The base 10 number to convert
             * @param base The base to convert to
             * @param getDigit The string representations of each digit. Each element is the string representation
             * of the digit at the particular index of the list.
             * @param negativeChar The character representation for the negative symbol
             * @param error A resultant pointer that indicates whether there was an error in converting the number
             * 
             * @return The converted string representation of the arbitrary base number
             */
            static std::string toStrBase(long long num, unsigned int base, const std::vector<std::string>& getDigit, const std::string& negativeChar, bool *error);

            /**
             * @brief Converts a base 10 number to a base 64 number
             * 
             * @param num The base 10 number to convert
             * @param error A resultant pointer that indicates whether there was an error in converting the number
             * @param getDigit
             @rst
             how to get the string representation of a digit. :raw-html:`<br />` :raw-html:`<br />`

             * If this argument is a list, each element is the string representation of the digit at the particular index of the string/list.
             * If this argument is ``None``, then will use the following string for each digit:

             ``ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+_``

             This is the same digit representation as the `standard base 64`_ except that the 63rd digit (``/``) is replaced with the ``_`` character
             @endrst
             * @param negativeChar The character representation for the negative symbol
             *
             * @return The converted string representation of the arbitrary base 64 number
             */
            static std::string toBase64(long long num, bool *error, const std::optional<std::vector<std::string>>& getDigit = std::nullopt, const std::string& negativeChar = NEGATIVE_STR);
    };
}

#endif