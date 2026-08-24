#include "AGRemapCore/model/strategies/texEditors/texFilters/BaseTexFilter.h"

namespace AGRemapCore {

    void BaseTexFilter::operator()(TextureFile &texFile) {
        transform(texFile);
    }

    void BaseTexFilter::transform(TextureFile &) {}
}
