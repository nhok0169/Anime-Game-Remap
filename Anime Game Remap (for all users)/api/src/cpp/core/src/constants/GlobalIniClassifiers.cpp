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
        //
        // The classifier's OWN keywords are genuinely one-shot: nothing empties them, so building
        // them once is right.
        static IniClassifier instance;
        static const bool populated = [] {
            populate(instance);
            return true;
        }();
        (void) populated;

        // The registry half is NOT one-shot, and used to be -- it sat inside the lambda above.
        // ModTypeIdTools::clear() can empty the registry at any point, and when it did, this
        // classifier kept finding mod type ids that nothing could resolve for the rest of the
        // process: every .ini file came back isMod == true with no mod types at all. Re-filed
        // whenever the registry has been cleared since the last time this looked.
        //
        // registerMissing rather than registerAll: filling in what is absent is the default doing
        // its job, whereas overwriting an id the caller registered for itself would be the default
        // overruling a decision that was explicitly made. See its own doc comment.
        //
        // No locking, matching the rest of ModTypeIdTools -- its registry is a plain static map
        // with no synchronization of its own, so a caller mutating it from several threads is
        // already outside what this class supports.
        static unsigned long long populatedAtGeneration = 0;
        const unsigned long long generation = ModTypeIdTools::generation();

        if (populatedAtGeneration != generation) {
            GlobalModTypes::registerMissing();
            populatedAtGeneration = generation;
        }

        return instance;
    }
}
