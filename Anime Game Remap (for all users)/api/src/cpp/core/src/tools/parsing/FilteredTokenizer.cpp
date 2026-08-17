#include "AGRemapCore/tools/parsing/FilteredTokenizer.h"


namespace AGRemapCore {
    FilteredTokenizer::FilteredTokenizer(std::unordered_map<std::string, std::string> tokens, std::unordered_set<std::string> keywordTokenIds, std::unordered_set<std::string> filteredTokenIds, bool setup):
        // Always constructs the base with setup=false, then calls this->setup() itself below (if
        // requested) once this object is a genuine FilteredTokenizer, not a BaseTokenizer under
        // construction -- see BaseTokenizer.h's own note on why a virtual call from within a base
        // class's constructor can never reach a derived override, trampoline or not.
        BaseTokenizer(std::move(tokens), false),
        keywordTokenIds_(std::move(keywordTokenIds)), filteredTokenIds_(std::move(filteredTokenIds)) {

        if (setup) {
            this->setup();
        }
    }

    void FilteredTokenizer::setup() {
        clear();

        for (const std::string &keywordId : keywordTokenIds_) {
            addKeyword(keywordId);
        }

        addStates();
        addTransitions();
    }

    bool FilteredTokenizer::acceptToken(const std::string &token, const std::string &stateId, bool isAccept, std::vector<Token> &result, size_t lineNo, size_t charNo, bool includeFiltered) {
        if (!isAccept) {
            return false;
        }

        auto tokenTypeKVP = tokens_.find(stateId);
        if (tokenTypeKVP == tokens_.end()) {
            return false;
        }

        if (includeFiltered || filteredTokenIds_.find(stateId) == filteredTokenIds_.end()) {
            result.emplace_back(tokenTypeKVP->second, token, lineNo, charNo);
        }

        dfa.reset();
        return true;
    }

    const std::unordered_set<std::string>& FilteredTokenizer::keywordTokenIds() const {
        return keywordTokenIds_;
    }

    const std::unordered_set<std::string>& FilteredTokenizer::filteredTokenIds() const {
        return filteredTokenIds_;
    }
}
