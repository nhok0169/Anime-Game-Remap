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

#include "AGRemapCore/constants/GlobalIniClassifiers.h"

#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "AGRemapCore/constants/GameTypeId.h"
#include "AGRemapCore/constants/GlobalModTypes.h"
#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/HashData.h"
#include "AGRemapCore/model/strategies/ModType.h"
#include "AGRemapCore/model/strategies/ModTypeIdData.h"


namespace AGRemapCore {
    namespace {
        /*
         * The hash types that IDENTIFY a character, as opposed to merely belonging to one.
         *
         * These five and no others, for two reasons that happen to agree. They are the hash
         * types a GIMI mod actually writes as a section's "hash = " line -- the ib, the three
         * vertex buffers, and the draw hash the VertexLimitRaise sections carry -- so they are
         * the only ones the classifier can see at all. And, measured over the whole of
         * HashData, they are UNIQUE: across 312 rows not one of their values is claimed by two
         * different characters.
         *
         * The texture hashes are the opposite and must stay out. A shadow ramp or a metal map
         * is a shared asset: 'b0e08915' is filed under FORTY different names and '7eb5b84e'
         * under thirty-three, so registering those would hand a +2 vote to forty characters for
         * one line of a mod's .ini -- noise loud enough to decide a classification on its own.
         * Even 'tex_face_diffuse', which does appear as a section hash, is shared in 21 of its
         * 37 rows.
         */
        const std::unordered_set<std::string>& identifyingHashTypes() {
            static const std::unordered_set<std::string> types = {
                "ib", "draw_vb", "position_vb", "blend_vb", "texcoord_vb"};
            return types;
        }


        /*
         * modTypeName -> every identifying hash value HashData files under it, across every
         * game version.
         *
         * Every version's, deliberately: a mod's .ini carries whichever version's hashes its
         * author dumped, and the classifier has no way to know which that was. The uniqueness
         * measured above holds across versions too, so an old hash identifies the character
         * just as well as the current one.
         *
         * Built off the raw row list rather than a Hashes instance: this needs the FORWARD
         * direction (name -> values) and nothing else, and getHashDataRows is already the flat
         * table every Hashes is parsed from.
         */
        const std::unordered_map<std::string, std::unordered_set<std::string>>& hashesByModName() {
            static const std::unordered_map<std::string, std::unordered_set<std::string>> byName = []() {
                std::unordered_map<std::string, std::unordered_set<std::string>> acc;

                // Each row is {{version, name, type}, hashValue} -- see HashData's own layout,
                // and Hashes' 3-index (version at position 0) ModDictAssets built from it.
                for (const std::pair<std::vector<std::string>, std::string>& row : Data::getHashDataRows()) {
                    if (row.first.size() < 3 || !identifyingHashTypes().contains(row.first[2])) {
                        continue;
                    }

                    acc[row.first[1]].insert(row.second);
                }

                return acc;
            }();

            return byName;
        }


        // Registers every shipped mod type on 'classifier'. The counterpart to the pure-Python
        // IniClassifierBuilderOld::build, minus its whole first half: that one also wires up the
        // generic isFixed/isMod machinery (comment markers, "textureoverride", RemapFix/RemapTex,
        // Blend, RemapBlend/RemapPosition, Position) as explicit DFA states, whereas IniClassifier
        // checks those prefixes directly in readSectionName. Only the per-mod-type registration
        // has to be reproduced here.
        void populate(IniClassifier& classifier) {
            static const std::unordered_set<std::string> noHashes;
            const std::unordered_map<std::string, std::unordered_set<std::string>>& byName = hashesByModName();

            for (const ModType& modType : GlobalModTypes::all()) {
                std::optional<ModTypeId> modTypeId = ModTypeIdTools::getEnum(modType.modTypeId);
                if (!modTypeId.has_value()) {
                    continue;
                }

                std::vector<std::string> keywords = ModTypeIdTools::getSectionKeywords(*modTypeId);
                if (keywords.empty()) {
                    continue;
                }

                // THE HASHES, which this used to pass as {} -- so for the whole life of the C++
                // classifier the GI half of it was name-only, and the hash states its DFA builds
                // were reachable by WuWa mod types alone.
                //
                // What that cost: a mod whose author named every section after the BASE character
                // while building on the SKIN's model classifies as the base character, and then
                // every reverse hash lookup the fix makes fails and writes 'HashNotFound'. The
                // section-name evidence was all there was, and it was wrong.
                //
                // The weighting this leans on is already in IniClassifier and already deliberate:
                // incModTypeCountByHash adds 2 where incModTypeCountBySectionName adds 1, so the
                // hashes outvote the names on a file where the two disagree. So is the guard that
                // makes it safe on an already-fixed mod -- readLine skips a 'hash =' inside a
                // Remap-named section, so the TARGET hashes a fix wrote in do not vote.
                auto hashIt = byName.find(modType.name);

                classifier.addGIModType(ModTypeIdData(static_cast<int>(GameTypeId::GI), modType.modTypeId),
                                        hashIt != byName.end() ? hashIt->second : noHashes,
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
