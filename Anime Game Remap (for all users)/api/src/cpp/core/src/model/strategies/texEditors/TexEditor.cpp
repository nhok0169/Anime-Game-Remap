#include "AGRemapCore/model/strategies/texEditors/TexEditor.h"

#include <utility>

#include "AGRemapCore/model/files/TextureFile.h"

namespace AGRemapCore {

    TexEditor::TexEditor(std::vector<Filter> filters): filters_(std::move(filters)) {}

    const std::vector<TexEditor::Filter>& TexEditor::getFilters() const {
        return filters_;
    }

    void TexEditor::setFilters(std::vector<Filter> filters) {
        filters_ = std::move(filters);
    }

    void TexEditor::fix(TextureFile &texFile, const std::string &fixedTexFile) {
        if (filters_.empty()) {
            return;
        }

        texFile.open();
        if (!texFile.hasImage()) {
            return;
        }

        for (const auto &filter : filters_) {
            filter(texFile);
        }

        texFile.setSrc(fixedTexFile);
        texFile.save();
    }
}
