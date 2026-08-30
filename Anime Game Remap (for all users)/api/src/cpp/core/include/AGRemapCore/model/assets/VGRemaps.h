#ifndef AGRemapCore_VGRemaps_H
#define AGRemapCore_VGRemaps_H

#include <string>

#include "AGRemapCore/model/VGRemap.h"
#include "AGRemapCore/model/assets/ModAssets.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     The vertex group remaps for a mod -- a :cpp:class:`ModAssets` pre-populated with this project's
     real remap data :raw-html:`<br />` :raw-html:`<br />`

     Maps the blend indices of one mod's vertex groups onto another's. Like its
     :cpp:class:`Hashes`/:cpp:class:`Indices`/:cpp:class:`VertexCounts` siblings, a
     default-constructed ``VGRemaps`` is already fully populated :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        **This is the only asset table built on** :cpp:class:`ModAssets` **rather than**
        :cpp:class:`ModDictAssets`, and the reason is structural, not stylistic: it has **two**
        version columns -- ``fromVersion`` *and* ``toVersion`` -- because a remap is defined between
        a version of one mod and a version of another. :cpp:class:`ModDictAssets` handles exactly one
        version column, so it cannot express this. :cpp:class:`ModAssets`'s linear scan is the cost of
        that extra dimension, which its own class note calls out as acceptable at this table's size
        (52 rows)

     .. note::
        Six index columns, in order: ``fromVersion`` (version), ``fromChar``, ``fromComp``,
        ``toVersion`` (version), ``toChar``, ``toComp``. So
        :cpp:func:`ModAssets::get` takes **four** non-version values -- ``fromChar``, ``fromComp``,
        ``toChar``, ``toComp``, in that relative order -- and **two** versions,
        ``fromVersion`` then ``toVersion`` :raw-html:`<br />` :raw-html:`<br />`

        Both component columns are ``""`` on every shipped row (see
        :cpp:func:`Data::getVGRemapDataRows`), so a caller remapping whole mods passes empty strings
        for them

     .. note::
        Unlike the other three asset tables, :cpp:member:`ModType::vgRemaps` does **not** default to
        a fresh instance per mod type -- it falls back to the single shared
        :cpp:func:`ModDataAssets::vgRemaps`. That mirrors the pure-Python ``ModType``, whose own
        default is ``ModDataAssets.VGRemaps.value`` rather than a new ``VGRemaps()``, and it matters
        here more than elsewhere: this is much the largest of the tables
     @endrst
     */
    class VGRemaps: public ModAssets<std::string, VGRemap> {
        public:

            /**
             * @brief
             @rst
             Constructs a new, fully-populated vertex group remap table :raw-html:`<br />`
             :raw-html:`<br />`

             .. note::
                Most callers want :cpp:func:`ModDataAssets::vgRemaps` instead -- see this class's
                last note. Constructing one directly is for when a genuinely independent, mutable
                table is wanted
             @endrst
             */
            VGRemaps();
    };
}

#endif
