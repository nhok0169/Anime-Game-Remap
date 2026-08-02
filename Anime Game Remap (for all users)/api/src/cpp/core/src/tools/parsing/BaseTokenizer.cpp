#include "AGRemapCore/tools/parsing/BaseTokenizer.h"
#include "AGRemapCore/tools/grapheme/GraphemeRange.h"
#include "AGRemapCore/tools/StringTools.h"


namespace AGRemapCore {
    BaseTokenizer::BaseTokenizer(std::unordered_map<std::string, std::string>&& tokens, bool setup): tokens(std::move(tokens))  {

    }

    // BaseTokenizer::BaseTokenizer(std::shared_ptr<std::unordered_map<std::string, std::string>> tokens, bool setup): tokens(std::move(tokens)) {

    // }

    void BaseTokenizer::clear() {
        dfa.clear();
    }

    void BaseTokenizer::reset() {
        dfa.reset();
    }

    bool BaseTokenizer::addStartState() {
        return dfa.addState(startStateId, std::nullopt, true);
    }

    bool BaseTokenizer::addKeyword(const std::string &keyword) {
        bool result = addStartState();
        std::string prevStateId = startStateId;
        std::string stateId = startStateId;
        std::optional<bool> isAccept;
        size_t i = 0;
        size_t keywordsLen = StringTools::countGrapheme(keyword);

        for (std::string_view grapheme : GraphemeRange(keyword)) {
            stateId += grapheme;

            isAccept = std::nullopt;
            if (i == keywordsLen - 1) {
                isAccept = true;
            }

            result &= dfa.addState(stateId, isAccept);
            result &= dfa.addKeywordTransition(prevStateId, std::string(grapheme), stateId);
            prevStateId += grapheme;
        }

        return result;
    }

    bool BaseTokenizer::addASCIIRangeTransitions(const std::string &srcState, const char startChar, const char endChar, const std::string &dstState) {
        bool result = true;

        for (char c = startChar; c < endChar; ++c) {
            std::string charStr(1, c);
            result &= dfa.addKeywordTransition(srcState, charStr, dstState); 
        }

        return result;
    }

    void BaseTokenizer::setup() {
        clear();
        addStates();
        addTransitions();
    }

    void BaseTokenizer::addStates() {
        
    }

    void BaseTokenizer::addTransitions() {

    }
}