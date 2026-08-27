#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegNewVals.h"


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
        using ConditionalSpec = std::pair<V, ModTypePredicate>;

        // Every alternative but the conditional one carries straight over -- only the conditional
        // one needs 'modType' bound into it, narrowing this class's own wider ModTypePredicate
        // down to the plain single-argument Predicate replaceVals takes.
        std::vector<std::pair<K, ReplaceSpec>> replaceSpecs;
        replaceSpecs.reserve(vals.size());

        for (const auto& [key, spec] : vals) {
            if (std::holds_alternative<std::vector<V>>(spec)) {
                replaceSpecs.emplace_back(key, ReplaceSpec(std::get<std::vector<V>>(spec)));
                continue;
            }

            if (std::holds_alternative<ConditionalSpec>(spec)) {
                const auto& conditional = std::get<ConditionalSpec>(spec);
                const ModTypePredicate& predicate = conditional.second;

                Predicate boundPredicate = [predicate, modType](const V& oldValue) {
                    return predicate(oldValue, modType);
                };

                replaceSpecs.emplace_back(key, ReplaceSpec(std::pair<V, Predicate>(conditional.first, std::move(boundPredicate))));
                continue;
            }

            replaceSpecs.emplace_back(key, ReplaceSpec(std::get<V>(spec)));
        }

        part.replaceVals(replaceSpecs, addNewKVPs, Base::toRangeSpec(partRanges));
        return part;
    }
}
