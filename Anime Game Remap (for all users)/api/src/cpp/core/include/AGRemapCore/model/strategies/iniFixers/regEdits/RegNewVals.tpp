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

#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegNewVals.h"

#include <algorithm>


namespace AGRemapCore {

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    RegNewVals<K, V, KeyHash, KeyEqual>::RegNewVals(std::vector<std::pair<K, NewValSpec>> vals, bool addNewKVPs):
        vals(std::move(vals)), addNewKVPs(addNewKVPs) {}

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename RegNewVals<K, V, KeyHash, KeyEqual>::ContentPart& RegNewVals<K, V, KeyHash, KeyEqual>::edit(
            ContentPart& part, const std::string& sectionName, const ModType* modType, const std::string& modName,
            const OrderRanges* partRanges) {
        (void)sectionName;
        (void)modName;

        using ReplaceSpec = typename ContentPart::ReplaceSpec;
        using Predicate = typename ContentPart::Predicate;
        using ConditionalSpec = std::pair<NewVal, ModTypePredicate>;

        // Neither of the two things this class's specs carry beyond replaceVals' own survives the
        // handoff: a ValProducer is called here to get the plain value replaceVals wants, and a
        // conditional's ModTypePredicate has 'modType' bound into it, narrowing it down to the
        // single-argument Predicate replaceVals takes.
        auto resolve = [modType](const NewVal& newVal) -> V {
            if (std::holds_alternative<ValProducer>(newVal)) {
                return std::get<ValProducer>(newVal)(modType);
            }

            return std::get<V>(newVal);
        };

        // An OldValProducer's value is not known until the occurrence it rewrites is in hand, so a
        // spec holding one cannot become a ReplaceSpec. Those keys are written below instead.
        auto isOldVal = [](const NewVal& newVal) {
            return std::holds_alternative<OldValProducer>(newVal);
        };

        auto specNeedsOldVal = [&isOldVal](const NewValSpec& spec) {
            if (std::holds_alternative<std::vector<NewVal>>(spec)) {
                const auto& newVals = std::get<std::vector<NewVal>>(spec);
                return std::any_of(newVals.begin(), newVals.end(), isOldVal);
            }

            if (std::holds_alternative<ConditionalSpec>(spec)) {
                return isOldVal(std::get<ConditionalSpec>(spec).first);
            }

            return isOldVal(std::get<NewVal>(spec));
        };

        const std::optional<typename ContentPart::RangeSpec> ranges = Base::toRangeSpec(partRanges);

        std::vector<std::pair<K, ReplaceSpec>> replaceSpecs;
        replaceSpecs.reserve(vals.size());

        for (const auto& [key, spec] : vals) {
            if (specNeedsOldVal(spec)) {
                // The indices are TRUE POSITIONAL ones and they are per PART -- the same convention
                // replaceVals sorts a key's bucket into, which is what keeps the rules below the
                // same rules.
                const std::vector<std::pair<long long, V>> occurrences =
                    part.getValsWithInds(key, true, ranges);

                for (std::size_t i = 0; i < occurrences.size(); ++i) {
                    const long long at = occurrences[i].first;
                    const V& oldValue = occurrences[i].second;

                    const NewVal* newVal = nullptr;
                    if (std::holds_alternative<std::vector<NewVal>>(spec)) {
                        const auto& newVals = std::get<std::vector<NewVal>>(spec);
                        if (i < newVals.size()) {            // an entry past the end is unused
                            newVal = &newVals[i];
                        }
                    } else if (std::holds_alternative<ConditionalSpec>(spec)) {
                        const auto& conditional = std::get<ConditionalSpec>(spec);
                        if (conditional.second(oldValue, modType)) {
                            newVal = &conditional.first;
                        }
                    } else {
                        newVal = &std::get<NewVal>(spec);
                    }

                    if (newVal == nullptr) {
                        continue;
                    }

                    const V newValue = isOldVal(*newVal)
                        ? std::get<OldValProducer>(*newVal)(oldValue, modType) : resolve(*newVal);

                    if (newValue != oldValue) {
                        part.setValByInd(at, newValue);
                    }
                }

                continue;
            }

            if (std::holds_alternative<std::vector<NewVal>>(spec)) {
                const auto& newVals = std::get<std::vector<NewVal>>(spec);

                std::vector<V> resolved;
                resolved.reserve(newVals.size());
                for (const NewVal& newVal : newVals) {
                    resolved.push_back(resolve(newVal));
                }

                replaceSpecs.emplace_back(key, ReplaceSpec(std::move(resolved)));
                continue;
            }

            if (std::holds_alternative<ConditionalSpec>(spec)) {
                const auto& conditional = std::get<ConditionalSpec>(spec);
                const ModTypePredicate& predicate = conditional.second;

                Predicate boundPredicate = [predicate, modType](const V& oldValue) {
                    return predicate(oldValue, modType);
                };

                replaceSpecs.emplace_back(key, ReplaceSpec(std::pair<V, Predicate>(resolve(conditional.first), std::move(boundPredicate))));
                continue;
            }

            replaceSpecs.emplace_back(key, ReplaceSpec(resolve(std::get<NewVal>(spec))));
        }

        part.replaceVals(replaceSpecs, addNewKVPs, ranges);
        return part;
    }
}
