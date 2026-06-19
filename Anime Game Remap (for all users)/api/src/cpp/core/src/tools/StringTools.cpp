#include "AGRemapCore/tools/StringTools.h"


namespace AGRemapCore {
    std::int8_t StringTools::compareStrPtrs(const std::string *strPtr1, const std::string *strPtr2) {
        int res = strPtr1->compare(*strPtr2);
        return (res > 0) - (res < 0);
    }
}