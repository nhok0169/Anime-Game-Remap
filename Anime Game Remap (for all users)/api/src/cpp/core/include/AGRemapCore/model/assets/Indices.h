#ifndef AGRemapCore_Indices_H
#define AGRemapCore_Indices_H

#include <string>
#include <unordered_map>
#include <vector>

#include "AGRemapCore/model/assets/ModMappedAssets.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     The indices related to a mod and its fix -- a :cpp:class:`ModMappedAssets` pre-populated with
     this project's real index data :raw-html:`<br />` :raw-html:`<br />`

     The exact sibling of :cpp:class:`Hashes`; read that class first. Everything there applies here
     too -- a default-constructed ``Indices`` is already fully populated, each instance gets its own
     mutable table, and the pybind layer's ``PyIndices`` is a separate ``py::object``-keyed class
     sharing only the underlying data (:cpp:func:`Data::getIndexDataRows`) :raw-html:`<br />`
     :raw-html:`<br />`

     .. note::
        The one difference from :cpp:class:`Hashes` is the shape: **4** index columns rather than 3
        -- ``version`` (the version index, at position 0), ``name``, ``component`` and ``type``.
        :cpp:class:`ModMappedAssets`'s query methods therefore take **three** non-version values
        positionally, in that order. ``component`` is frequently the empty string (see
        ``IndexData``'s own rows), which is a real key value here, not a "missing" marker
     @endrst
     */
    class Indices: public ModMappedAssets<std::string, std::string> {
        public:

            /**
             * @brief Constructs a new, fully-populated index lookup table
             *
             * @param map
             @rst
             The initial `adjacency list`_ mapping the indices to fix **from** to the indices to fix
             **to** -- see :cpp:func:`ModMappedAssets::getMap` :raw-html:`<br />` :raw-html:`<br />`

             **Default**: empty
             @endrst
             */
            explicit Indices(std::unordered_map<std::string, std::vector<std::string>> map = {});
    };
}

#endif
