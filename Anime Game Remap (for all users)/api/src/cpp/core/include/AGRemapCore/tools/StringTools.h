#ifndef AGRemapCore_StringTools_H
#define AGRemapCore_StringTools_H

#include <cstdint>
#include <string>


namespace AGRemapCore {
    class StringTools {
        public:
            static std::int8_t compareStrPtrs(const std::string *strPtr1, const std::string *strPtr2);
    };
}


#endif