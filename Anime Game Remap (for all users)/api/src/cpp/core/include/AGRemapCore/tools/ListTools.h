#ifndef ListTools_H
#define ListTools_H

#include <vector>
#include <cstddef>


namespace AGRemapCore {
    class ListTools {
        public:
            static std::vector<std::ptrdiff_t> getIndsAfterRemove(const std::vector<size_t>& removedInds, size_t lstLen);
    };
}

#endif