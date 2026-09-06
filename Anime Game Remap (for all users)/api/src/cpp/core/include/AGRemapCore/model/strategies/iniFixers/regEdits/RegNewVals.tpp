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

        std::vector<std::pair<K, ReplaceSpec>> replaceSpecs;
        replaceSpecs.reserve(vals.size());

        for (const auto& [key, spec] : vals) {
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

        part.replaceVals(replaceSpecs, addNewKVPs, Base::toRangeSpec(partRanges));
        return part;
    }
}
