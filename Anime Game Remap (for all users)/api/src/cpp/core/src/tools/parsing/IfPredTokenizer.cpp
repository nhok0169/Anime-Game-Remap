#include "AGRemapCore/tools/parsing/IfPredTokenizer.h"

#include <vector>


namespace AGRemapCore {

    namespace {
        std::unordered_map<std::string, std::string> buildTokens() {
            return {
                {"id", "ID"},
                {"integer", "INT"},
                {"float", "FLOAT"},
                {"null", "NULL"},
                {"+", "PLUS"},
                {"-", "MINUS"},
                {"*", "STAR"},
                {"/", "SLASH"},
                {"(", "LPAREN"},
                {")", "RPAREN"},
                {"==", "EQ"},
                {"!=", "NE"},
                {"<", "LT"},
                {">", "GT"},
                {"<=", "LE"},
                {">=", "GE"},
                {"&&", "AND"},
                {"||", "OR"},
                {"!", "NOT"},
                {" ", "SPACE"},
                {"\t", "TAB"},
            };
        }

        std::unordered_set<std::string> buildKeywordTokenIds() {
            return {"null", "+", "-", "*", "/", "(", ")", "==", "!=", "<", ">", "<=", ">=", "&&", "||", "!", " ", "\t"};
        }

        std::unordered_set<std::string> buildFilteredTokenIds() {
            return {" ", "\t"};
        }
    }

    IfPredTokenizer::IfPredTokenizer(bool setup):
        // Always constructs the base with setup=false, then calls this->setup() itself below --
        // same reasoning as FilteredTokenizer's own constructor (see its comment): a virtual call
        // from within a base class's own constructor can never reach a more-derived override, and
        // that's still true here even though FilteredTokenizer's constructor already does its own
        // "construct base with setup=false, then self-call setup()" -- IfPredTokenizer::addStates/
        // addTransitions are yet another level more derived than that.
        FilteredTokenizer(buildTokens(), buildKeywordTokenIds(), buildFilteredTokenIds(), false) {

        if (setup) {
            this->setup();
        }
    }

    void IfPredTokenizer::addStates() {
        FilteredTokenizer::addStates();

        // id variables
        dfa.addState("idStart");
        dfa.addState("id", true);

        // numbers
        dfa.addState("integer", true);
        dfa.addState("decimalPoint");
        dfa.addState("float", true);
    }

    void IfPredTokenizer::addTransitions() {
        const std::string &startId = startStateId();

        // id variables: "$" followed by at least one identifier char (letter/digit/symbol,
        // continuing for as many more such chars as follow) -- no closing "$" required, unlike
        // SympyTokenizer's "$...$" -- eg. "$swapvar" is itself already a complete identifier.
        const std::string varStartId = "idStart";
        const std::string varAcceptId = "id";
        const std::vector<std::string> varSymbols = {"%", ",", ".", ":", "?", "@", "[", "\\", "]", "^", "_", "`", "{", "}", "~"};

        dfa.addKeywordTransition(startId, "$", varStartId);
        addASCIIRangeTransitions(varStartId, 'a', 'z', varAcceptId);
        addASCIIRangeTransitions(varStartId, 'A', 'Z', varAcceptId);
        addASCIIRangeTransitions(varStartId, '0', '9', varAcceptId);
        for (const std::string &symbol : varSymbols) {
            dfa.addKeywordTransition(varStartId, symbol, varAcceptId);
        }

        addASCIIRangeTransitions(varAcceptId, 'a', 'z', varAcceptId);
        addASCIIRangeTransitions(varAcceptId, 'A', 'Z', varAcceptId);
        addASCIIRangeTransitions(varAcceptId, '0', '9', varAcceptId);
        for (const std::string &symbol : varSymbols) {
            dfa.addKeywordTransition(varAcceptId, symbol, varAcceptId);
        }

        // numbers: a leading "-" continues into "integer" instead of accepting MINUS by itself
        // (maximal munch), so "-5" tokenizes as one INT token, not MINUS followed by INT
        addASCIIRangeTransitions("-", '0', '9', "integer");
        addASCIIRangeTransitions(startId, '0', '9', "integer");
        addASCIIRangeTransitions("integer", '0', '9', "integer");
        dfa.addKeywordTransition("integer", ".", "decimalPoint");
        addASCIIRangeTransitions("decimalPoint", '0', '9', "float");
        addASCIIRangeTransitions("float", '0', '9', "float");
    }
}
