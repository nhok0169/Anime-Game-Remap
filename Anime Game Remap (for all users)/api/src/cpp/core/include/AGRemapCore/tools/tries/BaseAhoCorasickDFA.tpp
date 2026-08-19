#include "AGRemapCore/tools/tries/BaseAhoCorasickDFA.h"
#include "AGRemapCore/tools/StringTools.h"

#include <queue>
#include <iostream>


namespace AGRemapCore {
    template <typename TrieVal>
    BaseAhoCorasickDFA<TrieVal>::BaseAhoCorasickDFA(const std::optional<std::unordered_map<std::string, TrieVal>> &data, const std::optional<std::variant<DupHandler, DupHandler2>>& handler): BaseTrie<TrieVal>(data, handler) {

    }

    template <typename TrieVal>
    void BaseAhoCorasickDFA<TrieVal>::clear() {
        BaseTrie<TrieVal>::clear();
        fail.clear();
    }

    template <typename TrieVal>
    bool BaseAhoCorasickDFA<TrieVal>::contains(const std::string &txt) {
        size_t ind;
        const std::string *keywordPtr = findPtr(txt, &ind);
        return keywordPtr != nullptr;
    }

    template <typename TrieVal>
    void BaseAhoCorasickDFA<TrieVal>::build(const std::optional<std::unordered_map<std::string, TrieVal>> &data) {
        BaseTrie<TrieVal>::build(data);

        std::unordered_map<std::string, std::uint64_t, StringViewHash, std::equal_to<void>> *nodeChildrenIds;
        auto childrenKVP = this->children.find(this->rootId);

        // no keywords added
        if (childrenKVP == this->children.end()) return;

        // all depth 1 children in the trie have a failure
        //   function that returns to the root
        nodeChildrenIds = &(childrenKVP->second);
        for (const auto &[letter, childId]: *nodeChildrenIds) {
            fail[childId] = this->rootId;
        }

        // BFS to complete the failure function and the output results
        std::unordered_set<std::uint64_t> visitedNodeIds;
        std::queue<std::uint64_t> nodeIdQueue;
        std::uint64_t nodeId;
        std::optional<std::uint64_t> failureId;
        std::optional<std::uint64_t> currentFailureId;
        std::optional<std::uint64_t> childFailureId = std::nullopt;
        std::unordered_map<std::string, std::uint64_t, StringViewHash, std::equal_to<void>> *childrenFailureIds;
        bool failureIdHasVal;

        visitedNodeIds.emplace(this->rootId);
        nodeIdQueue.push(this->rootId);

        while (!nodeIdQueue.empty()) {
            nodeId = nodeIdQueue.front();
            nodeIdQueue.pop();

            childrenKVP = this->children.find(nodeId);
            if (childrenKVP == this->children.end()) continue;

            // should be able to get the failure of every node
            // except for the root node
            auto failureKVP = fail.find(nodeId);
            if (failureKVP == fail.end() && nodeId != this->rootId) continue;

            nodeChildrenIds = &(childrenKVP->second);
            
            if (failureKVP == fail.end()) {
                failureId = std::nullopt;
            } else {
                failureId = failureKVP->second;
            }

            failureIdHasVal = failureId.has_value();

            for (const auto &[letter, childId]: *nodeChildrenIds) {
                if (visitedNodeIds.contains(childId)) continue;

                visitedNodeIds.emplace(childId);
                nodeIdQueue.push(childId);

                if (!failureIdHasVal) {
                    currentFailureId = std::nullopt;
                    childFailureId = std::nullopt;

                } else {
                    currentFailureId = failureId;
                    childrenKVP = this->children.find(*currentFailureId);
                    if (childrenKVP == this->children.end()) {
                        childFailureId = std::nullopt;
                    } else {
                        childrenFailureIds = &(childrenKVP->second);
                        auto childFailureKVP = childrenFailureIds->find(letter);

                        if (childFailureKVP == childrenFailureIds->end()) {
                            childFailureId = std::nullopt;
                        } else {
                            childFailureId = childFailureKVP->second;
                        }
                    }
                }

                // Failure node is the node that forms the longest proper suffix
                //     with the current substring read
                //  Note: Longest proper suffix is the prefix of some keyword
                while (currentFailureId.has_value() && *currentFailureId != this->rootId && !childFailureId.has_value()) {
                    failureKVP = fail.find(*currentFailureId);
                    if (failureKVP == fail.end()) {
                        currentFailureId = std::nullopt;
                    } else {
                        currentFailureId = failureKVP->second;
                    }

                    childrenKVP = this->children.find(*currentFailureId);
                    if (childrenKVP == this->children.end()) {
                        childFailureId = std::nullopt;
                        continue;
                    }

                    childrenFailureIds = &(childrenKVP->second);
                    auto childFailureKVP = childrenFailureIds->find(letter);

                    if (childFailureKVP == childrenFailureIds->end()) {
                        childFailureId = std::nullopt;
                    } else {
                        childFailureId = childFailureKVP->second;
                    }
                }

                // default failure node if no other keyword has a proper prefix
                //   that matches the proper suffix of the current substring read
                if (!childFailureId.has_value()) {
                    childFailureId = this->rootId;
                }

                fail[childId] = *childFailureId;
                
                auto childOutKVP = this->nodeKeywordIds.find(childId);
                auto childFailureOutKVP = this->nodeKeywordIds.find(*childFailureId);

                if (childOutKVP == this->nodeKeywordIds.end() && childFailureOutKVP == this->nodeKeywordIds.end()) {
                    this->nodeKeywordIds[childId];
                } else if (childOutKVP == this->nodeKeywordIds.end()) {
                    this->nodeKeywordIds[childId] = childFailureOutKVP->second;
                } else if (childFailureOutKVP != this->nodeKeywordIds.end()) {
                    std::vector<const std::vector<std::uint64_t>*> sortedLsts = {&(childOutKVP->second), &(childFailureOutKVP->second)};
                    std::vector<std::uint64_t> mergedLst;

                    Algo::merge(sortedLsts, 
                                [this](const auto& k1, const auto& k2) {
                                    return this->comparekeywordIds(k1, k2);
                                }, mergedLst);

                    this->nodeKeywordIds[childId] = std::move(mergedLst);
                }
            }
        }
    }

    template <typename TrieVal>
    bool BaseAhoCorasickDFA<TrieVal>::add(std::string_view key, const TrieVal &val) {
        std::unordered_map<std::string, TrieVal> data;

        for (const auto &[keywordId, currentKeyword]: this->keywords) {
            auto valKVP = this->vals.find(keywordId);
            if (valKVP == this->vals.end()) continue;
            data[currentKeyword] = valKVP->second;
        }

        std::string keyStr(key); 
        auto it = data.find(keyStr);
        bool insertSuccess = (it == data.end());

        if (insertSuccess) {
            data.emplace(std::move(keyStr), val);
        } else {
            it->second = this->handleDuplicate(key, it->second, val);
        }

        build(data);
        return insertSuccess;
    }

    template <typename TrieVal>
    bool BaseAhoCorasickDFA<TrieVal>::add(const std::string &key, const TrieVal &val) {
        return add(std::string_view(key), val);
    }

    template <typename TrieVal>
    std::uint64_t BaseAhoCorasickDFA<TrieVal>::getNextState(std::uint64_t currentStateId, std::string_view letter, bool *resIsFail) {
        std::unordered_map<std::string, std::uint64_t, StringViewHash, std::equal_to<void>> *nextStateChildren;
        std::optional<std::uint64_t> nextStateId;

        auto childrenKVP = this->children.find(currentStateId);
        if (childrenKVP == this->children.end()) {
            nextStateId = std::nullopt;
        } else {
            nextStateChildren = &(childrenKVP->second);
            auto nextStateChildrenKVP = nextStateChildren->find(letter);

            if (nextStateChildrenKVP == nextStateChildren->end()) {
                nextStateId = std::nullopt;
            } else {
                nextStateId = nextStateChildrenKVP->second;
            }
        }

        bool isFail = false;

        while (!nextStateId.has_value() && currentStateId != this->rootId) {
            auto failKVP = fail.find(currentStateId);
            if (failKVP == fail.end()) {
                currentStateId = this->rootId;
            } else {
                currentStateId = failKVP->second;
            }

            childrenKVP = this->children.find(currentStateId);
            if (childrenKVP == this->children.end()) {
                nextStateId = std::nullopt;
            } else {
                nextStateChildren = &(childrenKVP->second);
                auto nextStateChildrenKVP = nextStateChildren->find(letter);

                if (nextStateChildrenKVP == nextStateChildren->end()) {
                    nextStateId = std::nullopt;
                } else {
                    nextStateId = nextStateChildrenKVP->second;
                }
            }

            if (!isFail) {
                isFail = true;
            }
        }

        if (!nextStateId.has_value()) {
            nextStateId = this->rootId;
            isFail = true;
        }

        *resIsFail = isFail;
        return *nextStateId;
    }

    template <typename TrieVal>
    std::unordered_map<std::string, std::vector<std::tuple<size_t, size_t>>> BaseAhoCorasickDFA<TrieVal>::findAll(const std::string_view txt) {
        std::unordered_map<std::string, std::vector<std::tuple<size_t, size_t>>> result;
        std::vector<std::uint64_t> *currentKeywords;
        std::uint64_t stateId = this->rootId;
        std::string_view emptyStr = "";
        size_t i = 0;
        const std::string *keyword = nullptr;
        bool isFail;

        stateId = getNextState(stateId, emptyStr, &isFail);
        auto currentKeywordsKVP = this->nodeKeywordIds.find(stateId);
        if (currentKeywordsKVP != this->nodeKeywordIds.end()) {
            currentKeywords = &(currentKeywordsKVP->second);

            for (const std::uint64_t &keywordId: *currentKeywords) {
                keyword = this->keywords.findValuePtr(keywordId);
                result[*keyword].emplace_back(-1 - StringTools::countGrapheme(*keyword) + 1, 0);
            }
        }

        for (std::string_view grapheme : GraphemeRange(txt)) {
            stateId = getNextState(stateId, grapheme, &isFail);
            currentKeywordsKVP = this->nodeKeywordIds.find(stateId);
            if (currentKeywordsKVP == this->nodeKeywordIds.end()) {
                i++;
                continue;
            }

            currentKeywords = &(currentKeywordsKVP->second);

            for (const std::uint64_t &keywordId: *currentKeywords) {
                keyword = this->keywords.findValuePtr(keywordId);
                result[*keyword].emplace_back(i - StringTools::countGrapheme(*keyword) + 1, i + 1);
            }

            i++;
        }

        return result;
    }

    template <typename TrieVal>
    std::unordered_map<std::string, std::vector<std::tuple<size_t, size_t>>> BaseAhoCorasickDFA<TrieVal>::findAll(const std::string &txt) {
        return findAll(std::string_view(txt));
    }

    template <typename TrieVal>
    std::unordered_map<std::string, std::tuple<size_t, size_t>> BaseAhoCorasickDFA<TrieVal>::findFirstAll(std::string_view txt) {
        std::unordered_map<std::string, std::tuple<size_t, size_t>> result;
        std::vector<std::uint64_t> *currentKeywords;
        std::uint64_t stateId = this->rootId;
        std::string_view emptyStr = "";
        size_t i = 0;
        const std::string *keyword = nullptr;
        size_t keywordsLen = this->keywords.size();
        bool isFail;

        stateId = getNextState(stateId, emptyStr, &isFail);
        auto currentKeywordsKVP = this->nodeKeywordIds.find(stateId);
        if (currentKeywordsKVP != this->nodeKeywordIds.end()) {
            currentKeywords = &(currentKeywordsKVP->second);

            for (const std::uint64_t &keywordId: *currentKeywords) {
                keyword = this->keywords.findValuePtr(keywordId);
                if (result.contains(*keyword)) continue;

                result[*keyword] = std::make_tuple(-1 - StringTools::countGrapheme(*keyword) + 1, 0);
                if (result.size() == keywordsLen) return result;
            }
        }

        for (std::string_view grapheme : GraphemeRange(txt)) {
            stateId = getNextState(stateId, grapheme, &isFail);
            currentKeywordsKVP = this->nodeKeywordIds.find(stateId);
            if (currentKeywordsKVP == this->nodeKeywordIds.end()) {
                i++;
                continue;
            }

            currentKeywords = &(currentKeywordsKVP->second);

            for (const std::uint64_t &keywordId: *currentKeywords) {
                keyword = this->keywords.findValuePtr(keywordId);
                if (result.contains(*keyword)) continue;

                result[*keyword] = std::make_tuple(i - StringTools::countGrapheme(*keyword) + 1, i + 1);
                if (result.size() == keywordsLen) return result;
            }

            i++;
        }

        return result;
    }

    template <typename TrieVal>
    std::unordered_map<std::string, std::tuple<size_t, size_t>> BaseAhoCorasickDFA<TrieVal>::findFirstAll(const std::string &txt) {
        return findFirstAll(std::string_view(txt));
    }

    template <typename TrieVal>
    const std::string *BaseAhoCorasickDFA<TrieVal>::findPtr(std::string_view txt, size_t *resultInd) {
        std::uint64_t keywordId;
        const std::string *keyword = nullptr;
        size_t keywordInd = 0;
        size_t i = 0;
        std::uint64_t stateId = this->rootId;
        bool isFail;
        std::string_view emptyStr = "";

        stateId = getNextState(stateId, emptyStr, &isFail);
        auto currentKeywordsKVP = this->nodeKeywordIds.find(stateId);
        if (currentKeywordsKVP != this->nodeKeywordIds.end() && !(currentKeywordsKVP->second).empty()) {
            keywordId = (currentKeywordsKVP->second)[0];
            keyword = this->keywords.findValuePtr(keywordId);
            *resultInd = -1 - StringTools::countGrapheme(*keyword) + 1;
            return keyword;
        }
        
        for (std::string_view grapheme : GraphemeRange(txt)) {
            stateId = getNextState(stateId, grapheme, &isFail);

            currentKeywordsKVP = this->nodeKeywordIds.find(stateId);
            if (currentKeywordsKVP != this->nodeKeywordIds.end() && !(currentKeywordsKVP->second).empty()) {
                keywordId = (currentKeywordsKVP->second)[0];
                keyword = this->keywords.findValuePtr(keywordId);
                *resultInd = i - StringTools::countGrapheme(*keyword) + 1;
                return keyword;
            }

            i++;
        }

        return nullptr;
    }

    template <typename TrieVal>
    const std::string *BaseAhoCorasickDFA<TrieVal>::findPtr(const std::string &txt, size_t *resultInd) {
        return findPtr(std::string_view(txt), resultInd);
    }

    static void raiseNoKeywordsFound(std::string_view txt) {
        throw std::out_of_range("No keywords found in the AhoCorasickDFA for the given text: " + std::string(txt));
    }

    template <typename TrieVal>
    const std::string &BaseAhoCorasickDFA<TrieVal>::find(std::string_view txt, size_t *resultInd) {
        const std::string *result = findPtr(txt, resultInd);

        if (result == nullptr) {
            raiseNoKeywordsFound(txt);
        }

        return *result;
    }

    template <typename TrieVal>
    const std::string &BaseAhoCorasickDFA<TrieVal>::find(const std::string &txt, size_t *resultInd) {
        return find(std::string_view(txt), resultInd);
    }

    template <typename TrieVal>
    const std::string *BaseAhoCorasickDFA<TrieVal>::findMaximalPtr(std::string_view txt, size_t *resultInd) {
        const std::string *keyword = nullptr;
        size_t keywordInd = 0;
        std::uint64_t stateId = this->rootId;
        std::string_view emptyStr = "";
        bool isFail = false;
        size_t i = 0;
        std::vector<std::uint64_t> *currentKeywords;

        stateId = getNextState(stateId, emptyStr, &isFail);
        if (keyword != nullptr && isFail) {
            *resultInd = keywordInd;
            return keyword;
        }

        if (!(keyword != nullptr && !keyword->empty() && !this->acceptNodeIds.contains(stateId))) {
            auto currentKeywordsKVP = this->nodeKeywordIds.find(stateId);
            if (currentKeywordsKVP != this->nodeKeywordIds.end() && !currentKeywordsKVP->second.empty()) {
                currentKeywords = &(currentKeywordsKVP->second);
                keyword = this->keywords.findValuePtr(currentKeywords->front());
                keywordInd = -1 - StringTools::countGrapheme(*keyword) + 1;
            }
        }

        for (std::string_view grapheme : GraphemeRange(txt)) {
            stateId = getNextState(stateId, grapheme, &isFail);
            if (keyword != nullptr && isFail) {
                break;
            }

            if (keyword != nullptr && !keyword->empty() && !this->acceptNodeIds.contains(stateId)) {
                i++;
                continue;
            }

            auto currentKeywordsKVP = this->nodeKeywordIds.find(stateId);
            if (currentKeywordsKVP != this->nodeKeywordIds.end() && !currentKeywordsKVP->second.empty()) {
                currentKeywords = &(currentKeywordsKVP->second);
                keyword = this->keywords.findValuePtr(currentKeywords->front());
                keywordInd = i - StringTools::countGrapheme(*keyword) + 1;
            }

            i++;
        }

        *resultInd = keywordInd;
        return keyword;
    }

    template <typename TrieVal>
    const std::string *BaseAhoCorasickDFA<TrieVal>::findMaximalPtr(const std::string &txt, size_t *resultInd) {
        return findMaximalPtr(std::string_view(txt), resultInd);
    }

    template <typename TrieVal>
    const std::string &BaseAhoCorasickDFA<TrieVal>::findMaximal(std::string_view txt, size_t *resultInd) {
        const std::string *result = findMaximalPtr(txt, resultInd);
        if (result == nullptr) {
            raiseNoKeywordsFound(txt);
        }

        return *result;
    }

    template <typename TrieVal>
    const std::string &BaseAhoCorasickDFA<TrieVal>::findMaximal(const std::string &txt, size_t *resultInd) {
        return findMaximal(std::string_view(txt), resultInd);
    }

    template <typename TrieVal>
    std::tuple<std::vector<std::string_view>, std::vector<size_t>> BaseAhoCorasickDFA<TrieVal>::findMaximal(std::string_view txt, size_t count) {
        std::vector<std::string_view> keywordLst;
        std::vector<size_t> keywordIndLst;
        const std::string *keyword;
        size_t keywordInd;

        if (count <= 1) {
            keyword = findMaximalPtr(txt, &keywordInd);
            if (keyword == nullptr) return { keywordLst, keywordIndLst };

            keywordLst.push_back(*keyword);
            keywordIndLst.push_back(keywordInd);
            return { keywordLst, keywordIndLst };
        }

        size_t currentTxtInd = 0;
        size_t txtLen = txt.size();
        size_t numOfFoundKeywords = count;
        std::string_view remainingTxt;

        while (currentTxtInd < txtLen && numOfFoundKeywords > 0) {
            remainingTxt = txt.substr(currentTxtInd);
            keyword = findMaximalPtr(remainingTxt, &keywordInd);
            if (keyword == nullptr) break;

            keywordLst.push_back(*keyword);
            keywordIndLst.push_back(currentTxtInd + keywordInd);

            if (keyword->empty()) {
                GraphemeIterator it(remainingTxt, 0);
                currentTxtInd += (*it).size();
            } else {
                currentTxtInd += keywordInd + StringTools::countGrapheme(*keyword);
            }

            numOfFoundKeywords--;
        }

        std::string emptyStr = "";
        auto [emptyId, emptyVal] = this->keywords.findKVPPtrByVal(emptyStr);

        if (emptyId != nullptr && numOfFoundKeywords) {
            keywordLst.push_back(*emptyVal);
            keywordIndLst.push_back(txtLen);
        }

        return { keywordLst, keywordIndLst };
    }

    template <typename TrieVal>
    std::tuple<std::vector<std::string_view>, std::vector<size_t>> BaseAhoCorasickDFA<TrieVal>::findMaximal(const std::string &txt, size_t count) {
        return findMaximal(std::string_view(txt), count);
    }

    // std::tuple<const TrieVal *, const std::string *> getPtr(const std::string &txt);
    // std::tuple<const TrieVal &, const std::string &> get(const std::string &txt);

    template <typename TrieVal>
    std::tuple<const std::string *, const TrieVal *> BaseAhoCorasickDFA<TrieVal>::getMaximalPtr(std::string_view txt) {
        size_t keywordInd;
        const std::string *keyword = findMaximalPtr(txt, &keywordInd);

        if (keyword == nullptr) {
            return std::make_tuple(nullptr, nullptr);
        }

        const std::uint64_t &keywordId = this->keywords.getKey(*keyword);
        const TrieVal* val = &(this->vals.at(keywordId));
        return std::make_tuple(keyword, val);
    }

    template <typename TrieVal>
    std::tuple<const std::string *, const TrieVal *> BaseAhoCorasickDFA<TrieVal>::getKVPPtr(std::string_view txt) {
        size_t keywordInd;
        const std::string *keywordPtr = findPtr(txt, &keywordInd);

        if (keywordPtr == nullptr) {
            return std::make_tuple(nullptr, nullptr);
        }

        const std::uint64_t &keywordId = this->keywords.getKey(*keywordPtr);
        const TrieVal *valPtr = &(this->vals.at(keywordId));
        return std::make_tuple(keywordPtr, valPtr);
    }

    template <typename TrieVal>
    std::tuple<const std::string *, const TrieVal *> BaseAhoCorasickDFA<TrieVal>::getKVPPtr(const std::string &txt) {
        return getKVPPtr(std::string_view(txt));
    }

    template <typename TrieVal>
    std::tuple<const std::string &, const TrieVal &> BaseAhoCorasickDFA<TrieVal>::getKVP(std::string_view txt) {
        auto [keywordPtr, valPtr] = getKVPPtr(txt);
        if (keywordPtr == nullptr) {
            raiseNoKeywordsFound(txt);
        }

        return std::tie(*keywordPtr, *valPtr);
    }

    template <typename TrieVal>
    std::tuple<const std::string &, const TrieVal &> BaseAhoCorasickDFA<TrieVal>::getKVP(const std::string &txt) {
        return getKVP(std::string_view(txt));
    }

    template <typename TrieVal>
    std::unordered_map<std::string, const TrieVal *> BaseAhoCorasickDFA<TrieVal>::getAll(std::string_view txt) {
        std::unordered_map<std::string, const TrieVal *> result;
        std::vector<std::uint64_t> *currentKeywords;
        std::uint64_t stateId = this->rootId;
        std::string_view emptyStr = "";
        size_t i = 0;
        const std::string *keyword = nullptr;
        size_t keywordsLen = this->keywords.size();
        bool isFail;

        stateId = getNextState(stateId, emptyStr, &isFail);
        auto currentKeywordsKVP = this->nodeKeywordIds.find(stateId);
        if (currentKeywordsKVP != this->nodeKeywordIds.end()) {
            currentKeywords = &(currentKeywordsKVP->second);

            for (const std::uint64_t &keywordId: *currentKeywords) {
                keyword = this->keywords.findValuePtr(keywordId);
                if (result.contains(*keyword)) continue;

                result[*keyword] = &(this->vals.at(keywordId));
                if (result.size() == keywordsLen) return result;
            }
        }

        for (std::string_view grapheme : GraphemeRange(txt)) {
            stateId = getNextState(stateId, grapheme, &isFail);
            currentKeywordsKVP = this->nodeKeywordIds.find(stateId);
            if (currentKeywordsKVP == this->nodeKeywordIds.end()) {
                i++;
                continue;
            }

            currentKeywords = &(currentKeywordsKVP->second);

            for (const std::uint64_t &keywordId: *currentKeywords) {
                keyword = this->keywords.findValuePtr(keywordId);
                if (result.contains(*keyword)) continue;

                result[*keyword] = &(this->vals.at(keywordId));
                if (result.size() == keywordsLen) return result;
            }

            i++;
        }

        return result;
    }

    template <typename TrieVal>
    std::unordered_map<std::string, const TrieVal *> BaseAhoCorasickDFA<TrieVal>::getAll(const std::string &txt) {
        return getAll(std::string_view(txt));
    }

    template <typename TrieVal>
    std::tuple<const std::string *, const TrieVal *> BaseAhoCorasickDFA<TrieVal>::getMaximalPtr(const std::string &txt) {
        return getMaximalPtr(std::string_view(txt));
    }

    template <typename TrieVal>
    std::tuple<const std::string &, const TrieVal &> BaseAhoCorasickDFA<TrieVal>::getMaximal(std::string_view txt) {
        auto [keyword, val] = getMaximalPtr(txt);

        if (keyword == nullptr) {
            raiseNoKeywordsFound(txt);
        }

        return std::tie(*keyword, *val);
    }

    template <typename TrieVal>
    std::tuple<const std::string &, const TrieVal &> BaseAhoCorasickDFA<TrieVal>::getMaximal(const std::string &txt) {
        return getMaximal(std::string_view(txt));
    }

    template <typename TrieVal>
    std::tuple<std::vector<std::string_view>, std::vector<const TrieVal *>> BaseAhoCorasickDFA<TrieVal>::getMaximal(std::string_view txt, size_t count) {
        std::vector<const TrieVal *> resVals;

        if (count <= 1) {
            std::vector<std::string_view> resKeywords;

            auto [keywordPtr, valPtr] = getMaximalPtr(txt);
            if (keywordPtr == nullptr) return { resKeywords, resVals };

            resKeywords.push_back(*keywordPtr);
            resVals.push_back(valPtr);
            return { resKeywords, resVals };
        }

        auto [resKeywords, resKeywordInds] = findMaximal(txt, count);
        const std::uint64_t *keywordIdPtr;
        
        for (const auto &keyword: resKeywords) {
            keywordIdPtr = this->keywords.findKeyPtr(keyword);
            resVals.push_back(&(this->vals.at(*keywordIdPtr)));
        }

        return { resKeywords, resVals };
    }

    template <typename TrieVal>
    std::tuple<std::vector<std::string_view>, std::vector<const TrieVal *>> BaseAhoCorasickDFA<TrieVal>::getMaximal(const std::string &txt, size_t count) {
        return getMaximal(std::string_view(txt), count);
    }

    template <typename TrieVal>
    const std::string * BaseAhoCorasickDFA<TrieVal>::maximalStartsWithPtr(std::string_view txt) {
        std::uint64_t prevNodeId = this->rootId;
        std::uint64_t nodeId;
        std::string currentPrefix;
        const std::unordered_map<std::string, std::uint64_t, StringViewHash, std::equal_to<void>> *nodeChildren;

        const std::string *prefixPtr;
        const std::uint64_t *prefixIdPtr;
        const std::string *resPrefixPtr = nullptr;

        for (std::string_view grapheme : GraphemeRange(txt)) {
            auto nodeChildrenKVP = this->children.find(prevNodeId);
            if (nodeChildrenKVP == this->children.end()) break;

            nodeChildren = &(nodeChildrenKVP->second);
            auto nodeIdKVP = nodeChildren->find(grapheme);
            if (nodeIdKVP == nodeChildren->end()) break;

            nodeId = nodeIdKVP->second;
            prevNodeId = nodeId;

            currentPrefix += grapheme;
            std::tie(prefixIdPtr, prefixPtr) = this->keywords.findKVPPtrByVal(currentPrefix);
            if (prefixIdPtr != nullptr) {
                resPrefixPtr = prefixPtr;
            }
        }

        if (resPrefixPtr != nullptr) return resPrefixPtr;

        std::string emptyStr = "";
        auto [emptyIdPtr, emptyStrPtr] = this->keywords.findKVPPtrByVal(emptyStr);
        return emptyStrPtr;
    }

    template <typename TrieVal>
    const std::string * BaseAhoCorasickDFA<TrieVal>::maximalStartsWithPtr(const std::string &txt) {
        return maximalStartsWithPtr(std::string_view(txt));
    }

    template <typename TrieVal>
    const std::string & BaseAhoCorasickDFA<TrieVal>::maximalStartsWith(std::string_view txt) {
        auto resultPtr = maximalStartsWithPtr(txt);
        if (resultPtr == nullptr) {
            raiseNoKeywordsFound(txt);
        }

        return *resultPtr;
    }

    template <typename TrieVal>
    const std::string & BaseAhoCorasickDFA<TrieVal>::maximalStartsWith(const std::string &txt) {
        return maximalStartsWith(std::string_view(txt));
    }
}