#ifndef AGRemapCore_BaseIniParser_TPP
#define AGRemapCore_BaseIniParser_TPP

#include "BaseIniParser.h"


namespace AGRemapCore {
    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    BaseIniParser<K, V, KeyHash, KeyEqual>::BaseIniParser(IniFile* iniFile): iniFile_(iniFile) {}

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    IniFile* BaseIniParser<K, V, KeyHash, KeyEqual>::getIniFile() const {
        return iniFile_;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void BaseIniParser<K, V, KeyHash, KeyEqual>::setIniFile(IniFile* iniFile) {
        iniFile_ = iniFile;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void BaseIniParser<K, V, KeyHash, KeyEqual>::clear() {}

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::vector<typename BaseIniParser<K, V, KeyHash, KeyEqual>::GraphGroup> BaseIniParser<K, V, KeyHash, KeyEqual>::parse() {
        return {};
    }
}

#endif
