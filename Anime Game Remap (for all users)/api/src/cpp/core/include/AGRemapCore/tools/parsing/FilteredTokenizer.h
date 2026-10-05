#ifndef AGRemapCore_FilteredTokenizer_H
#define AGRemapCore_FilteredTokenizer_H

// ##### Credits

// ===== Anime Game Remap (AG Remap) =====
// Authors: Albert Gold#2696, NK#1321
//
// if you used it to remap your mods pls give credit for "Albert Gold#2696" and "Nhok0169"
// Special Thanks:
//   nguen#2011 (for support)
//   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
//   HazrateGolabi#1364 (for being awesome, and improving the code)

// ##### EndCredits

#include <cstddef>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "AGRemapCore/tools/parsing/BaseTokenizer.h"
#include "AGRemapCore/tools/parsing/Token.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BaseTokenizer`

     A tokenizer that still accepts all tokens, but does not include certain tokens into the
     tokenized result (unless explicitly asked to via ``includeFiltered``)
     @endrst
     */
    class FilteredTokenizer : public BaseTokenizer {
        public:

            /**
             * @brief Constructs a new tokenizer
             *
             * @param tokens
             @rst
             The tokens used for tokenization :raw-html:`<br />` :raw-html:`<br />`

             The keys are the ids to the accepting states of the `DFA`_ and the values are the tokens
             @endrst
             * @param keywordTokenIds The ids of the accepting states in the `DFA`_ such that their corresponding tokens are simply keyword names
             * @param filteredTokenIds The ids of the accepting states in the `DFA`_ to not include their corresponding tokens into the tokenized result
             * @param setup Whether to initialize all the setup for the tokenizer automatically by calling :cpp:func:`setup`
             */
            FilteredTokenizer(std::unordered_map<std::string, std::string> tokens, std::unordered_set<std::string> keywordTokenIds, std::unordered_set<std::string> filteredTokenIds, bool setup = true);

            /**
             * @brief
             @rst
             The ids of the accepting states in the `DFA`_ such that their corresponding tokens are simply keyword names
             @endrst
             */
            const std::unordered_set<std::string>& keywordTokenIds() const;

            /**
             * @brief
             @rst
             The ids of the accepting states in the `DFA`_ to not include their corresponding tokens into the tokenized result
             @endrst
             */
            const std::unordered_set<std::string>& filteredTokenIds() const;

        protected:
            std::unordered_set<std::string> keywordTokenIds_;
            std::unordered_set<std::string> filteredTokenIds_;

            void setup() override;

            bool acceptToken(const std::string &token, const std::string &stateId, bool isAccept, std::vector<Token> &result, size_t lineNo, size_t charNo, bool includeFiltered = false) override;
    };
}

#endif
