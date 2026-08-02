#include "AGRemapCore/tools/parsing/ParseContext.h"
#include "AGRemapCore/tools/StringTools.h"


namespace AGRemapCore {
    ParseContext::ParseContext(std::string_view src, std::optional<std::string_view> file, size_t startLineNo): startLineNo(startLineNo), file(file) {
        lines = StringTools::splitlines(src);
    }

    size_t ParseContext::getEndLineNo() {
        return startLineNo + lines.size();
    }
}

