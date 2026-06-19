#include "AGRemapCore/tools/idGenerator/IncIdGenerator.h"


namespace AGRemapCore {

    template <typename Id>
    IncIdGenerator<Id>::IncIdGenerator(const Id &defaultId): defaultId(defaultId), currentId(defaultId) {

    }

    template <typename Id>
    bool IncIdGenerator<Id>::getId(Id &result) {
        result = currentId;
        currentId++;
        return true;
    }

    template <typename Id>
    void IncIdGenerator<Id>::reset() {
        currentId = defaultId;
    }
}