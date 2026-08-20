#include "AGRemapCore/model/strategies/iniClassifiers/BaseIniClassifier.h"


namespace AGRemapCore {
    IniClassifyStats BaseIniClassifier::classify(const std::string& iniTxt, std::optional<GameTypeId> gameTypeId) {
        return IniClassifyStats();
    }

    IniClassifyStats BaseIniClassifier::classify(const std::vector<std::string>& iniTxt, std::optional<GameTypeId> gameTypeId) {
        return IniClassifyStats();
    }

    void BaseIniClassifier::clear() {
        // TODO: filled in later
    }
}
