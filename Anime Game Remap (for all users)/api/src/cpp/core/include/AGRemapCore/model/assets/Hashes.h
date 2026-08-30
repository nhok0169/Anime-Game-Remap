#ifndef AGRemapCore_Hashes_H
#define AGRemapCore_Hashes_H

#include <string>
#include <unordered_map>
#include <vector>

#include "AGRemapCore/model/assets/ModMappedAssets.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     The hashes related to a mod and its fix -- a :cpp:class:`ModMappedAssets` pre-populated with
     this project's real hash data :raw-html:`<br />` :raw-html:`<br />`

     Unlike :cpp:class:`ModMappedAssets` (a generic, reusable engine), this is one *specific*
     instance of it: its rows (:cpp:func:`Data::getHashDataRows`) and its 3-index
     ``(version, name, type)`` shape are baked in at construction, so a default-constructed
     ``Hashes`` already knows every hash the software ships with. That matches the pure-Python
     ``Hashes``'s own contract exactly -- there, too, a bare ``Hashes()`` is fully populated, which
     is why :cpp:member:`ModType::hashes` can fall back to one and still be useful
     :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        The index columns are ``version`` (the version index, at position 0), ``name`` and ``type``.
        :cpp:class:`ModMappedAssets`'s query methods take the two non-version values positionally in
        that order -- there is no name-keyed argument form here, unlike the Python side, where the
        ``ModAssets`` wrapper adds one

     .. note::
        This is the core-side counterpart of the pybind layer's ``PyHashes``, which binds the same
        data as Python's ``Hashes``. The two are deliberately separate classes rather than one
        deriving from the other: the bound one is keyed by ``py::object`` (so Python callers can use
        any hashable), while this one is plain ``std::string``-keyed, which is all
        :cpp:class:`AGRemapCore` needs and keeps the core free of any Python dependency. They share
        the underlying data table (:cpp:func:`Data::getHashDataRows`) and nothing else
     @endrst
     */
    class Hashes: public ModMappedAssets<std::string, std::string> {
        public:

            /**
             * @brief Constructs a new, fully-populated hash lookup table
             *
             * @param map
             @rst
             The initial `adjacency list`_ mapping the hashes to fix **from** to the hashes to fix
             **to** -- see :cpp:func:`ModMappedAssets::getMap` :raw-html:`<br />` :raw-html:`<br />`

             **Default**: empty
             @endrst
             */
            explicit Hashes(std::unordered_map<std::string, std::vector<std::string>> map = {});
    };
}

#endif
