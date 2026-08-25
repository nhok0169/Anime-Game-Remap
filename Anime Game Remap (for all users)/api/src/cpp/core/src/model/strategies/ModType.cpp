#include "AGRemapCore/model/strategies/ModType.h"


namespace AGRemapCore {
    ModType::ModType(int gameTypeId, int modTypeId, const std::string &name, const std::vector<std::string> &aliases): gameTypeId(gameTypeId), modTypeId(modTypeId), name(name), aliases(aliases) {}
}
