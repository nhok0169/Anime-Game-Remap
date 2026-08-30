#include "AGRemapCore/data/ModDataAssets.h"


namespace AGRemapCore {
    const std::shared_ptr<VGRemaps>& ModDataAssets::vgRemaps() {
        // Function-local static: the C++ equivalent of the pure-Python original's DeferredEnum --
        // built once, on first access, thread-safely. Held by shared_ptr because that is what
        // ModType::vgRemaps holds, and every ModType falling back to this one shares it.
        static const std::shared_ptr<VGRemaps> remaps = std::make_shared<VGRemaps>();
        return remaps;
    }
}
