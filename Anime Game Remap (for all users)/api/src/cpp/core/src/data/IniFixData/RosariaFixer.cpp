#include "AGRemapCore/data/IniFixData/RosariaFixer.h"

#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMICharFixer.h"


namespace AGRemapCore {

    IniFixBuilder::Factory IniFixBuilderFuncs::rosaria6_1() {
        // Remapped onto RosariaCN, a genuinely different model -- see makeGIMICharFixer for what
        // that shape does. Only what Rosaria does differently lives here.
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body", "dress", "extra"};

        // moveDrawIndexed stays false: the pure-Python row for Rosaria carries none of the Ib*
        // entries Amber's does, and a mod the old script has already fixed keeps
        // 'drawindexed = auto' on the remapped IB section untouched.

        return makeGIMICharFixer(std::move(config));
    }


    IniFixBuilder::Factory RosariaFixer::v6_1() {
        return IniFixBuilderFuncs::rosaria6_1();
    }
}
