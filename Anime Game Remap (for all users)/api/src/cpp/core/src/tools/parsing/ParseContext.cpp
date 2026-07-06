#include "AGRemapCore/tools/parsing/ParseContext.h"
#include "AGRemapCore/tools/StringTools.h"


namespace AGRemapCore {
    ParseContext::ParseContext(std::string_view src, std::optional<std::string_view> file = std::nullopt, size_t startLineNo = 1): startLineNo(startLineNo), file(file) {
        this.lines = StringTools::splitlines(src);
    }

    size_t ParseContext::getEndLineNo() {
        return startLineNo + lines.size();
    }
}

