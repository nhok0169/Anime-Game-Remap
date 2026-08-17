#include "AGRemapCore/model/iftemplate/IfTemplatePart.h"


namespace AGRemapCore {

    namespace {
        IncIdGenerator<size_t>& idGenerator() {
            static IncIdGenerator<size_t> generator(0);
            return generator;
        }
    }

    size_t IfTemplatePart::generateId() {
        size_t result;
        idGenerator().getId(result);
        return result;
    }

    IfTemplatePart::IfTemplatePart(const std::optional<size_t>& id): id_(id.has_value() ? *id : generateId()) {

    }

    size_t IfTemplatePart::id() const {
        return id_;
    }

    size_t IfTemplatePart::refreshId() {
        id_ = generateId();
        return id_;
    }

}
