#include "AGRemapCore/data/IniFixData/NingguangFixer.h"

#include <string>
#include <utility>
#include <vector>

#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/DarkDiffuse.h"
#include "AGRemapCore/data/IniFixData/GIMICharFixer.h"


namespace AGRemapCore {

    IniFixBuilder::Factory IniFixBuilderFuncs::ningguang6_1() {
        // Remapped onto NingguangOrchid, a genuinely different model -- see makeGIMICharFixer for
        // what that shape does. Only what Ningguang does differently lives here.
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body", "dress"};

        // NingguangOrchid does not read ps-t3 on any of the three. Ningguang binds one, and a
        // register the target's shader never samples is at best ignored and at worst read as
        // something else -- the pure-Python row strips it from all three objects and so does this.
        config.objRegRemovals = {{"head", {"ps-t3"}}, {"body", {"ps-t3"}}, {"dress", {"ps-t3"}}};

        // The head diffuse, blanked and gamma-corrected -- see DarkDiffuse for what the two halves
        // are for. Declared on the head's ps-t0 in the pure-Python parser row and repointed at the
        // same register by the fixer's RegTexEdit, so unlike Jean's there is no shift to follow.
        config.texEdits = {{"head", "ps-t0", "DarkDiffuse", &DarkDiffuse::edit}};

        // moveDrawIndexed stays false: the pure-Python row carries none of the Ib* entries.

        return makeGIMICharFixer(std::move(config));
    }


    IniFixBuilder::Factory NingguangFixer::v6_1() {
        return IniFixBuilderFuncs::ningguang6_1();
    }
}
