#include "AGRemapCore/constants/GlobalIniClassifiers.h"

#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

#include "AGRemapCore/constants/GameTypeId.h"
#include "AGRemapCore/constants/GlobalModTypes.h"
#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/model/strategies/ModType.h"
#include "AGRemapCore/model/strategies/ModTypeIdData.h"


namespace AGRemapCore {
    namespace {
        // Registers every shipped mod type on 'classifier'. The counterpart to the pure-Python
        // IniClassifierBuilderOld::build, minus its whole first half: that one also wires up the
        // generic isFixed/isMod machinery (comment markers, "textureoverride", RemapFix/RemapTex,
        // Blend, RemapBlend/RemapPosition, Position) as explicit DFA states, whereas IniClassifier
        // checks those prefixes directly in readSectionName. Only the per-mod-type registration
        // has to be reproduced here.
        void populate(IniClassifier& classifier) {
            // The other half of the same default. A classifier finds mod type *ids*; IniFile then
            // asks ModTypeIdTools to turn each one back into a ModType, and gets nothing unless the
            // registry was filled. Populating one without the other leaves classify() naming an id
            // it cannot resolve, so asking for the default classifier fills both.
            //
            // This deliberately does not make registration implicit in general -- see
            // GlobalModTypes::registerAll's own note. A caller that injects its own classifier is
            // never routed through here and keeps full control of the registry, which is exactly
            // what core/tests/IniFile_classify_test.cpp relies on.
            GlobalModTypes::registerAll();

            for (const ModType& modType : GlobalModTypes::all()) {
                std::optional<ModTypeId> modTypeId = ModTypeIdTools::getEnum(modType.modTypeId);
                if (!modTypeId.has_value()) {
                    continue;
                }

                std::vector<std::string> keywords = ModTypeIdTools::getSectionKeywords(*modTypeId);
                if (keywords.empty()) {
                    continue;
                }

                // No hashes: the pure-Python builder registers section-name keywords only, and
                // identifies a mod type by its name rather than by any hash it carries. Passing
                // ModType::hashes here would be a behaviour change, not a port.
                classifier.addGIModType(ModTypeIdData(static_cast<int>(GameTypeId::GI), modType.modTypeId), {},
                                        std::unordered_set<std::string>(keywords.begin(), keywords.end()));
            }
        }
    }

    IniClassifier& GlobalIniClassifiers::classifier() {
        // Two statics rather than one initialized from a lambda's return: IniClassifier holds a
        // BaseAhoCorasickDFA, which owns a unique_ptr and so has neither a copy nor a move
        // constructor to return through. Both are function-local statics, so both still get C++11's
        // guaranteed thread-safe exactly-once initialization, and 'populated' is initialized after
        // 'instance' by declaration order.
        static IniClassifier instance;
        static const bool populated = [] {
            populate(instance);
            return true;
        }();
        (void) populated;

        return instance;
    }
}
