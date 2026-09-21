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

#include "AGRemapCore/constants/GlobalModTypes.h"

#include <iterator>
#include <utility>

#include "AGRemapCore/constants/GIBuilder.h"
#include "AGRemapCore/constants/WWMIBuilder.h"
#include "AGRemapCore/constants/ModTypeId.h"


namespace AGRemapCore {

    std::vector<ModType> GlobalModTypes::all() {
        // Every game's builder, aggregated here rather than at each call site -- GI first, then
        // WuWa (2026-09-19).
        std::vector<ModType> result = GIBuilder::all();
        std::vector<ModType> wuwa = WWMIBuilder::all();
        result.insert(result.end(), std::make_move_iterator(wuwa.begin()), std::make_move_iterator(wuwa.end()));
        return result;
    }

    void GlobalModTypes::registerMissing() {
        // NOTHING AT ALL when the shipped set cannot have gone missing since this last filed it.
        //
        // The expensive half of this function is all() -- it builds every one of the 49 shipped
        // mod types, with their asset tables, just to ask which ids are absent -- and an ordinary
        // run calls this at least TWICE: once from RemapServiceCLI's constructor, once when the
        // classifiers are populated. Measured at ~0.11s a call, which on a small mod is a tenth of
        // the whole run spent rediscovering that nothing is missing.
        //
        // The generation counter is the right guard rather than a plain "done" flag, and it is
        // already used this way by GlobalIniClassifiers: ModTypeIdTools::clear() can empty the
        // registry at any time, and it is the ONLY thing that bumps the generation. Registering a
        // mod type does not -- which is exactly right here, because a caller that filed its own
        // type under a shipped id is a caller this function must not overrule anyway (see the doc
        // comment), so there would have been nothing to fill in.
        static unsigned long long filedAtGeneration = 0;
        const unsigned long long generation = ModTypeIdTools::generation();

        if (filedAtGeneration == generation) {
            return;
        }

        std::vector<ModType> everything = all();
        std::vector<ModType> missing;

        for (ModType& modType : everything) {
            // Only the gap-filling half of registerAll -- see this function's own doc comment for
            // why the implicit path must not overwrite.
            if (!ModTypeIdTools::getModType(modType.modTypeId).has_value()) {
                missing.push_back(std::move(modType));
            }
        }

        // Handed over together rather than one at a time: each registration otherwise rebuilds the
        // whole name automaton from every name already filed.
        ModTypeIdTools::registerModTypes(missing);

        // Recorded AFTER the work, so a registerModTypes that threw leaves this to be retried.
        filedAtGeneration = generation;
    }


    void GlobalModTypes::registerAll() {
        ModTypeIdTools::registerModTypes(all());
    }
}
