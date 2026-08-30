#ifndef AGRemapCore_VertexCounts_H
#define AGRemapCore_VertexCounts_H

#include <string>

#include "AGRemapCore/model/assets/ModDictAssets.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     The vertex counts related to a mod -- a :cpp:class:`ModDictAssets` pre-populated with this
     project's real vertex count data :raw-html:`<br />` :raw-html:`<br />`

     Like :cpp:class:`Hashes` and :cpp:class:`Indices`, a default-constructed ``VertexCounts`` is
     already fully populated, which is what lets :cpp:member:`ModType::vertexCounts` fall back to one
     and still be useful :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        Two differences from its :cpp:class:`Hashes`/:cpp:class:`Indices` siblings, both inherited
        from the pure-Python original:

        #. It derives from :cpp:class:`ModDictAssets`, **not** :cpp:class:`ModMappedAssets` -- the
           pure-Python ``VertexCounts`` extends ``ModAssets`` rather than ``ModMappedAssets``, so
           there is no fix-from/fix-to adjacency list here and therefore no ``getMap``/``hasFrom``.
           A vertex count is looked *up*, never remapped
        #. Its value type is ``int`` rather than ``std::string``

     .. note::
        Three index columns: ``version`` (the version index, at position 0), ``name`` and
        ``component``. So :cpp:func:`ModDictAssets::get` takes exactly two non-version values --
        the mod's name and the component -- in that order :raw-html:`<br />` :raw-html:`<br />`

        ``component`` is ``""`` for every row the software currently ships (see
        :cpp:func:`Data::getVertexCountDataRows`), so a caller wanting a mod's overall count passes
        an empty string for it. It is a real key value, not a "missing" marker -- the same
        convention :cpp:class:`Indices` uses for its own ``component`` column

     .. note::
        :cpp:class:`ModDictAssets` rather than :cpp:class:`ModAssets` (which is what the pure-Python
        original inherits): this table has exactly one version column, which is the case
        :cpp:class:`ModDictAssets`'s own class note calls out as belonging in the hash-based table
        rather than the linear-scanning one. The pure-Python ``ModAssets`` supports several version
        columns; ``VertexCounts`` never used more than one
     @endrst
     */
    class VertexCounts: public ModDictAssets<std::string, int> {
        public:

            /**
             * @brief
             @rst
             Constructs a new, fully-populated vertex count lookup table :raw-html:`<br />`
             :raw-html:`<br />`

             .. note::
                Unlike the pure-Python original there is no ``repo`` argument to swap the whole
                table out with -- nothing in this project ever passed one, and
                :cpp:func:`ModDictAssets::addRows` already covers extending it
             @endrst
             */
            VertexCounts();
    };
}

#endif
