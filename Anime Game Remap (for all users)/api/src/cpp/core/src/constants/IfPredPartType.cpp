#include "AGRemapCore/constants/IfPredPartType.h"

#include <cctype>

#include "AGRemapCore/tools/StringTools.h"


namespace AGRemapCore {

    namespace {
        // Plain-ASCII lowercasing/prefix-matching, matching the pure-Python original's own
        // '.lower()' + 'str.startswith(...)' usage here -- these are fixed English keywords
        // ("if"/"else"/"elif"/"endif"), so no need for utf8proc's full grapheme-aware machinery.
        std::string toLowerAscii(std::string_view txt) {
            std::string result(txt);
            for (char& c : result) {
                c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            }

            return result;
        }

        bool startsWithAscii(std::string_view txt, std::string_view prefix) {
            return txt.size() >= prefix.size() && txt.substr(0, prefix.size()) == prefix;
        }

        std::string_view lstripAscii(std::string_view txt) {
            size_t start = 0;
            while (start < txt.size() && std::isspace(static_cast<unsigned char>(txt[start]))) {
                ++start;
            }

            return txt.substr(start);
        }
    }

    std::string IfPredPartTypeTools::getName(IfPredPartType value) {
        switch (value) {
            case IfPredPartType::If: return "if";
            case IfPredPartType::Else: return "else";
            case IfPredPartType::Elif: return "elif";
            case IfPredPartType::EndIf: return "endif";
        }

        return "";
    }

    std::optional<IfPredPartType> IfPredPartTypeTools::getType(const std::string& rawPredPart) {
        std::string cleaned = toLowerAscii(StringTools::strip(rawPredPart));

        if (startsWithAscii(cleaned, getName(IfPredPartType::If))) {
            return IfPredPartType::If;
        }
        if (startsWithAscii(cleaned, getName(IfPredPartType::EndIf))) {
            return IfPredPartType::EndIf;
        }
        if (startsWithAscii(cleaned, getName(IfPredPartType::Elif))) {
            return IfPredPartType::Elif;
        }
        if (!startsWithAscii(cleaned, getName(IfPredPartType::Else))) {
            return std::nullopt;
        }

        std::string_view rest = lstripAscii(std::string_view(cleaned).substr(getName(IfPredPartType::Else).size()));
        if (startsWithAscii(rest, getName(IfPredPartType::If))) {
            return IfPredPartType::Elif;
        }

        return IfPredPartType::Else;
    }
}
