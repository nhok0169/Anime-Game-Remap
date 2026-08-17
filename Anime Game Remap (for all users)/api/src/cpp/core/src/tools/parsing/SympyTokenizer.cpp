#include "AGRemapCore/tools/parsing/SympyTokenizer.h"

#include <vector>


namespace AGRemapCore {

    namespace {
        std::unordered_map<std::string, std::string> buildTokens() {
            return {
                {"id", "ID"},
                {"integer", "INT"},
                {"float", "FLOAT"},
                {"+", "PLUS"},
                {"-", "MINUS"},
                {"*", "STAR"},
                {"/", "SLASH"},
                {"(", "LPAREN"},
                {")", "RPAREN"},
                {",", "COMMA"},
                {"==", "EQ"},
                {"!=", "NE"},
                {"<", "LT"},
                {">", "GT"},
                {"<=", "LE"},
                {">=", "GE"},
                {"&", "AND"},
                {"|", "OR"},
                {"~", "NOT"},
                {"Eq", "EQFUNC"},
                {"Ne", "NEFUNC"},
                {"Lt", "LTFUNC"},
                {"Gt", "GTFUNC"},
                {"Le", "LEFUNC"},
                {"Ge", "GEFUNC"},
                {"And", "ANDFUNC"},
                {"Or", "ORFUNC"},
                {"Not", "NOTFUNC"},
                {"True", "TRUE"},
                {"False", "FALSE"},
                {" ", "SPACE"},
                {"\t", "TAB"},
            };
        }

        std::unordered_set<std::string> buildKeywordTokenIds() {
            return {
                "+", "-", "*", "/", "(", ")", ",",
                "==", "!=", "<", ">", "<=", ">=", "&", "|", "~",
                "Eq", "Ne", "Lt", "Gt", "Le", "Ge", "And", "Or", "Not",
                "True", "False", " ", "\t",
            };
        }

        std::unordered_set<std::string> buildFilteredTokenIds() {
            return {" ", "\t"};
        }
    }

    SympyTokenizer::SympyTokenizer(bool setup):
        // See IfPredTokenizer's constructor (and FilteredTokenizer's) for why this always
        // constructs the base with setup=false and self-calls setup() afterward instead.
        FilteredTokenizer(buildTokens(), buildKeywordTokenIds(), buildFilteredTokenIds(), false) {

        if (setup) {
            this->setup();
        }
    }

    void SympyTokenizer::addStates() {
        FilteredTokenizer::addStates();

        // id variables
        dfa.addState("idStart");
        dfa.addState("idParsing");
        dfa.addState("id", true);

        // numbers
        dfa.addState("integer", true);
        dfa.addState("decimalPoint");
        dfa.addState("float", true);
    }

    void SympyTokenizer::addTransitions() {
        const std::string &startId = startStateId();

        // id variables: "$" ... "$" (identifier chars in between; a *closing* "$" is required
        // to reach the accepting "id" state, unlike IfPredTokenizer's "$"-prefix-only grammar)
        // -- eg. "$swapvar$" is a complete identifier, but "$swapvar" alone is not.
        const std::string varStartId = "idStart";
        const std::string varPendingId = "idParsing";
        const std::string varAcceptId = "id";
        const std::vector<std::string> varSymbols = {"%", ",", ".", ":", "?", "@", "[", "\\", "]", "^", "_", "`", "{", "}", "~"};

        dfa.addKeywordTransition(startId, "$", varStartId);
        addASCIIRangeTransitions(varStartId, 'a', 'z', varPendingId);
        addASCIIRangeTransitions(varStartId, 'A', 'Z', varPendingId);
        addASCIIRangeTransitions(varStartId, '0', '9', varPendingId);
        for (const std::string &symbol : varSymbols) {
            dfa.addKeywordTransition(varStartId, symbol, varPendingId);
        }

        addASCIIRangeTransitions(varPendingId, 'a', 'z', varPendingId);
        addASCIIRangeTransitions(varPendingId, 'A', 'Z', varPendingId);
        addASCIIRangeTransitions(varPendingId, '0', '9', varPendingId);
        for (const std::string &symbol : varSymbols) {
            dfa.addKeywordTransition(varPendingId, symbol, varPendingId);
        }

        dfa.addKeywordTransition(varPendingId, "$", varAcceptId);

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
