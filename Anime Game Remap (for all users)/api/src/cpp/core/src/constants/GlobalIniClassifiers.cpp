#include "AGRemapCore/constants/GlobalIniClassifiers.h"


namespace AGRemapCore {
    IniClassifier& GlobalIniClassifiers::classifier() {
        static IniClassifier instance;
        return instance;
    }
}
