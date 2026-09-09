#include "AGRemapCore/data/IniFixData/Amber/AmberFixer.h"

#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMICharFixer.h"


namespace AGRemapCore {

    IniFixBuilder::Factory IniFixBuilderFuncs::amber6_1() {
        // Remapped onto AmberCN, a genuinely different model -- see makeGIMICharFixer for what
        // that shape does. Only what Amber does differently lives here.
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body"};

        // Amber's fix DOES move the shared draw call onto her drawn objects. Mona's and Rosaria's do
        // not, despite their .ini files having the same shape -- see GIMICharFixerConfig.
        config.moveDrawIndexed = true;

        return makeGIMICharFixer(std::move(config));
    }


    IniFixBuilder::Factory AmberFixer::v6_1() {
        return IniFixBuilderFuncs::amber6_1();
    }
}
