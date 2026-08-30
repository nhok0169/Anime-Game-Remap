#ifndef AGRemapCore_BaseIniFixer_TPP
#define AGRemapCore_BaseIniFixer_TPP

// No IniFile.h include: IniFile is only ever named as a pointer here, and pulling it in would
// close a cycle (IniFile.h -> ModType.h -> BaseIniFixer.h -> this file).
#include "BaseIniFixer.h"


namespace AGRemapCore {
    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    BaseIniFixer<K, V, KeyHash, KeyEqual>::BaseIniFixer(Parser* parser):
        parser_(parser),
        iniFile_(parser != nullptr ? parser->getIniFile() : nullptr) {}

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename BaseIniFixer<K, V, KeyHash, KeyEqual>::Parser* BaseIniFixer<K, V, KeyHash, KeyEqual>::getParser() const {
        return parser_;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void BaseIniFixer<K, V, KeyHash, KeyEqual>::setParser(Parser* parser) {
        parser_ = parser;
        iniFile_ = (parser != nullptr) ? parser->getIniFile() : nullptr;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    IniFile* BaseIniFixer<K, V, KeyHash, KeyEqual>::getIniFile() const {
        return iniFile_;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void BaseIniFixer<K, V, KeyHash, KeyEqual>::clear() {}

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename BaseIniFixer<K, V, KeyHash, KeyEqual>::FixResult
    BaseIniFixer<K, V, KeyHash, KeyEqual>::fix(ParseData& parseData, bool keepBackup, bool fixOnly, bool hideOrig) {
        return fixImpl(parseData, keepBackup, fixOnly, hideOrig, true, true);
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename BaseIniFixer<K, V, KeyHash, KeyEqual>::FixResult
    BaseIniFixer<K, V, KeyHash, KeyEqual>::fixImpl(ParseData& parseData, bool keepBackup, bool fixOnly, bool hideOrig,
                                                    bool withBoilerPlate, bool withSrc) {
        (void)parseData;
        (void)keepBackup;
        (void)fixOnly;
        (void)hideOrig;
        (void)withBoilerPlate;
        (void)withSrc;
        return {};
    }
}

#endif
