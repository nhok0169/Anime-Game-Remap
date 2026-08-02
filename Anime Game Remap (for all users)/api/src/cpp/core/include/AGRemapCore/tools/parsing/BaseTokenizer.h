#ifndef AGRemapCore_BaseTokenizer_H
#define AGRemapCore_BaseTokenizer_H

#include "AGRemapCore/tools/dfa/BaseDFA.h"
#include "AGRemapCore/tools/StringHash.h"
#include "AGRemapCore/tools/parsing/ParseContext.h"

#include <memory>


namespace AGRemapCore {
    class BaseTokenizer {
        public:
            BaseTokenizer(std::unordered_map<std::string, std::string>&& tokens, bool setup = true);
            // BaseTokenizer(std::shared_ptr<std::unordered_map<std::string, std::string>> tokens, bool setup = true);

            virtual void clear();
            virtual void reset();

            virtual bool addStartState();

            virtual bool addKeyword(const std::string &keyword);
            virtual bool addASCIIRangeTransitions(const std::string &srcState, const char startChar, const char endChar, const std::string &dstState);

            virtual void setup();

        protected:
            std::unordered_map<std::string, std::string> tokens;
            BaseDFA<std::string, std::string, std::equal_to<std::string>, StringViewHash, std::equal_to<std::string>, StringViewHash> dfa;
            std::string startStateId = "";

            virtual void addStates();
            virtual void addTransitions();

            // virtual simplifiedMaximalMunch(const ParseContext& src);

    };
}

#endif