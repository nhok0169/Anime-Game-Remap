#include "AGRemapCore/tools/tries/BaseTrie.h"


namespace AGRemapCore {
    template <typename TrieVal>
    BaseTrie<TrieVal>::BaseTrie(const std::optional<std::unordered_map<std::string, TrieVal>> &data, std::optional<DupHandler> handler) {
        
    }
}