// ##### Credits

// ===== Anime Game Remap (AG Remap) =====
// Authors: Albert Gold#2696, NK#1321
//
// if you used it to remap your mods pls give credit for "Albert Gold#2696" and "Nhok0169"
// Special Thanks:
//   nguen#2011 (for support)
//   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
//   HazrateGolabi#1364 (for being awesome, and improving the code)

// ##### EndCredits

#include "AGRemapCore/constants/GlobalIniRemoveBuilders.h"

// The general-use remover globalRemoveBuilder hands out. Included here rather than from the header
// for the same reason IniRemoveBuilder.cpp includes RemapIniRemover.h in its own .cpp -- that header
// reaches the whole IniFile, which comes back here through ModType.h.
#include "AGRemapCore/model/strategies/iniRemovers/GlobalRemapIniRemover.h"


namespace AGRemapCore {
    const std::shared_ptr<IniRemoveBuilder>& GlobalIniRemoveBuilders::removeBuilder() {
        // Function-local static: the C++ equivalent of the pure-Python original's DeferredEnum --
        // built once, on first access, thread-safely. Held by shared_ptr rather than by value
        // because ModType::iniRemoveBuilder is a shared_ptr and every ModType falling back to this
        // one shares it. A builder is immutable once constructed, so sharing it is free.
        static const std::shared_ptr<IniRemoveBuilder> builder = std::make_shared<IniRemoveBuilder>();
        return builder;
    }

    const std::shared_ptr<IniRemoveBuilder>& GlobalIniRemoveBuilders::globalRemoveBuilder() {
        // Same one-shot lazy static as removeBuilder above -- only the factory differs. The
        // fixed-factory IniRemoveBuilder constructor, so every build() hands back a GlobalRemapIniRemover
        // whatever mod name or version it is asked about (there is no per-mod-type table to consult
        // here by definition: this builder exists for the file that has no mod type).
        static const std::shared_ptr<IniRemoveBuilder> builder =
            std::make_shared<IniRemoveBuilder>(GlobalRemapIniRemover<>::factory());
        return builder;
    }
}
