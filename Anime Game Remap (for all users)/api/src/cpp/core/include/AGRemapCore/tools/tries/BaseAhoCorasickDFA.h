#ifndef AGRemapCore_BaseAhoCorasickDFA_H
#define AGRemapCore_BaseAhoCorasickDFA_H

#include <tuple>

#include "AGRemapCore/tools/tries/BaseTrie.h"


namespace AGRemapCore {

    /**
     * @brief 
     @rst
     The `DFA (Deterministic Finite Automaton)`_ used in the `Aho-Corasick`_ algorithm
     @endrst
     *
     * @tparam TrieVal
     @rst
     The types for the values to store in the `DFA`_
     @endrst
     */
    template <typename TrieVal>
    class BaseAhoCorasickDFA: public BaseTrie<TrieVal> {
        public:
            #ifdef AGREMAPCORE_DOCS_PARSE
            #define DupHandler std::function<TrieVal(std::string_view, const TrieVal&, const TrieVal&)>
            #define DupHandler2 std::function<TrieVal(const std::string &, const TrieVal&, const TrieVal&)>
            #else
            using DupHandler = std::function<TrieVal(std::string_view, const TrieVal&, const TrieVal&)>;
            using DupHandler2 = std::function<TrieVal(const std::string &, const TrieVal&, const TrieVal&)>;
            #endif

            /**
             * @copydoc BaseTrie::BaseTrie(const std::optional<std::unordered_map<std::string, TrieVal>> &, const std::optional<std::variant<std::function<TrieVal(std::string_view, const TrieVal&, const TrieVal&)>, std::function<TrieVal(const std::string &, const TrieVal&, const TrieVal&)>>>&)
             */
            BaseAhoCorasickDFA(const std::optional<std::unordered_map<std::string, TrieVal>> &data = std::nullopt, const std::optional<std::variant<DupHandler, DupHandler2>>& handler = std::nullopt);

            void clear() override;
            bool contains(const std::string &txt) override;

            void build(const std::optional<std::unordered_map<std::string, TrieVal>> &data = std::nullopt) override;

            bool add(std::string_view key, const TrieVal &val) override;
            bool add(const std::string &key, const TrieVal &val) override;

            /**
             * @brief
             @rst
             Finds all occurences of the keywords from the `DFA`_ in the given text
             @endrst
             *
             * @param txt The text to search for keywords
             * 
             * @return 
             @rst
             The indices for all the found keywords within the given text :raw-html:`<br />` :raw-html:`<br />`

             * The keys are the keywords found
             * The values are all instances of the keyword found
             * The tuple contains the starting index of the found instance and the ending index of the found instance
             @endrst
             */
            virtual std::unordered_map<std::string, std::vector<std::tuple<size_t, size_t>>> findAll(std::string_view txt);

            /**
             * @copydoc findAll(std::string_view)
             */
            virtual std::unordered_map<std::string, std::vector<std::tuple<size_t, size_t>>> findAll(const std::string &txt);

            /**
             * @brief
             @rst
             Finds the first occurences of the keywords from the `DFA`_ in the given text
             @endrst
             *
             * @param txt The text to search for keywords
             * 
             * @return
             @rst
             The indices for all the found keywords within the given text :raw-html:`<br />` :raw-html:`<br />`

             * The keys are the keywords found
             * The tuple contains the starting index of the found instance and the ending index of the first found instance
             @endrst
             */
            virtual std::unordered_map<std::string, std::tuple<size_t, size_t>> findFirstAll(std::string_view txt);

            /**
             * @copydoc findFirstAll(std::string_view)
             */
            virtual std::unordered_map<std::string, std::tuple<size_t, size_t>> findFirstAll(const std::string &txt);

            /**
             * @brief Finds the first keyword within 'txt'
             * 
             * @param txt The text to search for keywords
             * @param resultInd The pointer to store the starting index of where the keyword was found
             * 
             * @return The found keyword
             */
            virtual const std::string *findPtr(std::string_view txt, size_t *resultInd);

            /**
             * @copydoc findPtr(std::string_view, size_t *)
             */
            virtual const std::string *findPtr(const std::string &txt, size_t *resultInd);

            /**
             * @copydoc findPtr(std::string_view, size_t *)
             * 
             * @throw std::runtime_error Thrown when no keywords are found in the text
             */
            virtual const std::string &find(std::string_view txt, size_t *resultInd);

            /**
             * @copydoc find(std::string_view, size_t *)
             */
            virtual const std::string &find(const std::string &txt, size_t *resultInd);
            
            /**
             * @brief Finds the first largest keyword within 'txt'
             * 
             * @param txt The text to search for keywords
             * @param resultInd The pointer to store the starting index of where the keyword was found
             * 
             * @return The found keyword
             */
            virtual const std::string *findMaximalPtr(std::string_view txt, size_t *resultInd);

            /**
             * @copydoc findMaximalPtr(std::string_view, size_t *)
             */
            virtual const std::string *findMaximalPtr(const std::string &txt, size_t *resultInd);

            /**
             * @copydoc findMaximalPtr(std::string_view, size_t *)
             * 
             * @throw std::runtime_error Thrown when no keywords are found in the text
             */
            virtual const std::string &findMaximal(std::string_view txt, size_t *resultInd);

            /**
             * @copydoc findMaximal(std::string_view, size_t *)
             */
            virtual const std::string &findMaximal(const std::string &txt, size_t *resultInd);

            /**
             * @brief Finds the first few largest keywords within 'txt'
             * 
             * @param txt The text to search for keywords
             * @param count The count of how many keywords to find in the search string
             * 
             * @return The found keyword
             */
            virtual std::tuple<std::vector<std::string_view>, std::vector<size_t>> findMaximal(std::string_view txt, size_t count = 1);

            /**
             * @copydoc findMaximal(std::string_view, size_t)
             */
            virtual std::tuple<std::vector<std::string_view>, std::vector<size_t>> findMaximal(const std::string &txt, size_t count = 1);

            /**
             * @brief Retrieves the corresponding value from the first keyword found in 'txt'
             * 
             * @param txt The text to search for keywords
             * 
             * @return A tuple that contains the keyword and its corresponding value
             */
            virtual std::tuple<const std::string *, const TrieVal *> getKVPPtr(std::string_view txt);

            /**
             * @copydoc getKVPPtr(std::string_view)
             */
            virtual std::tuple<const std::string *, const TrieVal *> getKVPPtr(const std::string &txt);

            /**
             * @copydoc getKVPPtr(std::string_view)
             * 
             * @throw std::runtime_error Thrown when no keywords are found in the text
             */
            virtual std::tuple<const std::string &, const TrieVal &> getKVP(std::string_view txt);

            /**
             * @copydoc getKVP(std::string_view)
             */
            virtual std::tuple<const std::string &, const TrieVal &> getKVP(const std::string &txt);

            /**
             * @brief Retrieves all the corresponding values to all the keywords found within 'txt'
             * 
             * @param txt The text to search for keywords
             * 
             * @return
             @rst
             The found keywords and their corresponding values :raw-html:`<br />` :raw-html:`<br />`

             The keys are the keywords found and the values are the values to the keywords
             @endrst
             */
            virtual std::unordered_map<std::string, const TrieVal *> getAll(std::string_view txt);

            /**
             * @copydoc getAll(std::string_view)
             */
            virtual std::unordered_map<std::string, const TrieVal *> getAll(const std::string &txt);
            
            /**
             * @brief Retrieves the corresponding value from the first largest keyword fround in 'txt'
             * 
             * @param txt The text to search for keywords
             * 
             * @return A tuple containing the keyword found and its corresopnding value
             */
            virtual std::tuple<const std::string *, const TrieVal *> getMaximalPtr(std::string_view txt);

            /**
             * @copydoc getMaximalPtr(std::string_view)
             */
            virtual std::tuple<const std::string *, const TrieVal *> getMaximalPtr(const std::string &txt);

            /**
             * @copydoc getMaximalPtr(std::string_view)
             * 
             * @throw std::runtime_error Thrown when no keywords are found in the text
             */
            virtual std::tuple<const std::string &, const TrieVal &> getMaximal(std::string_view txt);

            /**
             * @copydoc getMaximal(std::string_view)
             */
            virtual std::tuple<const std::string &, const TrieVal &> getMaximal(const std::string &txt);

            /**
             * @brief Retrieves the corresponding value from the first few largest keywords fround in 'txt'
             * 
             * @param txt The text to search for keywords
             * @param count The count of how many keywords to find in the search string
             * 
             * @return
             @rst
             A tuple containing:

             #. The list of keywords found
             #. The corresponding found values to the keywords
             @endrst
             */
            virtual std::tuple<std::vector<std::string_view>, std::vector<const TrieVal *>> getMaximal(std::string_view txt, size_t count);

            /**
             * @copydoc getMaximal(std::string_view, size_t)
             */
            virtual std::tuple<std::vector<std::string_view>, std::vector<const TrieVal *>> getMaximal(const std::string &txt, size_t count);

            /**
             * @brief Finds the largest keyword that is a prefix of the search text
             * 
             * @param txt The text to search for keywords
             * 
             * @return The found keyword
             */
            virtual const std::string * maximalStartsWithPtr(std::string_view txt);

            /**
             * @copydoc maximalStartsWithPtr(std::string_view)
             */
            virtual const std::string * maximalStartsWithPtr(const std::string &txt);

            /**
             * @copydoc maximalStartsWithPtr(std::string_view)
             * 
             * @throw std::runtime_error Thrown when no keywords are found in the text
             */
            virtual const std::string & maximalStartsWith(std::string_view txt);

            /**
             * @copydoc maximalStartsWith(std::string_view)
             */
            virtual const std::string & maximalStartsWith(const std::string &txt);

        protected:
            /**
             * @brief
             @rst
             The failure edges in the `DFA`_ :raw-html:`<br />` :raw-html:`<br />`

             The keys are the ids to the sources node of the edges and the values are the ids to the sink nodes of the edges
             @endrst
             */
            std::unordered_map<std::uint64_t, std::uint64_t> fail;

            /**
             * @brief
             @rst
             Retrieves the next state for travel to in the `DFA`_
             @endrst
             *
             * @param currentStateId The internal id of the current state
             * @param letter The transition letter to go to the next state
             * @param resIsFail The resultant pointer that indicates whether the next state is from a failure transition
             * 
             * @return The internal id to the next state
             */
            std::uint64_t getNextState(std::uint64_t currentStateId, std::string_view letter, bool *resIsFail);
    };
}

#include "BaseAhoCorasickDFA.tpp"

#endif