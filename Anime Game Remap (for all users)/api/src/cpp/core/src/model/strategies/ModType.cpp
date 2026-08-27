#include "AGRemapCore/model/strategies/ModType.h"

#include <utility>


namespace AGRemapCore {
    ModType::ModType(int gameTypeId, int modTypeId, const std::string &name, const std::vector<std::string> &aliases,
                      std::shared_ptr<BaseIniParser> iniParser, std::shared_ptr<BaseIniFixer> iniFixer):
        gameTypeId(gameTypeId), modTypeId(modTypeId), name(name), aliases(aliases),
        iniParser(std::move(iniParser)), iniFixer(std::move(iniFixer)) {
        // Mirrors the pure-Python original's own "if (iniParseBuilder is None): iniParseBuilder =
        // IniParseBuilder(GIMIParser)" fallback -- except that no concrete GIMIParser/GIMIFixer
        // equivalent exists in C++ yet, so the plain base classes stand in for now.
        if (this->iniParser == nullptr) {
            this->iniParser = std::make_shared<BaseIniParser>();
        }

        if (this->iniFixer == nullptr) {
            this->iniFixer = std::make_shared<BaseIniFixer>(this->iniParser.get());
        }
    }
}
