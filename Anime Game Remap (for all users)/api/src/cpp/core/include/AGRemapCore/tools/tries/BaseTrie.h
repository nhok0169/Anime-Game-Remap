#ifndef AGRemapCore_BaseTrie_H
#define AGRemapCore_BaseTrie_H

#include <functional>
#include <string>
#include <string_view>
#include <optional>
#include <cstdint>
#include <vector>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <stdexcept>

#include "AGRemapCore/tools/BiMap.h"
#include "AGRemapCore/tools/idGenerator/BaseIdGenerator.h"
#include "AGRemapCore/tools/StringHash.h"


namespace AGRemapCore {
    extern template class BiMap<std::uint64_t, std::string, std::hash<std::uint64_t>, std::equal_to<std::uint64_t>, std::hash<std::string>, std::equal_to<std::string>>;

    /**
     * @brief Base class for a Trie
     * 
     * @tparam TrieVal
     *      The types for the values to store in the Trie
     */
    template <typename TrieVal>
    class BaseTrie {
        public:
            using DupHandler = std::function<TrieVal(const std::string&, const TrieVal&, const TrieVal&)>;

            /**
             * @brief Constucts a new trie
             * 
             * @param data
             @rst
             Any initial data to insert :raw-html:`<br />` :raw-html:`<br />`

             The keys are the keywords to put into the `trie`_ and the values are the corresponding values to the keywords :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``std::nullopt``
             @endrst
             * 
             * @param handler
             @rst
             Function to handle the case where 2 `KVPs`_ inserted have the same key(word) :raw-html:`<br />` :raw-html:`<br />`

             The function takes in the following parameters:

             #. The duplicate keyword in both `KVPs`_
             #. The value of the existing `KVP`_
             #. The value of the new `KVP`_

             If this value is ``std::nullopt``, will return the value of the new `KVP`_ by default :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``std::nullopt``
             @endrst
             */
            BaseTrie(const std::optional<std::unordered_map<std::string, TrieVal>> &data = std::nullopt, const std::optional<DupHandler>& handler = std::nullopt);

            /**
             * @brief Destroys the trie
             */
            virtual ~BaseTrie() = default; 

            /**
             * @brief Getter for #handleDuplicate
             * 
             * @return The corresponding handler
             */
            const DupHandler &getHandleDuplicate() const;

            /**
             * @brief Setter for #handleDuplicate
             * 
             * @param newHandler The new function to set
             */
            void setHandleDuplicate(const DupHandler &newHandler);

            /**
             * @brief Clears the data in the cache
             */
            virtual void clear();

            /**
             * @brief
             @rst
             Rebuilds the `trie`_
             @endrst
             *
             * @param data
             @rst
             Any initial data to put into the `trie`_ :raw-html:`<br />` :raw-html:`<br />`

             The keys are the keywords to put into the trie and the values are the corresponding values to the keywords :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``std::nullopt``
             @endrst
             */
            virtual void build(const std::optional<std::unordered_map<std::string, TrieVal>> &data = std::nullopt);

            /**
             * @brief Adds a new keyword
             * 
             * @param key The keyword to add
             * 
             * @param val The value associated with the keyword
             * 
             * @return Whether the keyword has been added
             */
            virtual bool add(const std::string &key, const TrieVal &val);

            /**
             * @brief Whether the trie contains the corresponding keyword
             * 
             * @param keyword The keyword to find
             * 
             * @return Whether the keyword exists in the trie
             */
            virtual bool contains(const std::string &keyword);

            /**
             * @brief Retrieves the pointer to the corresponding value of the keyword
             * 
             * @param keyword The keyword to find
             * 
             * @return The pointer to the value if available, otherwise returns the null pointer
             */
            virtual TrieVal* getPtr(const std::string &keyword);

            /**
             * @brief Retrieves the corresponding value of the keyword
             * 
             * @param keyword The keyword to find
             * 
             * @return The corresponding value if available, otherwise returns `std::nullopt`
             */
            virtual std::optional<TrieVal> get(const std::string &keyword);

        protected:
            /**
             * @brief
             @rst
             The function to handle the case where 2 `KVPs`_ inserted have the same key(word) :raw-html:`<br />` :raw-html:`<br />`

             The function takes in the following parameters:
 
             #. The duplicate keyword in both `KVPs`_
             #. The value of the existing `KVP`_
             #. The value of the new `KVP`_
             @endrst
             */
            DupHandler handleDuplicate;

            /**
             * @brief The internal id generator for nodes
             */
            std::unique_ptr<BaseIdGenerator<std::uint64_t>> nodeIdGenerator;

            /**
             * @brief The internal id generator for keywords
             */
            std::unique_ptr<BaseIdGenerator<std::uint64_t>> keywordIdGenerator;

            /**
             * @brief The internal id for the root node
             */
            std::uint64_t rootId = 0;

            /**
             * @brief Initializes #nodeIdGenerator
             */
            virtual void initNodeIdGenerator();

            /**
             * @brief Initializes #keywordIdGenerator
             */
            virtual void initKeywordIdGenerator();

            /**
             * @brief
             @rst
             The default function to handle the case where 2 `KVPs`_ inserted have the same key(word) :raw-html:`<br />` :raw-html:`<br />`

             The function will always return `newVal`
             @endrst
             *
             * @param key
             @rst
             The duplicate key for both `KVPs`_
             @endrst
             *
             * @param srcVal
             @rst
             The value of the `KVP`_ that already exists in the trie
             @endrst
             *
             * @param newVal
             @rst
             The value of the new `KVP`_ to insert
             @endrst
             *
             * @return The new value from the argument, `newVal`
             */
            static TrieVal defaultHandleDuplicate(const std::string& key, const TrieVal &srcVal, const TrieVal &newVal);

            /**
             * @brief 
             @rst
             The `compare function`_ for the ids of the keywords :raw-html:`<br />` :raw-html:`<br />`

             The sorting order for keyword ids is as follows:

             #. ids to existing keywords go before ids that do not correspond to a keyword
             #. ids with longer length keywords go before ids with shorter length keywords
             #. keywords of ids are ordered in alphabetical order
             @endrst
             *
             * @param keywordId1 The id for the first keyword
             * 
             * @param keywordId2 The id for the second keyword
             * 
             * @return
             @rst
             The comparison result of a `compare function`_
             @endrst
             */
            virtual std::int8_t comparekeywordIds(const std::uint64_t &keywordId1, const std::uint64_t &keywordId2);

            /**
             * @brief
             @rst
             Adds a keyword to the `trie`_
             @endrst
             *
             * @param keyword The keyword to add
             * 
             * @param val The value associated with the keyword
             * 
             * @return
             @rst
             Whether the keyword has not already been inserted into the `trie`_
             @endrst
             */
            virtual bool addKeyword(const std::string &keyword, const TrieVal &val);

            /**
             * @brief 
             @rst
             Adds in a new `KVP`_
             @endrst
             *
             * @warning
             @rst
             If 'keyword' already exists, then the new value for the `KVP`_ will be
             determined based off the :attr:`handleDuplicate` function
             @endrst
             *
             * @param keyword The keyword to add
             * 
             * @param val The value associated with the keyword
             * 
             * @return The internal id of the added keyword
             */
            virtual std::uint64_t addKVP(const std::string &keyword, const TrieVal &val);

            /**
             * @brief The mapping of the keywords and their internal ids
             */
            BiMap<std::uint64_t, std::string, std::hash<std::uint64_t>, std::equal_to<std::uint64_t>, std::hash<std::string>, std::equal_to<std::string>> keywords;

            /**
             * @brief The mapping of the internal keyword ids to their corresponding values
             */
            std::unordered_map<std::uint64_t, TrieVal> vals;

            /**
             * @brief
             @rst
             The children nodes associated to a node :raw-html:`<br />` :raw-html:`<br />`

             * The outer keys are the ids of the nodes
             * The inner keys are the string sequences of the edges between a node and its children
             * The inner values are the ids for the children
             @endrst
             *
             * @note
             @rst
             This is the `adjacency list`_ for the trie
             @endrst
             */
            std::unordered_map<std::uint64_t, std::unordered_map<std::string, std::uint64_t, StringViewHash, std::equal_to<void>>> children;

            /**
             * @brief
             @rst
             The keywords found at a node :raw-html:`<br />` :raw-html:`<br />`

             The keys are the ids for the nodes and the values are the ids for the found keywords
             @endrst
             */
            std::unordered_map<std::uint64_t, std::vector<std::uint64_t>> nodeKeywordIds;

            /**
             * @brief The ids to the nodes that are considered as accepting states
             */
            std::unordered_set<std::uint64_t> acceptNodeIds;
    };
}

#include "BaseTrie.tpp"

#endif