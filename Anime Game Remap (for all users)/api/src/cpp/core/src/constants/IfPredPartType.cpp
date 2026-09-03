#include "AGRemapCore/constants/IfPredPartType.h"

#include "AGRemapCore/tools/StringTools.h"


namespace AGRemapCore {

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
        // Whitespace and case are handled by grapheme through StringTools (Unicode whitespace is
        // stripped, non-ASCII letters are lowered) -- matching the pure-Python original's own
        // '.strip().lower()' + 'str.startswith(...)' on a Unicode str, rather than the byte-wise
        // ASCII shortcut this used to take.
        std::string cleaned = StringTools::toLower(StringTools::strip(rawPredPart));

        if (StringTools::startsWith(cleaned, getName(IfPredPartType::If))) {
            return IfPredPartType::If;
        }
        if (StringTools::startsWith(cleaned, getName(IfPredPartType::EndIf))) {
            return IfPredPartType::EndIf;
        }
        if (StringTools::startsWith(cleaned, getName(IfPredPartType::Elif))) {
            return IfPredPartType::Elif;
        }

        std::string elseKeyword = getName(IfPredPartType::Else);
        if (!StringTools::startsWith(cleaned, elseKeyword)) {
            return std::nullopt;
        }

        std::string_view rest = StringTools::lstrip(std::string_view(cleaned).substr(elseKeyword.size()));
        if (StringTools::startsWith(rest, getName(IfPredPartType::If))) {
            return IfPredPartType::Elif;
        }

        return IfPredPartType::Else;
    }
}
