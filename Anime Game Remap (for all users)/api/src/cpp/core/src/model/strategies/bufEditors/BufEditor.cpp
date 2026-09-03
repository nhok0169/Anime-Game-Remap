#include "AGRemapCore/model/strategies/bufEditors/BufEditor.h"

#include <utility>

namespace AGRemapCore {

    BufEditor::BufEditor(std::vector<BufFile::Filter> filters): filters(std::move(filters)) {}

    BufFile::FixResult BufEditor::fix(BufFile &bufFile, const std::optional<std::string> &fixedBufFile) {
        return bufFile.fix(fixedBufFile, filters);
    }
}
