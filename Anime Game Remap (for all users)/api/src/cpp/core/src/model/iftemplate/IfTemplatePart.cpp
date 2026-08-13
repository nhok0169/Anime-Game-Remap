#include "AGRemapCore/model/iftemplate/IfTemplatePart.h"


namespace AGRemapCore {

    namespace {
        IncIdGenerator<size_t>& idGenerator() {
            static IncIdGenerator<size_t> generator(0);
            return generator;
        }
    }

    IfTemplatePart::IfTemplatePart() {

    }

}
