#include "AGRemapCore/data/IniFixData/MonaFixer.h"

#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMICharFixer.h"


namespace AGRemapCore {

    IniFixBuilder::Factory IniFixBuilderFuncs::mona6_1() {
        // Remapped onto MonaCN, a genuinely different model -- see makeGIMICharFixer for what
        // that shape does. Only what Mona does differently lives here.
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body"};

        // moveDrawIndexed stays false: the pure-Python row for Mona carries none of the Ib*
        // entries Amber's does, and a mod the old script has already fixed keeps
        // 'drawindexed = auto' on the remapped IB section untouched.

        return makeGIMICharFixer(std::move(config));
    }


    IniFixBuilder::Factory MonaFixer::v6_1() {
        return IniFixBuilderFuncs::mona6_1();
    }
}
