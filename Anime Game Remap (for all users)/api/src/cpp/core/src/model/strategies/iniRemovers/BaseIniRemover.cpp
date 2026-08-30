#include "AGRemapCore/model/strategies/iniRemovers/BaseIniRemover.h"

#include "AGRemapCore/model/files/IniFile.h"


namespace AGRemapCore {
    BaseIniRemover::BaseIniRemover(IniFile* iniFile): iniFile_(iniFile) {}

    IniFile* BaseIniRemover::getIniFile() const {
        return iniFile_;
    }

    void BaseIniRemover::setIniFile(IniFile* iniFile) {
        iniFile_ = iniFile;
    }

    std::string BaseIniRemover::remove(bool parse, bool writeBack) {
        (void)parse;
        (void)writeBack;
        return "";
    }
}
