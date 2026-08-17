#ifndef AGRemapCore_IfTemplatePart_H
#define AGRemapCore_IfTemplatePart_H

#include <cstddef>
#include <optional>

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
            /**
             * @brief Constructs a new part
             *
             * @param id
             @rst
             The id for the part. If not provided, a new id is generated (see :cpp:func:`refreshId`) :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``std::nullopt``
             @endrst
             */
            explicit IfTemplatePart(const std::optional<size_t>& id = std::nullopt);
            virtual ~IfTemplatePart() = default;

            /**
             * @brief Retrieves the id for the part
             */
            size_t id() const;

            /**
             * @brief Regenerates the id for the part
             *
             * @return The newly generated id
             */
            size_t refreshId();

        private:
            // Shared across every IfTemplatePart instance (a single, process-wide counter), the
            // same way IfTemplatePartAutoId is a single module-level counter in IfTemplatePart.py
            // rather than something reset per-instance.
            static size_t generateId();

            size_t id_;
    };

}

#endif
