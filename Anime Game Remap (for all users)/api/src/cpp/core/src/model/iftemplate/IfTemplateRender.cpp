#include "AGRemapCore/model/iftemplate/IfTemplateRender.h"

#include <vector>

#include "AGRemapCore/model/iftemplate/IfPredPart.h"


namespace AGRemapCore {

    std::string renderIfContentPart(const IfContentPart<std::string, std::string>& part,
                                     const std::string& linePrefix) {
        std::string result;

        const std::vector<std::pair<std::string, std::string>> entries = part.entries();
        const std::size_t count = entries.size();

        for (std::size_t i = 0; i < count; ++i) {
            result += linePrefix;
            result += entries[i].first;
            result += " = ";
            result += entries[i].second;

            if (i + 1 < count) {
                result += "\n";
            }
        }

        return result;
    }


    std::string renderIfTemplate(IfTemplate<std::string, std::string>& section,
                                  const std::string& linePrefix, bool autoindent) {
        std::vector<std::string> lines;
        int tabCount = 0;

        if (!section.prefix.empty()) {
            lines.push_back(section.prefix);
        }

        lines.push_back("[" + section.name + "]");

        for (const auto& part : section.parts()) {
            if (part == nullptr) {
                continue;
            }

            IfPredPart* predPart = dynamic_cast<IfPredPart*>(part.get());
            const bool isPredPart = predPart != nullptr;

            // An 'elif'/'else'/'endif' dedents BEFORE it is written, so it lines up with the 'if'
            // that opened the block rather than with the block's body.
            if (autoindent && isPredPart && predPart->type != IfPredPartType::If) {
                --tabCount;
            }

            const std::string tabs(static_cast<std::size_t>(tabCount < 0 ? 0 : tabCount), '\t');
            const std::string currentLinePrefix = linePrefix + tabs;

            if (isPredPart) {
                lines.push_back(predPart->toStr(currentLinePrefix));
            } else {
                // The one line the pybind11 original could not do directly -- it cast the part to a
                // Python object and called back through IfContentPart.toStr. Same instantiation
                // either way, so this is the identical render without the round-trip.
                lines.push_back(renderIfContentPart(
                    *static_cast<IfContentPart<std::string, std::string>*>(part.get()), currentLinePrefix));
            }

            // ...and everything that opens a block indents AFTER it is written.
            if (autoindent && isPredPart && predPart->type != IfPredPartType::EndIf) {
                ++tabCount;
            }
        }

        if (!section.suffix.empty()) {
            lines.push_back(section.suffix);
        }

        std::string joined;
        for (std::size_t i = 0; i < lines.size(); ++i) {
            if (i > 0) {
                joined += "\n";
            }
            joined += lines[i];
        }

        return joined;
    }
}
