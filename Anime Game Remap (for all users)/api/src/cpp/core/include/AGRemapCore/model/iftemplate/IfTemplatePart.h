#ifndef AGRemapCore_IfTemplatePart_H
#define AGRemapCore_IfTemplatePart_H

#include <cstddef>

#include "AGRemapCore/tools/idGenerator/IncIdGenerator.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Base class for some part in an `IfTemplate` -- the C++ port of ``IfTemplatePart.py``.
     Deliberately minimal for now (just an auto-generated ``id``): nothing in the C++ core yet
     needs to hold a heterogeneous collection of part types polymorphically (that would come
     with a future C++ port of the rest of the `IfTemplate` tree), so adding any *behavioral*
     virtual method now would be speculative. The destructor is still virtual, though -- unlike
     :cpp:class:`BaseOrderedMultiMap` (which avoids a vtable entirely for its own hot-path
     performance reasons, see its doc comment), this class exists specifically to be inherited
     from, and a base meant for inheritance should always be safely destructible through a base
     pointer even before anything actually does so.
     @endrst
     */
    class IfTemplatePart {
        public:
            IfTemplatePart();
            virtual ~IfTemplatePart() = default;
    };

}

#endif
