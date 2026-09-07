#include "AGRemapCore/model/strategies/texEditors/TexEditor.h"

#include <algorithm>
#include <cstdint>
#include <utility>
#include <vector>

#include "AGRemapCore/model/files/TextureFile.h"

namespace AGRemapCore {

    void TexEditor::setTransparency(TextureFile &texFile, int alpha) {
        if (!texFile.hasImage()) {
            return;
        }

        const std::uint8_t value = static_cast<std::uint8_t>(std::clamp(alpha, 0, 255));

        // getPixels() hands back a const reference, so the buffer is copied, rewritten and handed
        // back through setPixels -- which is also what keeps the width/height bookkeeping honest.
        std::vector<std::uint8_t> pixels = texFile.getPixels();
        for (std::size_t i = 3; i < pixels.size(); i += 4) {
            pixels[i] = value;
        }

        texFile.setPixels(std::move(pixels), texFile.getWidth(), texFile.getHeight());
    }


    TexEditor::TexEditor(std::vector<Filter> filters, bool compress):
        filters_(std::move(filters)), compress_(compress) {}


    bool TexEditor::getCompress() const { return compress_; }


    void TexEditor::setCompress(bool compress) { compress_ = compress; }

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
        texFile.save(compress_);
    }
}
