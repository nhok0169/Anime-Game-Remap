#include "AGRemapCore/model/strategies/bufEditors/BaseBufEditor.h"

namespace AGRemapCore {

    BufFile::FixResult BaseBufEditor::fix(BufFile &, const std::optional<std::string> &) {
        return BufFile::FixResult(ByteVec{});
    }
}
