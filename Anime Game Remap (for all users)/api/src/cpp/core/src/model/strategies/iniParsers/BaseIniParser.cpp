#include "AGRemapCore/model/strategies/iniParsers/BaseIniParser.h"

#include "AGRemapCore/model/files/IniFile.h"


namespace AGRemapCore {
    BaseIniParser::BaseIniParser(IniFile* iniFile): iniFile_(iniFile) {}

    IniFile* BaseIniParser::getIniFile() const {
        return iniFile_;
    }

    void BaseIniParser::setIniFile(IniFile* iniFile) {
        iniFile_ = iniFile;
    }

    void BaseIniParser::clear() {}

    std::vector<IniGraphGroup> BaseIniParser::parse() {
        return {};
    }
}
