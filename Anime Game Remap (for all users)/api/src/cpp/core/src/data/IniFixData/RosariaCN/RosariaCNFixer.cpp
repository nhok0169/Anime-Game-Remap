#include "AGRemapCore/data/IniFixData/RosariaCN/RosariaCNFixer.h"

#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMICharFixer.h"


namespace AGRemapCore {

    IniFixBuilder::Factory IniFixBuilderFuncs::rosariaCN6_1() {
        // Remapped onto Rosaria, a genuinely different model -- see makeGIMICharFixer for what
        // that shape does. Only what RosariaCN does differently lives here.
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body", "dress", "extra"};

        // moveDrawIndexed stays false: the pure-Python row for RosariaCN carries none of the Ib*
        // entries Amber's does, and a mod the old script has already fixed keeps
        // 'drawindexed = auto' on the remapped IB section untouched.

        return makeGIMICharFixer(std::move(config));
    }


    IniFixBuilder::Factory RosariaCNFixer::v6_1() {
        return IniFixBuilderFuncs::rosariaCN6_1();
    }
}
