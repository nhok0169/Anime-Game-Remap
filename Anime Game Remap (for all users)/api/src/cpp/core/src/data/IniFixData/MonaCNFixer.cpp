#include "AGRemapCore/data/IniFixData/MonaCNFixer.h"

#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMICharFixer.h"


namespace AGRemapCore {

    IniFixBuilder::Factory IniFixBuilderFuncs::monaCN6_1() {
        // Remapped onto Mona, a genuinely different model -- see makeGIMICharFixer for what
        // that shape does. Only what MonaCN does differently lives here.
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body"};

        // moveDrawIndexed stays false: the pure-Python row for MonaCN carries none of the Ib*
        // entries Amber's does, and a mod the old script has already fixed keeps
        // 'drawindexed = auto' on the remapped IB section untouched.

        return makeGIMICharFixer(std::move(config));
    }


    IniFixBuilder::Factory MonaCNFixer::v6_1() {
        return IniFixBuilderFuncs::monaCN6_1();
    }
}
