#ifndef AGRemapCore_GIMISectionClassifier_TPP
#define AGRemapCore_GIMISectionClassifier_TPP

#include <type_traits>

#include "GIMISectionClassifier.h"


namespace AGRemapCore {
    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename GIMISectionClassifier<K, V, KeyHash, KeyEqual>::ClassifierConfig GIMISectionClassifier<K, V, KeyHash, KeyEqual>::defaultConfig() {
        ClassifierConfig result{};

        // Only a K that a .ini keyword can literally be spelled as gets real defaults. Every other
        // instantiation (notably the pybind11 layer's py::object) has to supply its own config --
        // there is nothing sensible to build here without knowing how to make a K out of a string.
        if constexpr (std::is_constructible_v<K, const std::string&>) {
            result.hashKey = K(IniKeywords::Hash);
            result.matchFirstIndexKey = K(IniKeywords::MatchFirstIndex);
        }

        return result;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    GIMISectionClassifier<K, V, KeyHash, KeyEqual>::GIMISectionClassifier(
            std::unordered_map<K, ModObj, KeyHash, KeyEqual> hashKeyOnlyToModObj, Assets* hashes,
            std::unordered_map<K, IndexModObjs, KeyHash, KeyEqual> indexKeyToModObj, Assets* indices,
            std::optional<Version> version, ClassifierConfig config):
        hashKeyOnlyToModObj(std::move(hashKeyOnlyToModObj)), indexKeyToModObj(std::move(indexKeyToModObj)),
        version(std::move(version)), hashes_(hashes), indices_(indices), config_(std::move(config)) {}


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename GIMISectionClassifier<K, V, KeyHash, KeyEqual>::Assets* GIMISectionClassifier<K, V, KeyHash, KeyEqual>::hashes() const {
        return hashes_;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void GIMISectionClassifier<K, V, KeyHash, KeyEqual>::setHashes(Assets* newHashes) {
        hashes_ = newHashes;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename GIMISectionClassifier<K, V, KeyHash, KeyEqual>::Assets* GIMISectionClassifier<K, V, KeyHash, KeyEqual>::indices() const {
        return indices_;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void GIMISectionClassifier<K, V, KeyHash, KeyEqual>::setIndices(Assets* newIndices) {
        indices_ = newIndices;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    const std::vector<std::optional<K>>& GIMISectionClassifier<K, V, KeyHash, KeyEqual>::hashNonVersionVals() const {
        return hashNonVersionVals_;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void GIMISectionClassifier<K, V, KeyHash, KeyEqual>::setHashNonVersionVals(std::vector<std::optional<K>> newVals) {
        hashNonVersionVals_ = std::move(newVals);
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    const std::vector<std::optional<K>>& GIMISectionClassifier<K, V, KeyHash, KeyEqual>::indexNonVersionVals() const {
        return indexNonVersionVals_;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void GIMISectionClassifier<K, V, KeyHash, KeyEqual>::setIndexNonVersionVals(std::vector<std::optional<K>> newVals) {
        indexNonVersionVals_ = std::move(newVals);
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    const typename GIMISectionClassifier<K, V, KeyHash, KeyEqual>::ClassifierConfig& GIMISectionClassifier<K, V, KeyHash, KeyEqual>::config() const {
        return config_;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    bool GIMISectionClassifier<K, V, KeyHash, KeyEqual>::indFilter(const std::optional<long long>& ind,
                                                                    const std::optional<long long>& startInd,
                                                                    const std::optional<long long>& endInd) {
        if (!ind.has_value()) {
            return false;
        }

        if (startInd.has_value() && *ind < *startInd) {
            return false;
        }

        return !endInd.has_value() || *ind < *endInd;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::vector<typename GIMISectionClassifier<K, V, KeyHash, KeyEqual>::ModObj> GIMISectionClassifier<K, V, KeyHash, KeyEqual>::classify(
            const std::string& sectionName, Section* section, const Colouring& partKeys) const {
        // Both are part of the pure-Python original's signature and unused by its body too -- kept
        // so a caller can swap this classifier for any other ObjTargetFunc without changing shape.
        (void)sectionName;
        (void)section;

        std::vector<ModObj> result;
        std::unordered_map<ModObj, bool, ModObjHash> seen;

        auto addResult = [&result, &seen](const ModObj& modObj) {
            if (seen.emplace(modObj, true).second) {
                result.push_back(modObj);
            }
        };

        if (hashes_ == nullptr) {
            return result;
        }

        std::vector<std::pair<std::optional<long long>, V>> hashVals = partKeys.getIndVals(config_.hashKey);
        std::size_t hashValsLen = hashVals.size();

        for (std::size_t i = 0; i < hashValsLen; ++i) {
            const std::optional<long long>& hashInd = hashVals[i].first;
            const V& hashVal = hashVals[i].second;

            std::optional<std::vector<K>> hashKeyRow = hashes_->getKey(hashVal, version, hashNonVersionVals_, false);
            if (!hashKeyRow.has_value() || hashKeyRow->empty()) {
                continue;
            }

            const K& hashKey = hashKeyRow->back();

            auto hashIndexToModObj = indexKeyToModObj.find(hashKey);
            auto hashModObj = hashKeyOnlyToModObj.find(hashKey);
            bool inHashOnly = hashModObj != hashKeyOnlyToModObj.end();
            bool hashInIndex = hashIndexToModObj != indexKeyToModObj.end();

            std::optional<long long> nextHashInd;
            if (i != hashValsLen - 1) {
                nextHashInd = hashVals[i + 1].first;
            }

            if (!hashInIndex && !inHashOnly) {
                continue;
            }

            if (!hashInIndex) {
                addResult(hashModObj->second);
                continue;
            }

            typename Colouring::Filter filter = [&hashInd, &nextHashInd](std::optional<long long> ind, const V&) {
                return indFilter(ind, hashInd, nextHashInd);
            };

            std::vector<V> indexVals = partKeys.getVals(config_.matchFirstIndexKey, filter);

            for (const V& indexVal : indexVals) {
                const ModObj* indexModObj = nullptr;

                if (indices_ != nullptr) {
                    std::optional<std::vector<K>> indexKeyRow = indices_->getKey(indexVal, version, indexNonVersionVals_, false);

                    if (indexKeyRow.has_value() && indexKeyRow->size() >= 2) {
                        IndexKey indexKey((*indexKeyRow)[indexKeyRow->size() - 2], indexKeyRow->back());

                        auto found = hashIndexToModObj->second.find(indexKey);
                        if (found != hashIndexToModObj->second.end()) {
                            indexModObj = &found->second;
                        }
                    }
                }

                if (indexModObj == nullptr && inHashOnly) {
                    addResult(hashModObj->second);
                } else if (indexModObj != nullptr) {
                    addResult(*indexModObj);
                }
            }

            if (indexVals.empty() && inHashOnly) {
                addResult(hashModObj->second);
            }
        }

        return result;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::unordered_map<K, typename GIMISectionClassifier<K, V, KeyHash, KeyEqual>::ModObj, KeyHash, KeyEqual>
            GIMISectionClassifier<K, V, KeyHash, KeyEqual>::buildDefaultHashKeyOnlyToModObj(const KeyMaker& makeKey) {
        std::unordered_map<K, ModObj, KeyHash, KeyEqual> result;

        if (!makeKey) {
            return result;
        }

        for (const std::pair<const std::string, Data::ModObjKey>& entry : Data::getHashKeyOnlyToModObj()) {
            result.emplace(makeKey(entry.first), ModObj(entry.second.first, entry.second.second));
        }

        return result;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::unordered_map<K, typename GIMISectionClassifier<K, V, KeyHash, KeyEqual>::IndexModObjs, KeyHash, KeyEqual>
            GIMISectionClassifier<K, V, KeyHash, KeyEqual>::buildDefaultIndexKeyToModObj(const KeyMaker& makeKey) {
        std::unordered_map<K, IndexModObjs, KeyHash, KeyEqual> result;

        if (!makeKey) {
            return result;
        }

        for (const std::pair<const std::string, Data::IndexKeyToModObj>& entry : Data::getIndexKeyToModObj()) {
            IndexModObjs inner;

            for (const std::pair<const Data::ModObjKey, Data::ModObjKey>& innerEntry : entry.second) {
                // The index key is whatever the Indices table's own last two columns hold, so it
                // goes through 'makeKey' too -- unlike the mod object it maps to, which is always
                // a plain pair of std::string.
                inner.emplace(IndexKey(makeKey(innerEntry.first.first), makeKey(innerEntry.first.second)),
                              ModObj(innerEntry.second.first, innerEntry.second.second));
            }

            result.emplace(makeKey(entry.first), std::move(inner));
        }

        return result;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::unique_ptr<GIMISectionClassifier<K, V, KeyHash, KeyEqual>> GIMISectionClassifier<K, V, KeyHash, KeyEqual>::buildDefaultClassifier(
            Assets* hashes, Assets* indices, std::optional<Version> version, ClassifierConfig config) {
        KeyMaker makeKey;

        // Same constraint as defaultConfig's: a K that no .ini keyword can be spelled as has no
        // way to be made from the hash data table's own std::string keys here -- that
        // instantiation supplies its own mappings, as the pybind11 layer does.
        if constexpr (std::is_constructible_v<K, const std::string&>) {
            makeKey = [](const std::string& key) { return K(key); };
        }

        return std::make_unique<GIMISectionClassifier<K, V, KeyHash, KeyEqual>>(
            buildDefaultHashKeyOnlyToModObj(makeKey), hashes,
            buildDefaultIndexKeyToModObj(makeKey), indices, std::move(version), std::move(config));
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::unique_ptr<GIMISectionClassifier<K, V, KeyHash, KeyEqual>> GIMISectionClassifier<K, V, KeyHash, KeyEqual>::buildDefaultClassifierFromIni(
            IniParseContext<K, V, KeyHash, KeyEqual>& ctx, ClassifierConfig config) {
        return buildDefaultClassifier(ctx.modTypeHashes(), ctx.modTypeIndices(), ctx.version(), std::move(config));
    }
}

#endif
