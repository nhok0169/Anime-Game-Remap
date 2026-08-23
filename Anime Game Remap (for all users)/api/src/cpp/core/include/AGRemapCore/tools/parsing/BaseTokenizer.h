#ifndef AGRemapCore_BaseTokenizer_H
#define AGRemapCore_BaseTokenizer_H

#include <cstddef>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "AGRemapCore/tools/dfa/BaseDFA.h"
#include "AGRemapCore/tools/StringHash.h"
#include "AGRemapCore/tools/parsing/ParseContext.h"
#include "AGRemapCore/tools/parsing/Token.h"


namespace AGRemapCore {

    /**
     * @brief The base class used for tokenizing text
     */
    class BaseTokenizer {
        public:

            /**
             * @brief Constructs a new tokenizer
             *
             * @param tokens
             @rst
             The tokens used for tokenization :raw-html:`<br />` :raw-html:`<br />`

             The keys are the ids to the accepting states of the `DFA`_ and the values are the tokens
             @endrst
             * @param setup Whether to initialize all the setup for the tokenizer automatically by calling :cpp:func:`setup`
             */
            explicit BaseTokenizer(std::unordered_map<std::string, std::string> tokens, bool setup = true);

            virtual ~BaseTokenizer() = default;

            /**
             * @brief Clears the `DFA`_ of the tokenizer
             */
            virtual void clear();

            /**
             * @brief Resets the state of the `DFA`_ for the tokenizer
             */
            virtual void reset();

            /**
             * @brief Adds the start state representing an empty string
             *
             * @return The id of the start state
             */
            virtual const std::string& addStartState();

            /**
             * @brief Adds a keyword into the `DFA`_ of the tokenizer
             *
             * @param keyword The keyword to add
             *
             * @return The id of the accepting node in the `DFA`_
             */
            virtual std::string addKeyword(const std::string &keyword);

            /**
             * @brief
             @rst
             Adds a group of transitions from one state to another according to a range of `ASCII`_ characters
             (``startChar``/``endChar`` inclusive)
             @endrst
             *
             * @param srcId The id of the source state for the transition
             * @param startChar The starting character within the ASCII range to add a transition for
             * @param endChar The ending character within the ASCII range to add a transition for
             * @param destId The id of the destination state for the transition
             */
            virtual void addASCIIRangeTransitions(const std::string &srcId, char startChar, char endChar, const std::string &destId);

            /**
             * @brief Performs any necessary setup to the tokenizer
             */
            virtual void setup();

            /**
             * @brief
             @rst
             Tokenizes the source text into tokens using the `Simplified Maximal Munch`_ algorithm
             @endrst
             *
             * @param ctx The context for the source text to be tokenized
             * @param includeFiltered
             @rst
             Ignored by this base class -- exists so a subclass like :cpp:class:`FilteredTokenizer`
             can thread its own "include the otherwise-filtered tokens" flag through this same,
             shared loop via :cpp:func:`acceptToken` (which *does* look at it), rather than every
             subclass having to reimplement the whole loop just to pass one extra value along
             @endrst
             *
             * @throws AGRemapCore::SyntaxErr The provided source text cannot be correctly tokenized
             *
             * @return The list of tokens to the source text
             */
            virtual std::vector<Token> simplifiedMaximalMunch(const ParseContext &ctx, bool includeFiltered = false);

            /**
             * @brief
             @rst
             The tokens used for tokenization :raw-html:`<br />` :raw-html:`<br />`

             The keys are the ids to the accepting states of the `DFA`_ and the values are the tokens
             @endrst
             */
            const std::unordered_map<std::string, std::string>& tokens() const;

            /**
             * @brief The id of the starting state of the `DFA`_
             */
            const std::string& startStateId() const;

        protected:

            /**
             * @brief
             @rst
             The tokens used for tokenization :raw-html:`<br />` :raw-html:`<br />`

             The keys are the ids to the accepting states of the `DFA`_ and the values are the tokens
             @endrst
             */
            std::unordered_map<std::string, std::string> tokens_;

            /**
             * @brief The internal `DFA`_
             */
            BaseDFA<std::string, std::string, std::equal_to<std::string>, StringViewHash, std::equal_to<std::string>, StringViewHash> dfa;

            /**
             * @brief The id of the starting state of the `DFA`_
             */
            std::string startStateId_ = "";

            /**
             * @brief Adds all the necessary states into the `DFA`_ of the tokenizer
             */
            virtual void addStates();

            /**
             * @brief Adds all the necessary state transitions to the `DFA`_ of the tokenizer
             */
            virtual void addTransitions();

            /**
             * @brief
             @rst
             Tries to accept ``token`` as a completed token :raw-html:`<br />` :raw-html:`<br />`

             If accepted, appends the corresponding :cpp:class:`Token` to ``result`` and resets the `DFA`_
             @endrst
             *
             * @param token The candidate token text accumulated so far
             * @param stateId The id of the `DFA`_ state currently reached after consuming ``token``
             * @param isAccept Whether ``stateId`` is an accepting state
             * @param result The vector to append the accepted :cpp:class:`Token` to, if accepted
             * @param lineNo The line number ``token`` belongs to
             * @param charNo The character number ``token`` belongs to within its line
             * @param includeFiltered Ignored by this base class's own implementation (see :cpp:func:`simplifiedMaximalMunch`)
             *
             * @return Whether ``token`` was accepted
             */
            virtual bool acceptToken(const std::string &token, const std::string &stateId, bool isAccept, std::vector<Token> &result, size_t lineNo, size_t charNo, bool includeFiltered = false);

            /**
             * @brief Raises a syntax error for some unrecognized text
             *
             * @throws AGRemapCore::SyntaxErr Always
             */
            [[noreturn]] void raiseSyntaxErr(const ParseContext &ctx, const std::string &currentToken, size_t lineNo, size_t charNo);
    };
}

#endif
