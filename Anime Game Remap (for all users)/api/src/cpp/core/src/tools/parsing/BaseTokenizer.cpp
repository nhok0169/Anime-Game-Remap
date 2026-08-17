#include "AGRemapCore/tools/parsing/BaseTokenizer.h"

#include "AGRemapCore/tools/grapheme/GraphemeRange.h"
#include "AGRemapCore/tools/StringTools.h"
#include "AGRemapCore/tools/parsing/SyntaxErr.h"


namespace AGRemapCore {
    BaseTokenizer::BaseTokenizer(std::unordered_map<std::string, std::string> tokens, bool setup): tokens_(std::move(tokens)) {
        if (setup) {
            this->setup();
        }
    }

    void BaseTokenizer::clear() {
        dfa.clear();
    }

    void BaseTokenizer::reset() {
        dfa.reset();
    }

    const std::string& BaseTokenizer::addStartState() {
        dfa.addState(startStateId_, std::nullopt, true);
        return startStateId_;
    }

    std::string BaseTokenizer::addKeyword(const std::string &keyword) {
        addStartState();
        std::string prevStateId = startStateId_;
        std::string stateId = startStateId_;
        size_t keywordLen = StringTools::countGrapheme(keyword);
        size_t i = 0;

        for (std::string_view grapheme : GraphemeRange(keyword)) {
            stateId += grapheme;

            std::optional<bool> isAccept = std::nullopt;
            if (i == keywordLen - 1) {
                isAccept = true;
            }

            dfa.addState(stateId, isAccept);
            dfa.addKeywordTransition(prevStateId, std::string(grapheme), stateId);
            prevStateId = stateId;

            ++i;
        }

        return stateId;
    }

    void BaseTokenizer::addASCIIRangeTransitions(const std::string &srcId, char startChar, char endChar, const std::string &destId) {
        for (int c = static_cast<unsigned char>(startChar); c <= static_cast<unsigned char>(endChar); ++c) {
            std::string charStr(1, static_cast<char>(c));
            dfa.addKeywordTransition(srcId, charStr, destId);
        }
    }

    void BaseTokenizer::setup() {
        clear();
        addStates();
        addTransitions();
    }

    void BaseTokenizer::addStates() {
        addStartState();
    }

    void BaseTokenizer::addTransitions() {

    }

    bool BaseTokenizer::acceptToken(const std::string &token, const std::string &stateId, bool isAccept, std::vector<Token> &result, size_t lineNo, size_t charNo, bool includeFiltered) {
        (void) includeFiltered; // unused by this base class -- see the header's doc comment

        if (!isAccept) {
            return false;
        }

        auto tokenTypeKVP = tokens_.find(stateId);
        if (tokenTypeKVP == tokens_.end()) {
            return false;
        }

        result.emplace_back(tokenTypeKVP->second, token, lineNo, charNo);
        dfa.reset();
        return true;
    }

    void BaseTokenizer::raiseSyntaxErr(const ParseContext &ctx, const std::string &currentToken, size_t lineNo, size_t charNo) {
        // Mirrors BaseTokenizerOld._raiseSyntaxErr's 'charNo - len(currentToken)' -- re-derives
        // the token's starting column from where it ended, rather than threading a start-column
        // value through separately. Clamped to 0 instead of allowed to underflow (charNo/tokenLen
        // are unsigned here, unlike Python's arbitrary-precision int) -- not a real behavior
        // change for any currently-reachable caller, since this class's own loop always keeps
        // charNo >= the in-progress token's grapheme count.
        size_t tokenLen = StringTools::countGrapheme(currentToken);
        size_t startCharNo = (charNo >= tokenLen) ? (charNo - tokenLen) : 0;

        Token token(std::nullopt, currentToken, lineNo, startCharNo);
        throw SyntaxErr(ctx, token, "tokenization");
    }

    std::vector<Token> BaseTokenizer::simplifiedMaximalMunch(const ParseContext &ctx, bool includeFiltered) {
        // "\n".join(ctx.lines) -- ctx.lines never carries terminators of its own (see
        // ParseContext), so re-joining with a plain '\n' is exactly the Python original's
        // normalization step, not a lossy approximation of it.
        std::string src;
        bool firstLine = true;
        for (const std::string &line : ctx.lines) {
            if (!firstLine) {
                src += '\n';
            }
            src += line;
            firstLine = false;
        }

        // Iterated by grapheme cluster, not by byte or codepoint -- matches addKeyword's own
        // grapheme-based transition keys (a DFA transition keyed by "é" as a single unit
        // wouldn't otherwise ever match a caller stepping byte-by-byte or codepoint-by-codepoint
        // through the same text), and is a strictly more correct upgrade over the pure-Python
        // original's per-codepoint 'src[i]' indexing for any multi-codepoint grapheme.
        std::vector<std::string_view> graphemes;
        for (std::string_view grapheme : GraphemeRange(src)) {
            graphemes.push_back(grapheme);
        }

        std::vector<Token> result;
        dfa.reset();

        std::string stateId = dfa.getCurrentStateId();
        bool isAccept = false;
        size_t srcLen = graphemes.size();
        std::string currentToken;
        size_t i = 0;

        size_t lineNo = ctx.startLineNo;
        size_t charNo = 1;
        size_t tokenLineNo = lineNo;
        size_t tokenCharNo = charNo;

        while (i < srcLen) {
            std::string letter(graphemes[i]);
            bool transitionTaken = false;
            dfa.transition(letter, &stateId, &isAccept, &transitionTaken);

            if (transitionTaken) {
                ++i;
                currentToken += letter;

                if (letter == "\n") {
                    ++lineNo;
                    charNo = 1;
                } else {
                    ++charNo;
                }

                continue;
            }

            // Deliberately doesn't advance 'i' here -- the rejected grapheme at graphemes[i] is
            // retried on the next iteration, now against a freshly-reset DFA (via acceptToken's
            // reset on success), as the start of a new token. This is the "maximal munch" retry
            // step, not an oversight.
            bool accepted = acceptToken(currentToken, stateId, isAccept, result, tokenLineNo, tokenCharNo, includeFiltered);
            if (!accepted) {
                raiseSyntaxErr(ctx, currentToken + letter, lineNo, charNo + 1);
            }

            currentToken.clear();
            tokenLineNo = lineNo;
            tokenCharNo = charNo;
        }

        bool accepted = acceptToken(currentToken, stateId, isAccept, result, tokenLineNo, tokenCharNo, includeFiltered);
        if (!accepted) {
            raiseSyntaxErr(ctx, currentToken, lineNo, charNo);
        }

        return result;
    }

    const std::unordered_map<std::string, std::string>& BaseTokenizer::tokens() const {
        return tokens_;
    }

    const std::string& BaseTokenizer::startStateId() const {
        return startStateId_;
    }
}
