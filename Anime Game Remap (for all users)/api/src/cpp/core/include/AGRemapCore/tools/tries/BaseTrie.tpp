#include "AGRemapCore/tools/tries/BaseTrie.h"
#include "AGRemapCore/tools/idGenerator/IncIdGenerator.h"
#include "AGRemapCore/tools/grapheme/GraphemeRange.h"
#include "AGRemapCore/tools/Algo.h"
#include "AGRemapCore/tools/StringTools.h"


static std::uint64_t DefaultNodeId = 1;


namespace AGRemapCore {
    template class BiMap<std::uint64_t, std::string, std::hash<std::uint64_t>, std::equal_to<std::uint64_t>, StringViewHash, std::equal_to<void>>;

    // note: IncIdGenerator<std::uint64_t> is explicitly instantiated in BaseDFA.tpp, and
    //   BaseAhoCorasickDFA.h pulls in both that and this file -- instantiating it here too is a
    //   duplicate explicit instantiation in the same translation unit, which MSVC accepts but GCC
    //   rejects outright ("duplicate explicit instantiation ... [-fpermissive]"), breaking the Linux
    //   build. A translation unit that includes only this file still gets the type via ordinary
    //   implicit instantiation, so nothing is lost by leaving it to BaseDFA.tpp.


    template <typename TrieVal>
    TrieVal BaseTrie<TrieVal>::defaultHandleDuplicate(std::string_view key, const TrieVal &srcVal, const TrieVal &newVal) {
        return newVal;
    }

    template <typename TrieVal>
    BaseTrie<TrieVal>::BaseTrie(const std::optional<std::unordered_map<std::string, TrieVal>> &data, const std::optional<std::variant<typename BaseTrie<TrieVal>::DupHandler, typename BaseTrie<TrieVal>::DupHandler2>>& handler) {
        initKeywordIdGenerator();
        initNodeIdGenerator();

        if (handler.has_value()) {
            std::visit([this](const auto& visitedHandler) {
                this->setHandleDuplicate(visitedHandler);
            }, *handler);
        } else {
            handleDuplicate = BaseTrie<TrieVal>::defaultHandleDuplicate;
        }

        build(data);
    }

    template <typename TrieVal>
    const typename BaseTrie<TrieVal>::DupHandler &BaseTrie<TrieVal>::getHandleDuplicate() const {
        return handleDuplicate;
    }

    template <typename TrieVal>
    const typename BaseTrie<TrieVal>::DupHandler2 BaseTrie<TrieVal>::getHandleDuplicateStrRef() const {
        if (!this->handleDuplicate) return nullptr;

        return [internalFn = this->handleDuplicate](const std::string& key, const TrieVal& val1, const TrieVal& val2) {
            return internalFn(key, val1, val2);
        };
    }

    template <typename TrieVal>
    void BaseTrie<TrieVal>::setHandleDuplicate(const typename BaseTrie<TrieVal>::DupHandler &newHandler) {
        this->handleDuplicate = newHandler;
    }

    template <typename TrieVal>
    void BaseTrie<TrieVal>::setHandleDuplicate(const typename BaseTrie<TrieVal>::DupHandler2 &newHandler) {
        this->handleDuplicate = [capturedHandler = std::move(newHandler)](std::string_view sv, const TrieVal& val1, const TrieVal& val2) {
            return capturedHandler(std::string(sv), val1, val2);
        };
    }

    template <typename TrieVal>
    void BaseTrie<TrieVal>::clear() {
        keywords.clear();
        vals.clear();
        children.clear();
        nodeKeywordIds.clear();
        acceptNodeIds.clear();

        keywordIdGenerator->reset();
        nodeIdGenerator->reset();
        nodeIdGenerator->getId(rootId);
    }

    template <typename TrieVal>
    size_t BaseTrie<TrieVal>::size() {
        return vals.size();
    }

    template <typename TrieVal>
    void BaseTrie<TrieVal>::build(const std::optional<std::unordered_map<std::string, TrieVal>> &data) {
        clear();
        if (!data.has_value()) return;

        for (const auto& [key, val] : *data) {
            addKeyword(key, val);
        }
    }

    template <typename TrieVal>
    bool BaseTrie<TrieVal>::add(std::string_view key, const TrieVal &val) {
        return addKeyword(key, val);
    }

    template <typename TrieVal>
    bool BaseTrie<TrieVal>::add(const std::string &key, const TrieVal &val) {
        return add(std::string_view(key), val);
    }

    template <typename TrieVal>
    void BaseTrie<TrieVal>::initNodeIdGenerator() {
        nodeIdGenerator = std::make_unique<IncIdGenerator<std::uint64_t>>(DefaultNodeId);
    }

    template <typename TrieVal>
    void BaseTrie<TrieVal>::initKeywordIdGenerator() {
        keywordIdGenerator = std::make_unique<IncIdGenerator<std::uint64_t>>(DefaultNodeId);
    }

    template <typename TrieVal>
    std::int8_t BaseTrie<TrieVal>::comparekeywordIds(const std::uint64_t &keywordId1, const std::uint64_t &keywordId2) {
        const std::string *keywordPtr1 = keywords.findValuePtr(keywordId1);
        const std::string *keywordPtr2 = keywords.findValuePtr(keywordId2);

        if (keywordPtr1 == nullptr && keywordPtr2 == nullptr) {
            return 0;
        } else if (keywordPtr1 == nullptr) {
            return 1;
        } else if (keywordPtr2 == nullptr) {
            return -1;
        }

        size_t keyword1Len = keywordPtr1->size();
        size_t keyword2Len = keywordPtr2->size();

        if (keyword1Len > keyword2Len) {
            return -1;
        } else if (keyword1Len < keyword2Len) {
            return 1;
        }

        return StringTools::compareStrPtrs(keywordPtr1, keywordPtr2);
    }

    template <typename TrieVal>
    bool BaseTrie<TrieVal>::addKeyword(std::string_view keyword, const TrieVal &val) {
        std::uint64_t prevNodeId = rootId;
        bool newKeyword = false;
        std::unordered_map<std::string, std::uint64_t, StringViewHash, std::equal_to<void>> *prevChildren;

        for (std::string_view grapheme : GraphemeRange(keyword)) {
            auto [prevChildrenKVP, prevChildrenExists] = children.try_emplace(prevNodeId);
            prevChildren = &(prevChildrenKVP->second);

            auto nodeIdKVP = prevChildren->find(grapheme);
            if (nodeIdKVP != prevChildren->end()) {
                prevNodeId = nodeIdKVP->second;
                continue;
            }

            if (!newKeyword) {
                newKeyword = true;
            }

            auto graphemeStr = std::string(grapheme);
            nodeIdGenerator->getId(prevNodeId);
            (*prevChildren)[graphemeStr] = prevNodeId;
        }

        // if the keyword to be inserted is a proper prefix of some keyword that
        //      already exists in the trie
        if (!newKeyword && !keywords.containsValue(keyword)) {
            newKeyword = true;
        }

        // add the KVP
        std::uint64_t keywordId;
        if (newKeyword) {
            keywordId = addKVP(keyword, val);
            auto [it, inserted] = nodeKeywordIds.try_emplace(prevNodeId);
            
            if (inserted) {
                it->second.emplace_back(keywordId);
                acceptNodeIds.insert(prevNodeId);
            } else {
                std::vector<std::uint64_t> &foundKeywordIds = it->second;
                Algo::binaryInsert(foundKeywordIds, keywordId, 
                                    [this](const auto& k1, const auto& k2) {
                                        return comparekeywordIds(k1, k2);
                                    }, true);
            }

        } else {
            keywordId = keywords.getKey(keyword);

            if (auto it = vals.find(keywordId); it != vals.end()) {
                it->second = handleDuplicate(keyword, it->second, val);
            } else {
                vals[keywordId] = val; 
            }
        }

        return newKeyword;
    }

    template <typename TrieVal>
    bool BaseTrie<TrieVal>::addKeyword(const std::string &keyword, const TrieVal &val) {
        return addKeyword(std::string_view(keyword), val);
    }

    template <typename TrieVal>
    std::uint64_t BaseTrie<TrieVal>::addKVP(std::string_view keyword, const TrieVal &val) {
        const std::uint64_t *keywordIdPtr = keywords.findKeyPtr(keyword);
        if (keywordIdPtr != nullptr) {
            vals[*keywordIdPtr] = handleDuplicate(keyword, vals.at(*keywordIdPtr), val);
            return *keywordIdPtr;
        }

        std::uint64_t keywordId;
        keywordIdGenerator->getId(keywordId);

        keywords.insert(keywordId, std::string(keyword));
        vals[keywordId] = val;
        return keywordId;        
    }

    template <typename TrieVal>
    std::uint64_t BaseTrie<TrieVal>::addKVP(const std::string &keyword, const TrieVal &val) {
        return addKVP(std::string_view(keyword), val);
    }

    template <typename TrieVal>
    bool BaseTrie<TrieVal>::contains(std::string_view keyword) {
        const std::uint64_t *keywordIdPtr = keywords.findKeyPtr(keyword);
        return keywordIdPtr != nullptr;
    }

    template <typename TrieVal>
    bool BaseTrie<TrieVal>::contains(const std::string &keyword) {
        return contains(std::string_view(keyword));
    }

    template <typename TrieVal>
    const TrieVal* BaseTrie<TrieVal>::getPtr(std::string_view keyword) {
        std::uint64_t prevNodeId = rootId;
        std::unordered_map<std::string, std::uint64_t, StringViewHash, std::equal_to<void>> *prevChildren;
        bool error = false;

        for (std::string_view grapheme : GraphemeRange(keyword)) {
            auto prevChildrenKVP = children.find(prevNodeId);
            if (prevChildrenKVP == children.end()) {
                error = true;
                break;
            }

            prevChildren = &(prevChildrenKVP->second);

            auto nodeKVP = prevChildren->find(grapheme);
            if (nodeKVP == prevChildren->end()) {
                error = true;
                break;
            }

            prevNodeId = nodeKVP->second;
        }

        // when there is no output at the reached node
        auto keywordIdsKVP = nodeKeywordIds.find(prevNodeId);
        if (keywordIdsKVP == nodeKeywordIds.end()) {
            error = true;
        }

        if (error) return nullptr;

        std::uint64_t keywordId = (keywordIdsKVP->second)[0];
        return &(vals.at(keywordId));
    }

    template <typename TrieVal>
    const TrieVal* BaseTrie<TrieVal>::getPtr(const std::string &keyword) {
        return getPtr(std::string_view(keyword));
    }

    template <typename TrieVal>
    std::optional<TrieVal> BaseTrie<TrieVal>::get(std::string_view keyword) {
        const TrieVal *resultPtr = getPtr(keyword);

        if (resultPtr == nullptr) return std::nullopt;
        return *resultPtr;
    }

    template <typename TrieVal>
    std::optional<TrieVal> BaseTrie<TrieVal>::get(const std::string &keyword) {
        const TrieVal *resultPtr = getPtr(keyword);

        if (resultPtr == nullptr) return std::nullopt;
        return *resultPtr;
    }

    template <typename TrieVal>
    const TrieVal* BaseTrie<TrieVal>::getKeyValPtr(std::string_view keyword) {
        const std::uint64_t *keywordIdPtr = keywords.findKeyPtr(keyword);
        if (keywordIdPtr == nullptr) {
            return nullptr;
        }

        return &(vals.at(*keywordIdPtr));
    }

    template <typename TrieVal>
    const TrieVal* BaseTrie<TrieVal>::getKeyValPtr(const std::string &keyword) {
        return getKeyValPtr(std::string_view(keyword));
    }

    template <typename TrieVal>
    std::optional<TrieVal> BaseTrie<TrieVal>::getKeyVal(std::string_view keyword) {
        const TrieVal *resultPtr = getKeyValPtr(keyword);

        if (resultPtr == nullptr) return std::nullopt;
        return *resultPtr;
    }

    template <typename TrieVal>
    std::optional<TrieVal> BaseTrie<TrieVal>::getKeyVal(const std::string &keyword) {
        const TrieVal *resultPtr = getKeyValPtr(keyword);

        if (resultPtr == nullptr) return std::nullopt;
        return *resultPtr;
    }
}