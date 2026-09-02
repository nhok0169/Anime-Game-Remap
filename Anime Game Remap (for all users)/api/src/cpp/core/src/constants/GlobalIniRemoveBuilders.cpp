#include "AGRemapCore/constants/GlobalIniRemoveBuilders.h"


namespace AGRemapCore {
    const std::shared_ptr<IniRemoveBuilder>& GlobalIniRemoveBuilders::removeBuilder() {
        // Function-local static: the C++ equivalent of the pure-Python original's DeferredEnum --
        // built once, on first access, thread-safely. Held by shared_ptr rather than by value
        // because ModType::iniRemoveBuilder is a shared_ptr and every ModType falling back to this
        // one shares it. A builder is immutable once constructed, so sharing it is free.
        static const std::shared_ptr<IniRemoveBuilder> builder = std::make_shared<IniRemoveBuilder>();
        return builder;
    }
}
