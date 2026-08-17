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
