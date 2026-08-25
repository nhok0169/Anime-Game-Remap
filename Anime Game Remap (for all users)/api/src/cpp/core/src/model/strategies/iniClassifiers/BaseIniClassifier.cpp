#include "AGRemapCore/model/strategies/iniClassifiers/BaseIniClassifier.h"


namespace AGRemapCore {
    IniClassifyStats BaseIniClassifier::classify(const std::string& iniTxt, std::optional<GameTypeId> gameTypeId) {
        return IniClassifyStats();
    }

    IniClassifyStats BaseIniClassifier::classify(const std::vector<std::string>& iniTxt, std::optional<GameTypeId> gameTypeId) {
        return IniClassifyStats();
    }

    bool BaseIniClassifier::checkIsMod(const std::string& iniTxt, std::optional<GameTypeId> gameTypeId) {
        return false;
    }

    bool BaseIniClassifier::checkIsMod(const std::vector<std::string>& iniTxt, std::optional<GameTypeId> gameTypeId) {
        return false;
    }

    void BaseIniClassifier::checkIsFixedMod(const std::string& iniTxt, bool* isFixed, bool* isMod, std::optional<GameTypeId> gameTypeId) {
        *isFixed = false;
        *isMod = false;
    }

    void BaseIniClassifier::checkIsFixedMod(const std::vector<std::string>& iniTxt, bool* isFixed, bool* isMod, std::optional<GameTypeId> gameTypeId) {
        *isFixed = false;
        *isMod = false;
    }

    void BaseIniClassifier::clear() {
        // TODO: filled in later
    }
}
