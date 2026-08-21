#include "AGRemapCore/model/strategies/iniClassifiers/IniClassifyStats.h"


namespace AGRemapCore {
    IniClassifyStats::IniClassifyStats(tsl::ordered_map<int, ModTypeIdData> modType, bool isMod, bool isFixed):
        modType(std::move(modType)), isMod(isMod), isFixed(isFixed) {}
}
