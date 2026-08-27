#include "AGRemapCore/model/strategies/iniFixers/BaseIniFixer.h"

#include "AGRemapCore/model/files/IniFile.h"
#include "AGRemapCore/model/strategies/iniParsers/BaseIniParser.h"


namespace AGRemapCore {
    BaseIniFixer::BaseIniFixer(BaseIniParser* parser):
        parser_(parser),
        iniFile_(parser != nullptr ? parser->getIniFile() : nullptr) {}

    BaseIniParser* BaseIniFixer::getParser() const {
        return parser_;
    }

    void BaseIniFixer::setParser(BaseIniParser* parser) {
        parser_ = parser;
        iniFile_ = (parser != nullptr) ? parser->getIniFile() : nullptr;
    }

    IniFile* BaseIniFixer::getIniFile() const {
        return iniFile_;
    }

    void BaseIniFixer::clear() {}

    BaseIniFixer::FixResult BaseIniFixer::fix(bool keepBackup, bool fixOnly, bool hideOrig) {
        return fixImpl(keepBackup, fixOnly, hideOrig, true, true);
    }

    BaseIniFixer::FixResult BaseIniFixer::fixImpl(bool keepBackup, bool fixOnly, bool hideOrig, bool withBoilerPlate, bool withSrc) {
        (void)keepBackup;
        (void)fixOnly;
        (void)hideOrig;
        (void)withBoilerPlate;
        (void)withSrc;
        return {};
    }
}
