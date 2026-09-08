#include "AGRemapCore/model/strategies/iniClassifiers/BaseIniClassifier.h"


namespace AGRemapCore {
    IniClassifyStats BaseIniClassifier::classify(const std::string& iniTxt, GameTypeIdFilter gameTypeIds) {
        return IniClassifyStats();
    }

    IniClassifyStats BaseIniClassifier::classify(const std::vector<std::string>& iniTxt, GameTypeIdFilter gameTypeIds) {
        return IniClassifyStats();
    }

    bool BaseIniClassifier::checkIsMod(const std::string& iniTxt, GameTypeIdFilter gameTypeIds) {
        return false;
    }

    bool BaseIniClassifier::checkIsMod(const std::vector<std::string>& iniTxt, GameTypeIdFilter gameTypeIds) {
        return false;
    }

    void BaseIniClassifier::checkIsFixedMod(const std::string& iniTxt, bool* isFixed, bool* isMod, GameTypeIdFilter gameTypeIds) {
        *isFixed = false;
        *isMod = false;
    }

    void BaseIniClassifier::checkIsFixedMod(const std::vector<std::string>& iniTxt, bool* isFixed, bool* isMod, GameTypeIdFilter gameTypeIds) {
        *isFixed = false;
        *isMod = false;
    }

    void BaseIniClassifier::clear() {
        // TODO: filled in later
    }
}
