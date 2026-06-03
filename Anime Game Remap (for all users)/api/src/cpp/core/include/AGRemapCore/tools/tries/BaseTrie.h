#ifndef BaseTrie_H
#define BaseTrie_H

#include <functional>
#include <string>
#include <optional>
#include <cstdint>

#include "../BiMap.h"


namespace AGRemapCore {
    template <typename TrieVal>
    class BaseTrie {
        public:
            using DupHandler = std::function<TrieVal(const std::string&, const TrieVal&, const TrieVal&)>;

            BaseTrie(const std::optional<std::unordered_map<std::string, TrieVal>> &data, std::optional<DupHandler> handler);
            virtual ~BaseTrie() = default; 

            void setHandleDuplicates(DupHandler newHandler);

            void clear();
            void build(const std::optional<std::unordered_map<std::string, TrieVal>> &data);
            bool add(const std::string &key, const TrieVal &val);


        protected:
            DupHandler handleDuplicate;

            virtual std::uint64_t getNextNodeId();
            virtual std::uint64_t getNextKeywordId();
            virtual std::uint64_t updateNextNodeId();
            virtual std::uint64_t updateNextKeywordId();
            virtual std::uint64_t resetNextNodeId();
            virtual std::uint64_t resetNextKeywordId();

            virtual std::int8_t comparekeywordIds(std::uint64_t keywordId1, std::uint64_t keywordId2);

            BiMap<std::uint64_t, std::string, typename KHash = std::hash<std::uint64_t>, std::equal_to<std::uint64_t>, std::hash<std::string>, std::equal_to<std::string>> keywords;
            std::unordered_map<std::uint64_t, std::unordered_map<std::string, std::uint64_t>> children
    };
}

#endif